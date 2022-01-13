// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "proc_struct.h"
#include "ue.h"
#include "upf_struct.h"
#include <stdio.h>
#include "pfcp.h"
#include "gx_interface.h"
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
#include "spgw_config_struct.h"
#include "cp_log.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gx_error_rsp.h"
#include "upf_struct.h"
#include "cp_events.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_enum.h"
#include "cp_transactions.h"
#include "cp_peer.h"
#include "proc_pfcp_assoc_setup.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "proc_initial_attach.h"
#include "assert.h"
#include "upf_apis.h"
#include "proc_struct.h"
#include "proc.h"

static int
assoication_setup_request(proc_context_t *proc);

void 
upf_pfcp_setup_failure(void *data, uint16_t event);

proc_context_t*
alloc_pfcp_association_setup_proc(void *upf_context)
{
    msg_info_t *setup_msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    setup_msg->event = PFCP_ASSOCIATION_SETUP; 
    proc_context_t *pfcp_setup_proc;
    pfcp_setup_proc = (proc_context_t*)calloc(1, sizeof(proc_context_t));
    strcpy(pfcp_setup_proc->proc_name, "PFCP_ASSOCIATION_SETUP");
    pfcp_setup_proc->proc_type = PFCP_ASSOC_SETUP_PROC; 
    pfcp_setup_proc->handler = pfcp_association_event_handler;
    SET_PROC_MSG(pfcp_setup_proc, setup_msg);
    pfcp_setup_proc->upf_context = upf_context;
    setup_msg->proc_context = pfcp_setup_proc;
    upf_context_t *upf = (upf_context_t*)upf_context;
    upf->proc = pfcp_setup_proc;
    LOG_MSG(LOG_DEBUG,"Proc %p upf context %p ", pfcp_setup_proc, upf_context);
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
        case PFCP_ASSOCIATION_SETUP_RSP: {
            handle_pfcp_association_setup_response(proc_context, msg);
            break;
        }
        default:
            assert(0); //wrong event received
    }
    return;
}

int
association_setup_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	upf_context_t *upf_context = (upf_context_t *)proc_context->upf_context;
    assert (upf_context->state != PFCP_ASSOC_RESP_RCVD_STATE); 
    upf_context->upf_sockaddr.sin_addr.s_addr = 0;
    upf_context = get_upf_context(upf_context->fqdn, upf_context->global_address);
    if (upf_context->upf_sockaddr.sin_addr.s_addr == 0) {
        //address is still not resolved 
		LOG_MSG(LOG_ERROR, "Error in resolving UPF address [%s]", upf_context->fqdn);
        upf_context->state = 0; 
        queue_stack_unwind_event(UPF_CONNECTION_SETUP_FAILED, (void *)upf_context, upf_pfcp_setup_failure);
	    return -1;
    }
    LOG_MSG(LOG_DEBUG1,"Initiate PFCP association setup to UPF %s ", inet_ntoa(upf_context->upf_sockaddr.sin_addr));
    ret = assoication_setup_request(proc_context);
	if(ret) {
		LOG_MSG(LOG_ERROR, "Error in setting up Association error: %d, msg = %p ", ret, msg);
        upf_context->state = 0; 
        queue_stack_unwind_event(UPF_CONNECTION_SETUP_FAILED, (void *)upf_context, upf_pfcp_setup_failure);
	    return -1;
	}
	return 0;
}

int
buffer_csr_request(proc_context_t *proc_context)
{
    ue_context_t *ue = (ue_context_t*)proc_context->ue_context;
    upf_context_t *upf_context = ue->upf_context;
	pending_proc_key_t *key = (pending_proc_key_t*) calloc(1, sizeof(pending_proc_key_t));
	key->proc_context = (void *)proc_context;
    LIST_INSERT_HEAD(&upf_context->pending_sub_procs, key, procentries);
	return 0;
}

