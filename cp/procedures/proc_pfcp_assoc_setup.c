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
#include "proc_pfcp_assoc_setup.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"


static int
assoication_setup_request(upf_context_t *upf_context);

void 
upf_pfcp_setup_failure(void *data, uint16_t event);

proc_context_t*
alloc_pfcp_association_setup_proc(void *upf_context)
{

    msg_info_t *setup_msg = calloc(1, sizeof(msg_info_t));
    setup_msg->event = PFCP_ASSOCIATION_SETUP; 
    proc_context_t *pfcp_setup_proc;
    pfcp_setup_proc = calloc(1, sizeof(proc_context_t));
    pfcp_setup_proc->proc_type = PFCP_ASSOC_SETUP_PROC; 
    pfcp_setup_proc->handler = pfcp_association_event_handler;
    SET_PROC_MSG(pfcp_setup_proc, setup_msg);
    pfcp_setup_proc->upf_context = upf_context;
    
    setup_msg->proc_context = pfcp_setup_proc;

    return pfcp_setup_proc;
}

void 
pfcp_association_event_handler(void *proc, void *msg_info) 
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case PFCP_ASSOCIATION_SETUP: {
            association_setup_handler(proc_context, msg); 
            break;
        }
        default:
            rte_panic("wrong event received");
    }
    return;
}

int
association_setup_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    RTE_SET_USED(msg);
    int ret = 0;
	upf_context_t *upf_context = proc_context->upf_context;
    assert (upf_context->state != PFCP_ASSOC_RESP_RCVD_STATE); 

    printf("[%s] - %d -  Initiate PFCP association setup to UPF %s \n",__FUNCTION__,__LINE__, inet_ntoa(upf_context->upf_sockaddr.sin_addr));
    ret = assoication_setup_request(upf_context);
	if(ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%s:%d Error: %d \n",
				__file__, __func__, __LINE__, ret);
        upf_context->state = UPF_SETUP_FAILED; 
        queue_stack_unwind_event(UPF_CONNECTION_SETUP_FAILED, (void *)upf_context, upf_pfcp_setup_failure);
	    return -1;
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
    LIST_INSERT_HEAD(&upf_context->pending_sub_procs, key, procentries);
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

    if (pfcp_trans == NULL ) {
        clLog(clSystemLog, eCLSeverityCritical, "%s: transaction not found. Dropping association response message. from UPF IP:%s",
                __func__, inet_ntoa(peer_addr->sin_addr));
        return -1;
    }

    upf_context = (upf_context_t *)(pfcp_trans->cb_data);

    stop_transaction_timer(pfcp_trans);
    upf_context->trans_entry = NULL;
    free(pfcp_trans);

	if(msg->pfcp_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED) {
		clLog(sxlogger, eCLSeverityDebug,
				"Cause received  Association response is %d\n",
				msg->pfcp_msg.pfcp_ass_resp.cause.cause_value);

		/* TODO: Add handling to send association to next upf
		 * for each buffered CSR 
         * Cleanup should be done for each layer
         *   1. Cleanup GTPv2
         *   2. Cleanup IP-CAN if GX is enabled. 
         */
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

static uint32_t 
fill_pfcp_association_setup_req(pfcp_assn_setup_req_t *pfcp_ass_setup_req)
{

    uint32_t seq  = 1;
    char node_addr[INET_ADDRSTRLEN] = {0};

    memset(pfcp_ass_setup_req, 0, sizeof(pfcp_assn_setup_req_t)) ;

    seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_SETUP_REQUEST, seq);
    set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_setup_req->header),
            PFCP_ASSOCIATION_SETUP_REQUEST, NO_SEID, seq);

    inet_ntop(AF_INET, &(cp_config->pfcp_ip), node_addr, INET_ADDRSTRLEN);

    unsigned long node_value = inet_addr(node_addr);
    set_node_id(&(pfcp_ass_setup_req->node_id), node_value);

    set_recovery_time_stamp(&(pfcp_ass_setup_req->rcvry_time_stmp));

    /* As we are not supporting this feature
       set_cpf_features(&(pfcp_ass_setup_req->cp_func_feat)); */
    return seq;
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

        increment_userplane_stats(MSG_TX_PFCP_SXASXB_ASSOCSETUPREQ, GET_UPF_ADDR(upf_context));

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
    msg_info_t *msg = NULL;
    upf_context_t *upf_context = (upf_context_t *)data;
    pending_proc_key_t *key;
    key = LIST_FIRST(&upf_context->pending_sub_procs);
    while(key != NULL)
    {
        printf("Reject buffered procedure \n");
        LIST_REMOVE(key, procentries);
        proc_context_t *proc = (proc_context_t *)key->proc_context;
        msg = calloc(1, sizeof(msg_info_t));
        msg->msg_type = PFCP_ASSOC_SETUP_FAILED;
        msg->proc_context = key->proc_context; 
        SET_PROC_MSG(proc, msg);
        proc->handler(proc, msg);
        rte_free(key);
        key = LIST_FIRST(&upf_context->pending_sub_procs);
    }
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

    key = LIST_FIRST(&upf_context->pending_sub_procs);
    while (key != NULL) {
        LIST_REMOVE(key, procentries);
        csreq_proc = key->proc_context;
        process_pfcp_sess_est_request(csreq_proc, upf_context);
        rte_free(key);
        key = LIST_FIRST(&upf_context->pending_sub_procs);
    }
	return ;
}

