// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "cp_proc.h"
#include "ue.h"
#include "upf_struct.h"
#include <stdio.h>
#include "rte_common.h"
#include "pfcp.h"
#include "gx_interface.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_session.h"
#include "cp_config.h"
#include "clogger.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "upf_struct.h"
#include "cp_events.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_enum.h"
#include "cp_transactions.h"
#include "cp_peer.h"
#include "pfcp_association_setup_proc.h"
#include "tables/tables.h"
#include "util.h"


extern udp_sock_t my_sock;

static int
assoication_setup_request(upf_context_t *upf_context);

void 
upf_pfcp_setup_failure(void *data, uint16_t event);

proc_context_t*
alloc_pfcp_association_setup_proc(msg_info_t *msg)
{
    proc_context_t *pfcp_setup_proc;
    pfcp_setup_proc = calloc(1, sizeof(proc_context_t));
    pfcp_setup_proc->proc_type = msg->proc; 
    pfcp_setup_proc->handler = pfcp_association_event_handler;
    msg->proc_context = pfcp_setup_proc;
    return pfcp_setup_proc;
}

void 
pfcp_association_event_handler(void *proc, uint32_t event, void *data)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    RTE_SET_USED(proc_context);
    switch(event) {
        case PFCP_ASSOCIATION_SETUP: {
            msg_info_t *msg = (msg_info_t *)data;
            association_setup_handler(msg, NULL); 
            break;
        }
        default:
            assert(0);
    }
    return;
}

int
association_setup_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	ue_context_t *context = msg->ue_context;
	upf_context_t *upf_context = context->upf_context;

    assert (upf_context->state != PFCP_ASSOC_RESP_RCVD_STATE); 

    // Initiate new request or buffer this CSReq 
	ret = process_pfcp_assoication_request(context); 
	if(ret) {
		if(ret != -1) {
			cs_error_response(msg, ret,
			   cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		}
	    process_error_occured_handler_new(data, unused_param);
		clLog(sxlogger, eCLSeverityCritical, "%s:%s:%d Error: %d \n",
				__file__, __func__, __LINE__, ret);
	    return -1;
	}

#ifdef TEMP
	pdn_connection_t *pdn = msg->pdn_context;
    // ITs not clear why we want to store csr in the resp.
    // and that too only if pfcp assocoation is up 
	pdn = context->pdns[ebi_index];
    upf_context = context->upf_context;
	if (upf_context->state == PFCP_ASSOC_RESP_RCVD_STATE) {
 		ret = get_sess_entry_seid(pdn->seid, &resp);
		if(ret != -1 && resp != NULL){
			resp->gtpc_msg.csr = msg->gtpc_msg.csr;
		}
	}
#endif

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_pfcp_assoication_request(ue_context_t *ue_context)
{
	int ret = 0;
	upf_context_t *upf_context = NULL;
    proc_context_t  *proc = ue_context->current_proc;

    upf_context = ue_context->upf_context;

    printf("UPF association state %d \n", upf_context->state);
    if(upf_context->state == PFCP_ASSOC_REQ_SNT_STATE) {
        printf("UPF association formation in progress. Buffer new CSReq  \n");
        // After every setup timeout we would flush the buffered entries..
        // dont worry about #of packets in buffer for now 
        buffer_csr_request(proc);
        proc->flags |= UPF_ASSOCIATION_PENDING;
        return 0;
    }
	if (upf_context->state == 0) {
		printf("[%s] - %d -  Initiate PFCP association setup to UPF %s \n",__FUNCTION__,__LINE__, inet_ntoa(upf_context->upf_sockaddr.sin_addr));
        ret = assoication_setup_request(upf_context);
		if (ret) {
				clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
						__func__, __LINE__, ret);
				return ret;
		}
	    buffer_csr_request(proc);
        proc->flags |= UPF_ASSOCIATION_PENDING;
        return 0;
	}
    return 0;
}

int
buffer_csr_request(proc_context_t *proc_context)
{
    ue_context_t *ue = proc_context->ue_context;
    upf_context_t *upf_context = ue->upf_context;
	pending_proc_key_t *key =
					rte_zmalloc_socket(NULL, sizeof(pending_proc_key_t),
						RTE_CACHE_LINE_SIZE, rte_socket_id());
	key->proc_context = (void *)proc_context;
    LIST_INSERT_HEAD(&upf_context->pendingProcs, key, procentries);
	return 0;
}