int 
handle_pfcp_association_setup_response(proc_context_t *proc, void *msg_t)
{
    msg_info_t *msg = (msg_info_t*)msg_t;
	upf_context_t *upf_context = (upf_context_t *)proc->upf_context;
    struct sockaddr_in *peer_addr = &msg->peer_addr;
    // Requirement : 
    // 0. Validate presence of IE
    // 1. node_id should come as name or ip address
    // 2. Can UPF change its address and can it be different 

#ifdef IMMEDIATE_NEED
    memcpy(&msg->upf_ipv4.s_addr,
           &msg->rx_msg.pfcp_ass_resp.node_id.node_id_value,
            IPV4_SIZE);
#endif

	if(msg->rx_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED) {
		LOG_MSG(LOG_ERROR, "Cause received  Association response is %d",
				msg->rx_msg.pfcp_ass_resp.cause.cause_value);

		/* TODO: Add handling to send association to next upf
		 * for each buffered CSR 
         * Cleanup should be done for each layer
         *   1. Cleanup GTPv2
         *   2. Cleanup IP-CAN if GX is enabled. 
         */
        upf_context->state = 0; 
        queue_stack_unwind_event(UPF_CONNECTION_SETUP_FAILED, (void *)upf_context, upf_pfcp_setup_failure);
		return 0;
	}

	//msg->state = upf_context->state;
	/* Set Hard code value for temporary purpose as assoc is only in initial pdn */
	msg->proc = INITIAL_PDN_ATTACH_PROC;
	msg->event = PFCP_ASSOC_SETUP_RESP_RCVD_EVNT;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:PFCP_ASSOCIATION_SETUP_RESPONSE[%u], UPF_IP:%u, "
			"Procedure:%s, Event:%s",
			msg->msg_type, GET_UPF_ADDR(upf_context),
			get_proc_string(msg->proc),
			get_event_string(msg->event));

    upf_context->state = PFCP_ASSOC_RESP_RCVD_STATE;
    upf_context->up_supp_features = msg->rx_msg.pfcp_ass_resp.up_func_feat.sup_feat;
    upf_context->add_up_supp_features1 = msg->rx_msg.pfcp_ass_resp.up_func_feat.add_sup_feat1;
    upf_context->add_up_supp_features2 = msg->rx_msg.pfcp_ass_resp.up_func_feat.add_sup_feat2;

    switch (cp_config->cp_type)
    {
        case PGWC :
            if (msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s5s8_pgwu_ip =
                    msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

        case SAEGWC :
            if( msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;
        default:
            assert(0);

    }

    /* teid_range from first user plane ip IE is used since, for same CP ,
     * DP will assigne single teid_range , So all IE's will have same value for teid_range*/
    /* Change teid base address here */
    if(msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teidri != 0){
        /* Requirement : This data should go in the upf context */
        set_base_teid(msg->rx_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teid_range);
    }


    /* Adding ip to cp  heartbeat when dp returns the association response*/
    add_ip_to_heartbeat_hash(peer_addr,
            msg->rx_msg.pfcp_ass_resp.rcvry_time_stmp.rcvry_time_stmp_val);

    if ((add_node_conn_entry((uint32_t)peer_addr->sin_addr.s_addr,
                    SX_PORT_ID)) != 0) {
        LOG_MSG(LOG_ERROR, "Failed to add connection entry for SGWU/SAEGWU");
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
assoication_setup_request(proc_context_t *proc_context)
{
	upf_context_t *upf_context = (upf_context_t *)proc_context->upf_context;
    transData_t *trans_entry;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    uint32_t seq_num;
    pfcp_assn_setup_req_t pfcp_ass_setup_req = {0};

    LOG_MSG(LOG_INFO, "Initiate PFCP setup to peer address = %s ", inet_ntoa(upf_context->upf_sockaddr.sin_addr));

    seq_num = fill_pfcp_association_setup_req(&pfcp_ass_setup_req);

    uint8_t pfcp_msg[256] = {0};
    int encoded = encode_pfcp_assn_setup_req_t(&pfcp_ass_setup_req, pfcp_msg);

    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
    header->message_len = htons(encoded - 4);

    pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_ASSOCSETUPREQ, GET_UPF_ADDR(upf_context));

    trans_entry = start_response_wait_timer((void *)proc_context, pfcp_msg, encoded, process_assoc_resp_timeout_handler);
    
    SET_TRANS_SELF_INITIATED(trans_entry);
    trans_entry->sequence = seq_num;
    // for now...upf is responding with 0 seq 
    add_pfcp_transaction(local_addr, port_num, seq_num, (void*)trans_entry);  
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc_context;

    upf_context->proc = proc_context;
    upf_context->state = PFCP_ASSOC_REQ_SNT_STATE;

    return 0;
}

void
process_assoc_resp_timeout_handler(void *data)
{
    proc_context_t *proc = (proc_context_t *)data;
    upf_context_t *upf_ctxt = (upf_context_t *)(proc->upf_context);
    LOG_MSG(LOG_ERROR, "PFCP association response timeout from %s  ", inet_ntoa(upf_ctxt->upf_sockaddr.sin_addr));
    // transaction will be freed in the cleanup 
    upf_pfcp_setup_failure((void *)upf_ctxt, PFCP_SETUP_TIMEOUT); 
}

void 
upf_pfcp_setup_failure(void *data, uint16_t event)
{
    msg_info_t *msg = NULL;
    upf_context_t *upf_context = (upf_context_t *)data;
    pending_proc_key_t *key;
    key = LIST_FIRST(&upf_context->pending_sub_procs);
    while(key != NULL)
    {
        LOG_MSG(LOG_ERROR, "Reject buffered procedure ");
        LIST_REMOVE(key, procentries);
        proc_context_t *proc = (proc_context_t *)key->proc_context;
        msg = (msg_info_t *)proc->msg_info; 
        msg->msg_type = PFCP_ASSOC_SETUP_FAILED;
        msg->event = PFCP_ASSOC_SETUP_FAILED;
        proc->handler(proc, msg);
        free(key);
        key = LIST_FIRST(&upf_context->pending_sub_procs);
    }
    LOG_MSG(LOG_ERROR, "PFCP association setup failed for service [%s] address [%s], event = %d ",
                        upf_context->fqdn, inet_ntoa(upf_context->upf_sockaddr.sin_addr), event);
    proc_context_t *upf_proc = (proc_context_t *)upf_context->proc;
    if(upf_proc != NULL) {
        proc_pfcp_assoc_setup_failure(upf_proc, 0);
    } else {
	    LOG_MSG(LOG_ERROR, "Looks like pfcp association is already scheduled for UPF %s (%s)",upf_context->fqdn, inet_ntoa(upf_context->upf_sockaddr.sin_addr));
    }
}


void 
upf_pfcp_setup_success(void *data, uint16_t event)
{
    upf_context_t *upf_context = (upf_context_t *)data;
    pending_proc_key_t *key;
    proc_context_t *csreq_proc = NULL;

	LOG_MSG(LOG_DEBUG1, "Received PFCP association response from %s",inet_ntoa(upf_context->upf_sockaddr.sin_addr));

    key = LIST_FIRST(&upf_context->pending_sub_procs);
    while (key != NULL) {
        LIST_REMOVE(key, procentries);
        csreq_proc = (proc_context_t *)key->proc_context;
        msg_info_t *msg = (msg_info_t *)csreq_proc->msg_info; 
        msg->msg_type = PFCP_ASSOC_SETUP_SUCCESS;
        msg->event = PFCP_ASSOC_SETUP_SUCCESS;
        csreq_proc->handler(csreq_proc, msg);
        free(key);
        key = LIST_FIRST(&upf_context->pending_sub_procs);
    }
    proc_context_t *upf_proc = (proc_context_t *)upf_context->proc;
    proc_pfcp_assoc_setup_success(upf_proc);
	return ;
}

void 
proc_pfcp_assoc_setup_success(proc_context_t *proc)
{
    proc->result = PROC_RESULT_SUCCESS;
    // increment stats 
    proc_pfcp_assoc_setup_complete(proc);
}

void 
proc_pfcp_assoc_setup_failure(proc_context_t *proc_context, int cause)
{
	upf_context_t *upf_context = (upf_context_t *)proc_context->upf_context;
    if(cause != -1) {
        //increment upf stats for association setup message
    } else {
        //increment upf stats for association setup message
    }
    proc_context->result = PROC_RESULT_FAILURE;
    upf_context->state = 0;
    upf_context_delete_entry(upf_context->upf_sockaddr.sin_addr.s_addr);
    upf_context->upf_sockaddr.sin_addr.s_addr = 0;
    proc_pfcp_assoc_setup_complete(proc_context);
    uint16_t timeout = rand() % 30;
    LOG_MSG(LOG_INFO, "Schedule PFCP association setup after %d seconds", timeout);
    schedule_pfcp_association(timeout, upf_context);
}

void
proc_pfcp_assoc_setup_complete(proc_context_t *proc)
{
    end_upf_procedure(proc);
    return;
}
