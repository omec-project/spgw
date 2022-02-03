// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "assert.h"
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
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "cp_transactions.h"
#include "gtp_ies.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "proc_nw_init_detach.h"
#include "proc.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_nw_init_detach_proc(msg_info_t *msg)
{
    proc_context_t *detach_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(detach_proc->proc_name, "NW_UE_DETACH");
    detach_proc->proc_type = NW_INIT_DETACH_PROC;
    detach_proc->handler = nw_init_detach_event_handler;
    detach_proc->ue_context = msg->ue_context;
    detach_proc->pdn_context = msg->pdn_context;
    msg->proc_context = detach_proc;
    SET_PROC_MSG(detach_proc, msg);
    return detach_proc;
}

void 
nw_init_detach_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;
    switch(event) {
        case SEND_PFCP_DEL_SESSION_REQ: {
            LOG_MSG(LOG_DEBUG4, "Received event SEND_PFCP_DEL_SESSION_REQ in procedure = %s", proc_context->proc_name);
            send_nw_init_detach_pfcp_sess_del_request(proc_context, msg);
            break;
        }

        case PFCP_SESS_DEL_RESP_RCVD_EVNT: {
            LOG_MSG(LOG_DEBUG4, "Received event PFCP_SESS_DEL_RESP_RCVD_EVNT in procedure = %s", proc_context->proc_name);
            process_nw_init_detach_sess_del_resp_handler(proc_context, msg);
            break;
        }

        case NW_DETACH_DBREQ_TIMEOUT: {
            LOG_MSG(LOG_DEBUG4, "Received event NW_DETACH_DBREQ_TIMEOUT in procedure = %s", proc_context->proc_name);
            break;
        }

        case RCVD_GTP_DEL_BEARER_RSP: {
            LOG_MSG(LOG_DEBUG4, "Received event RCVD_GTP_DEL_BEARER_RSP in procedure = %s", proc_context->proc_name);
            process_nw_init_detach_dbrsp_handler(proc_context, msg);
            break;
        } 
        
        case CCA_RCVD_EVNT: {
            LOG_MSG(LOG_DEBUG4, "Received event CCA_RCVD_EVNT in procedure = %s", proc_context->proc_name);
            process_nw_init_detach_cca(proc_context, msg);
            break;
        } 
        default:
            assert(0); // unknown event 
    }
    return;
}

void
proc_nw_init_detach_success(proc_context_t *proc_context)
{
    increment_stat(PROCEDURES_SPGW_NW_INIT_DETACH_SUCCESS);
    proc_nw_init_detach_complete(proc_context);
}

void
proc_nw_init_detach_failure(proc_context_t *proc_context)
{
    increment_stat(PROCEDURES_SPGW_NW_INIT_DETACH_FAILURE);
    proc_nw_init_detach_complete(proc_context);
}

/* free transactions. delink from procedure. Free procedure and de-link from subscriber */
void 
proc_nw_init_detach_complete(proc_context_t *proc_context)
{
    ue_context_t *context = (ue_context_t *)proc_context->ue_context;
    pdn_connection_t *pdn = (pdn_connection_t*)proc_context->pdn_context;
    decrement_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
    decrement_ue_info_stats(SUBSCRIBERS_INFO_SPGW_PDN, context->imsi64, pdn->ipv4.s_addr);
    end_procedure(proc_context);
    return;
}

void
proc_nw_init_detach_pfcp_sess_del_request_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    ue_context_t *context = (ue_context_t *)proc_context->ue_context;
    assert(proc_context->pfcp_trans != NULL);
    transData_t *pfcp_trans = (transData_t *)proc_context->pfcp_trans;
    pfcp_trans->itr_cnt++;
    if(pfcp_trans->itr_cnt < cp_config->request_tries) {
	    LOG_MSG(LOG_ERROR, "PFCP session delete request retry(%d). IMSI %lu, " 
                       "PFCP sequence %d ", pfcp_trans->itr_cnt, context->imsi64, pfcp_trans->sequence);
        pfcp_timer_retry_send(my_sock.sock_fd_pfcp, pfcp_trans, &pfcp_trans->peer_sockaddr);
        restart_response_wait_timer(pfcp_trans);
        return;
    }

    LOG_MSG(LOG_ERROR, "PFCP Session delete timeout %lu, PFCP Sequence %d ", context->imsi64, pfcp_trans->sequence);
    proc_nw_init_detach_failure(proc_context);
    return;
}

int
send_nw_init_detach_pfcp_sess_del_request(proc_context_t *proc_context, msg_info_t *msg) 
{
	ue_context_t *context = (ue_context_t *)proc_context->ue_context;
	pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
	uint8_t pfcp_msg[512]={0};

	/* Fill pfcp structure for pfcp delete request and send it */
	sequence = fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = pdn->dp_seid;


	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSDELREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);

    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, proc_nw_init_detach_pfcp_sess_del_request_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;
    trans_entry->peer_sockaddr = context->upf_context->upf_sockaddr; 

    // link new transaction and proc context 
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;
	return 0;
}