int 
handle_pfcp_association_setup_response(void *msg_t)
{
    msg_info_t *msg = (msg_info_t*)msg_t;
	upf_context_t *upf_context = NULL;
    int ret=0;
    struct sockaddr_in *peer_addr = &msg->peer_addr;
    uint32_t seq_num = msg->pfcp_msg.pfcp_ass_resp.header.seid_seqno.no_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    assert(msg->msg_type == PFCP_ASSOCIATION_SETUP_RESPONSE);
    RTE_SET_USED(ret);
    // Requirement : 
    // 0. Validate presence of IE
    // 1. node_id should come as name or ip address
    // 2. Can UPF change its address and can it be different 

#ifdef IMMEDIATE_NEED
    memcpy(&msg->upf_ipv4.s_addr,
           &msg->pfcp_msg.pfcp_ass_resp.node_id.node_id_value,
            IPV4_SIZE);
#endif
    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

    do {
        if(pfcp_trans != NULL) { 
            break;
        }
		clLog(clSystemLog, eCLSeverityCritical, "%s: transaction not found. using workaround to find transaction for  UPF IP:%s\n",
		__func__, inet_ntoa(peer_addr->sin_addr));
        upf_context = NULL; 
        upf_context_entry_lookup(peer_addr->sin_addr.s_addr, &upf_context);
        pfcp_trans = upf_context->trans_entry;
        // hack for pfcp which sends 0 sequence number in ack 
	    if (pfcp_trans == NULL ) {
		    clLog(clSystemLog, eCLSeverityCritical, "%s: transaction not found. Dropping association response message. from UPF IP:%s\n",
				__func__, inet_ntoa(peer_addr->sin_addr));
		    return -1;
	    }
        break;
    }while(0);
    upf_context = (upf_context_t *)(pfcp_trans->cb_data);
    msg->upf_context = upf_context;

    stop_transaction_timer(pfcp_trans);

    upf_context->trans_entry = NULL;
    free(pfcp_trans);

	if(msg->pfcp_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED) {
		msg->state = ERROR_OCCURED_STATE;
		msg->event = ERROR_OCCURED_EVNT;
		msg->proc = INITIAL_PDN_ATTACH_PROC;
		clLog(sxlogger, eCLSeverityDebug,
				"Cause received  Association response is %d\n",
				msg->pfcp_msg.pfcp_ass_resp.cause.cause_value);

		/* TODO: Add handling to send association to next upf
		 * for each buffered CSR 
         * Cleanup should be done for each layer
         *   1. Cleanup GTPv2
         *   2. Cleanup IP-CAN if GX is enabled. 
         */
        // Should I introduce new state ? 
        upf_context->state = UPF_SETUP_FAILED; 
        queue_stack_unwind_event(UPF_CONNECTION_SETUP_FAILED, (void *)upf_context, upf_pfcp_setup_failure);
		return -1;
	}

	msg->state = upf_context->state;
	/* Set Hard code value for temporary purpose as assoc is only in initial pdn */
	msg->proc = INITIAL_PDN_ATTACH_PROC;
	msg->event = PFCP_ASSOC_SETUP_RESP_RCVD_EVNT;

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_ASSOCIATION_SETUP_RESPONSE[%u], UPF_IP:%u, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type, GET_UPF_ADDR(upf_context),
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));
#if 0
    if(cp_config->cp_type != SGWC) {
        /* Init rule tables of user-plane */
        context->upf_context->upf_sockaddr.sin_addr.s_addr = msg->upf_ipv4.s_addr;
        // init_dp_rule_tables();
    }
#endif
    upf_context = msg->upf_context;
    assert(upf_context != NULL); /* if NULL we should have already dropped msg */
    upf_context->state = PFCP_ASSOC_RESP_RCVD_STATE;

    upf_context->up_supp_features =
        msg->pfcp_msg.pfcp_ass_resp.up_func_feat.sup_feat;

    switch (cp_config->cp_type)
    {
        case SGWC :
            if (msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;

            if( msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].src_intfc ==
                    SOURCE_INTERFACE_VALUE_CORE )
                upf_context->s5s8_sgwu_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].ipv4_address;
            break;

        case PGWC :
            if (msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s5s8_pgwu_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

        case SAEGWC :
            if( msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

    }

    /* teid_range from first user plane ip IE is used since, for same CP ,
     * DP will assigne single teid_range , So all IE's will have same value for teid_range*/
    /* Change teid base address here */
    if(msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teidri != 0){
        /* Requirement : This data should go in the upf context */
        set_base_teid(msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teid_range);
    }


    /* Adding ip to cp  heartbeat when dp returns the association response*/
    add_ip_to_heartbeat_hash(peer_addr,
            msg->pfcp_msg.pfcp_ass_resp.rcvry_time_stmp.rcvry_time_stmp_val);

    if ((add_node_conn_entry((uint32_t)peer_addr->sin_addr.s_addr,
                    SX_PORT_ID)) != 0) {

        clLog(clSystemLog, eCLSeverityCritical, "Failed to add connection entry for SGWU/SAEGWU");
    }
    queue_stack_unwind_event(UPF_CONNECTION_SETUP_SUCCESS, (void *)upf_context, upf_pfcp_setup_success);
    return 0;
}

/**
 * @brief  : This function creates association setup request and sends to peer
 * @param  : context holds information of ue
 * @param  : ebi_index denotes index of bearer stored in array
 * @return : This function dose not return anything
 */
static int
assoication_setup_request(upf_context_t *upf_context)
{
    transData_t *trans_entry;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    uint32_t seq_num;
    pfcp_assn_setup_req_t pfcp_ass_setup_req = {0};

    printf("Initiate PFCP setup to peer address = %s \n", inet_ntoa(upf_context->upf_sockaddr.sin_addr));

    seq_num = fill_pfcp_association_setup_req(&pfcp_ass_setup_req);
    RTE_SET_USED(seq_num);

    uint8_t pfcp_msg[256] = {0};
    int encoded = encode_pfcp_assn_setup_req_t(&pfcp_ass_setup_req, pfcp_msg);

    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
    header->message_len = htons(encoded - 4);

    if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &upf_context->upf_sockaddr) < 0 ) {
        clLog(clSystemLog, eCLSeverityDebug,"Error sending\n\n");
        // this cause will be put in csrsp 
        return GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER;
    } else {

        update_cli_stats(GET_UPF_ADDR(upf_context), pfcp_ass_setup_req.header.message_type, SENT, SX);

        trans_entry = start_pfcp_node_timer((void *)upf_context, pfcp_msg, encoded,  
                                            process_assoc_resp_timeout_handler);
        
        trans_entry->sequence = seq_num;
        // for now...upf is responding with 0 seq 
        add_pfcp_transaction(local_addr, port_num, seq_num, (void*)trans_entry);  
        upf_context->trans_entry = trans_entry;
        upf_context->state = PFCP_ASSOC_REQ_SNT_STATE;
    }
    return 0;
}

void
process_assoc_resp_timeout_handler(void *data1)
{
    transData_t *data = (transData_t *)data1;
    upf_context_t *upf_ctxt = (upf_context_t *)(data->cb_data);
    clLog(sxlogger, eCLSeverityCritical, "PFCP association response timeout from %s  \n",
          inet_ntoa(upf_ctxt->upf_sockaddr.sin_addr));
    // transaction will be freed in the cleanup 
    upf_pfcp_setup_failure((void *)upf_ctxt, PFCP_SETUP_TIMEOUT); 
}

void 
upf_pfcp_setup_failure(void *data, uint16_t event)
{
    RTE_SET_USED(event);
    msg_info_t msg = {0};
    upf_context_t *upf_context = (upf_context_t *)data;
    pending_proc_key_t *key;
    key = LIST_FIRST(&upf_context->pendingProcs);
    while(key != NULL)
    {
        printf("Reject buffered procedure \n");
        LIST_REMOVE(key, procentries);
        proc_context_t *csreq_proc = (proc_context_t *)key->proc_context;

        memset(&msg, 0, sizeof(msg));
        msg.msg_type = PFCP_ASSOCIATION_SETUP_RESPONSE;
        msg.upf_context = upf_context;
        msg.proc_context = key->proc_context; 
        msg.ue_context =  csreq_proc->ue_context;  
        msg.pdn_context = csreq_proc->pdn_context;  
        cs_error_response(&msg,
                          GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
                          cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        /* local cleanup of UE context */
        process_error_occured_handler_new(&msg, NULL);
        rte_free(key);
        key = LIST_FIRST(&upf_context->pendingProcs);
    }
    // ajay - check upf address carefully
    printf("Deleting UPF context %s",inet_ntoa(upf_context->upf_sockaddr.sin_addr));
    upf_context_delete_entry(upf_context->upf_sockaddr.sin_addr.s_addr);
    rte_free(upf_context);
}


void upf_pfcp_setup_success(void *data, uint16_t event)
{
    RTE_SET_USED(event);
    upf_context_t *upf_context = (upf_context_t *)data;
    pending_proc_key_t *key;
    proc_context_t *csreq_proc = NULL;
    int ret = 0;
    RTE_SET_USED(ret);

	printf("Received PFCP association response from %s\n",inet_ntoa(upf_context->upf_sockaddr.sin_addr));

    key = LIST_FIRST(&upf_context->pendingProcs);
    while (key != NULL) {
        LIST_REMOVE(key, procentries);
        csreq_proc = key->proc_context;
        pdn_connection_t *pdn = csreq_proc->pdn_context;
        process_pfcp_sess_est_request(pdn, upf_context);
        rte_free(key);
        key = LIST_FIRST(&upf_context->pendingProcs);
    }
	return ;
}