int
process_nw_init_detach_sess_del_resp_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    pfcp_sess_del_rsp_t *pfcp_sess_del_resp = &msg->rx_msg.pfcp_sess_del_resp;
    

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:PFCP_SESSION_DELETION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, Event:%s",
			msg->msg_type,
			pfcp_sess_del_resp->header.seid_seqno.has_seid.seid,
			get_proc_string(proc_context->proc_type),
			get_event_string(msg->event));


	if(pfcp_sess_del_resp->cause.cause_value != REQUESTACCEPTED) {
		LOG_MSG(LOG_ERROR, "Cause received in pfcp delete response is %d",
				pfcp_sess_del_resp->cause.cause_value);
        // still go ahead and send DBReq towards MME
	}

    generate_nw_init_detach_dbreq(proc_context);
	return 0;
}

int 
generate_nw_init_detach_dbreq(proc_context_t *proc_context)
{
    ue_context_t *ue_ctxt = (ue_context_t *)proc_context->ue_context;
    pdn_connection_t *pdn_ctxt = (pdn_connection_t *)proc_context->pdn_context;
    uint8_t lbi = pdn_ctxt->default_bearer_id;
    del_bearer_req_t dbreq = {0};
    int sequence = get_gtp_sequence();

	struct sockaddr_in mme_s11_sockaddr_in;
    memset((void*)&mme_s11_sockaddr_in, 0, sizeof(struct sockaddr_in));
	mme_s11_sockaddr_in.sin_family = AF_INET;
	mme_s11_sockaddr_in.sin_port = htons(GTPC_UDP_PORT);
	mme_s11_sockaddr_in.sin_addr.s_addr = htonl(ue_ctxt->s11_mme_gtpc_ipv4.s_addr);

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));

    /* Fill gtpv2c structure for sending on s11 interface */
    set_gtpv2c_teid_header(&dbreq.header, GTP_DELETE_BEARER_REQ,
                ue_ctxt->s11_mme_gtpc_teid, sequence);

	set_ebi(&dbreq.lbi, IE_INSTANCE_ZERO, lbi);

    /*Encode the S11 delete session response message. */
    uint16_t payload_length = encode_del_bearer_req(&dbreq, (uint8_t *)gtp_tx_buf);

    LOG_MSG(LOG_DEBUG,"payload_length = %d ", payload_length);
    gtpv2c_header_t *gtp_hdr = (gtpv2c_header_t *)gtp_tx_buf;
    gtp_hdr->gtpc.message_len = htons(payload_length - 4);

	/* Send response on s11 interface */
	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
                (struct sockaddr *) &mme_s11_sockaddr_in,
				sizeof(struct sockaddr_in));

    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;
    transData_t *gtpc_trans;
    gtpc_trans = start_response_wait_timer(proc_context, (uint8_t *)gtp_tx_buf, 
                                           payload_length, 
                                           nw_init_dbreq_timeout);

    SET_TRANS_SELF_INITIATED(gtpc_trans);
    gtpc_trans->proc_context = (void *)proc_context;
    proc_context->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = sequence;
    gtpc_trans->peer_sockaddr = mme_s11_sockaddr_in;
    add_gtp_transaction(local_addr, port_num, sequence, gtpc_trans);

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_DBREQ, mme_s11_sockaddr_in.sin_addr.s_addr);


	return 0;
}

void 
nw_init_dbreq_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    // Option 1 - Retry few times 
    // Opton  2 - after configurable retry, generate timeout event for fsm  
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    msg->event = NW_DETACH_DBREQ_TIMEOUT;
    msg->proc_context = proc_context;
    SET_PROC_MSG(proc_context, msg);
    proc_context->handler(proc_context, msg);
}

void 
process_nw_init_detach_dbrsp_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    proc_nw_init_detach_success(proc_context);
#if 0
    // create CCRT after sending RAR ?
    if(gx) {
        generate_ccrt();
    }
#endif
    return;
}

int 
generate_ccrt(proc_context_t *proc_context) 
{
	gx_msg ccr_request = {0};
	char *buffer = NULL;
	uint16_t msglen = 0;
    pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
    ue_context_t *context = (ue_context_t *) proc_context->ue_context;
    uint8_t ebi_index = pdn->default_bearer_id - 5;

	/* Retrive Gx_context based on Sess ID. */
	ue_context_t *temp_context  = (ue_context_t *)get_ue_context_from_gxsessid((uint8_t *)pdn->gx_sess_id); 
	if (temp_context == NULL) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
		return -1;
	}
    assert(temp_context == context);

	/* VS: Set the Msg header type for CCR-T */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* VS: Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = TERMINATION_REQUEST ;

	/* VG: Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = TERMINATION ;

	/* VS: Fill the Credit Crontrol Request to send PCRF */
	if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, pdn->gx_sess_id) != 0) {
		LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
		return -1;
	}
	/* Update UE State */
	pdn->state = CCR_SNT_STATE;


	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msglen = gx_ccr_calc_length(&ccr_request.data.ccr);

    buffer = (char *)calloc(1, msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
    if (buffer == NULL) {
        LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
        return -1;
    }

    memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

    if (gx_ccr_pack(&(ccr_request.data.ccr),
                (unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msglen) == 0) {
        LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
        return -1;
    }
	gx_send(my_sock.gx_app_sock, buffer,
			msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T, saddr_in.sin_addr.s_addr);
    return 0;
}

void 
process_nw_init_detach_cca(proc_context_t *proc, msg_info_t *msg)
{
}
