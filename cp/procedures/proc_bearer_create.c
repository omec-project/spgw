// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "sm_struct.h"
#include "gtp_ies.h"
#include "pfcp_cp_session.h"
#include "util.h"
#include "spgw_config_struct.h"
#include "gx_interface.h"
#include "gtpv2_interface.h"
#include "spgw_cpp_wrapper.h"
#include "cp_io_poll.h"
#include "gtpv2_set_ie.h"
#include "cp_transactions.h"
#include "gtpv2_interface.h"
#include "byteswap.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "ipc_api.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "cp_log.h"
#include <assert.h>
#include "proc_bearer_create.h"
#include "pfcp.h"
#include "pfcp_enum.h"
#include "proc.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_bearer_create_proc(msg_info_t *msg)
{
    proc_context_t *proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(proc->proc_name, "BEARER_CREATE");
    proc->proc_type = DED_BER_ACTIVATION_PROC;
    proc->ue_context = (void *)msg->ue_context;
    proc->pdn_context = (void *)msg->pdn_context; 
    proc->handler = bearer_create_event_handler;
    SET_PROC_MSG(proc, msg);
    msg->proc_context = proc;
    return proc;
}

void
bearer_create_event_handler(void *proc, void *msg_info)
{
    msg_info_t *msg = (msg_info_t *)msg_info;
    proc_context_t *proc_context = (proc_context_t *)proc;
    uint8_t event = msg->event;
    switch(event) {
        case BEARER_CREATE_EVNT: {
            send_pfcp_modify_session_pre_cbreq(proc, msg_info);
            break;
        }

        case CREATE_BER_RESP_RCVD_EVNT: {
            process_sgwc_create_bearer_rsp(proc_context, msg);
            break;
        }

        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            if(proc_context->state == PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE) {
                process_pfcp_sess_mod_resp_pre_cbr_handler(msg, proc_context);
            } else {
               process_pfcp_sess_mod_rsp_post_cbr_handler(proc_context);
            }
            break;
        }
        default:
            assert(0);
    }
    
    return;
}

void
proc_bearer_create_failed(proc_context_t *proc_context, uint8_t cause )
{
    LOG_MSG(LOG_DEBUG, "bearer create failed with cause = %d", cause);
    proc_context->result = PROC_RESULT_FAILURE;
    increment_stat(PROCEDURES_SPGW_DEDICATED_BEARER_ACTIVATION_PROC_FAILURE);
    if(proc_context->state == PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE) {
    } else if (proc_context->state == CREATE_BER_REQ_SNT_STATE) {
    }
    proc_bearer_create_complete(proc_context);
}

void 
proc_bearer_create_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
}

void
send_pfcp_modify_session_pre_cbreq(void *proc_cbr, void *msg)
{
    msg_info_t *msg_info = (msg_info_t *)msg;
    proc_context_t *proc_ctxt = (proc_context_t*)proc_cbr;
    assert(msg_info->proc_context == proc_ctxt);
    pdn_connection_t *pdn_cntxt = (pdn_connection_t *)proc_ctxt->pdn_context;
    ue_context_t *ue_context = (ue_context_t*)proc_ctxt->ue_context;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	int32_t seq_no;
	uint8_t pfcp_msg[1024] = {0};

	seq_no = fill_pfcp_gx_sess_mod_req(proc_ctxt, &pfcp_sess_mod_req, pdn_cntxt);

	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &ue_context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(ue_context->upf_context));

    transData_t *trans_entry = NULL;
    if(cp_config->cp_type == PGWC){
        trans_entry = start_response_wait_timer(ue_context,
                pfcp_msg, encoded,
                pfcp_modify_session_pre_cbreq_timeout);
    }
    else if(cp_config->cp_type == SAEGWC)
    {
        trans_entry = start_response_wait_timer(ue_context,
                pfcp_msg, encoded, pfcp_modify_session_pre_cbreq_timeout);
    }
    SET_TRANS_SELF_INITIATED(trans_entry);

    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, seq_no, (void*)trans_entry);
    trans_entry->sequence = seq_no;
    proc_ctxt->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc_ctxt;
    proc_ctxt->state = PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE; 

	return;
}

void 
pfcp_modify_session_pre_cbreq_timeout(void *data)
{
    // TODO : add handling here 
    assert(0);
    LOG_MSG(LOG_NEVER, "data = %p",data);
    return;
}



// process PFCP modify response and send CBReq on GTP interface 
int
process_pfcp_sess_mod_resp_pre_cbr_handler(void *data, void *p)
{
    proc_context_t *proc_ctxt = (proc_context_t *)p;
	uint16_t payload_length = 0;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

    uint64_t sess_id = msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid; 
	uint32_t teid = UE_SESS_ID(sess_id);
	uint32_t sequence = get_gtp_sequence(); 

	/* Retrive the session information based on session id. */
	if ((sess_id != 0)) {
        context = (ue_context_t *)get_sess_entry_seid(sess_id);
        if(context == NULL) {
		    LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", sess_id);
            proc_bearer_create_failed(proc_ctxt, GTPV2C_CAUSE_CONTEXT_NOT_FOUND);
		    return -1;
        }
	}
    assert(proc_ctxt->ue_context == context);

	/* Retrieve the UE context */
	if((teid != 0)) {
        context = (ue_context_t *)get_ue_context(teid);
        if(context == NULL) {
		    LOG_MSG(LOG_ERROR, "No session entry found for teid: %u", teid);
            proc_bearer_create_failed(proc_ctxt, GTPV2C_CAUSE_CONTEXT_NOT_FOUND);
		    return -1;
        }
	}
    assert(proc_ctxt->ue_context == context);

    /*Validate the modification is accepted or not. */
    if(msg->rx_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
        LOG_MSG(LOG_DEBUG, "Cause received Modify response is %d",
                msg->rx_msg.pfcp_sess_mod_resp.cause.cause_value);
        // TODO : mapping the pfcp cause on GTP
        proc_bearer_create_failed(proc_ctxt, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
        return -1;
    }

	ebi_index = UE_BEAR_ID(sess_id) - 5;
	bearer = context->eps_bearers[ebi_index];
	/* Update the UE state */
	pdn = GET_PDN(context, ebi_index);
	pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	if (!bearer) {
		LOG_MSG(LOG_ERROR,
				"Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet");
        proc_bearer_create_failed(proc_ctxt, GTPV2C_CAUSE_CONTEXT_NOT_FOUND);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

#if 0
    uint8_t ebi = 0;
    get_bearer_info_install_rules(pdn, &ebi);
    bearer = context->eps_bearers[ebi];
    if (!bearer) {
        LOG_MSG(LOG_ERROR,
                "Retrive modify bearer context but EBI is non-existent- "
                "Bitmap Inconsistency - Dropping packet");
        proc_bearer_create_failed(proc_ctxt, GTPV2C_CAUSE_CONTEXT_NOT_FOUND);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }
#endif
    bearer = (eps_bearer_t *)proc_ctxt->bearer_context;

    /* TODO: NC Need to remove hard coded pti value */
    set_create_bearer_request(gtpv2c_tx, sequence, context,
            bearer, pdn->default_bearer_id, 0, NULL, 0);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

#if 0
	resp = get_sess_entry_seid(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
	if(resp == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu",
				msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	if ((SAEGWC != cp_config->cp_type) && ((resp->msg_type == GTP_CREATE_BEARER_RSP) ||
			(resp->msg_type == GX_RAR_MSG))){
	    gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));
		if(resp->msg_type != GTP_CREATE_BEARER_RSP){
			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&(my_sock.s5s8_recv_sockaddr), gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S5S8_IFACE);
		}
        // standalone sgw case 
		if (resp->msg_type == GTP_CREATE_BEARER_RSP) {

            increment_mme_peer_stats(MSG_RX_GTPV2_S11_CBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
		}
		else {
            // peer address needs to be corrected 
            increment_mme_peer_stats(MSG_TX_GTPV2_S11_CBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
		}
	} else {
#endif
    
 	struct sockaddr_in s11_mme_sockaddr;
    memset(&s11_mme_sockaddr, 0, sizeof(struct sockaddr_in));

	s11_mme_sockaddr.sin_family = AF_INET;
	s11_mme_sockaddr.sin_port = htons(GTPC_UDP_PORT);
	s11_mme_sockaddr.sin_addr.s_addr = htonl(context->s11_mme_gtpc_ipv4.s_addr);

    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
            (struct sockaddr *) &s11_mme_sockaddr,
            sizeof(struct sockaddr_in));

    sequence = gtpv2c_tx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;
    transData_t *gtpc_trans;
    gtpc_trans = start_response_wait_timer(proc_ctxt, (uint8_t *)gtp_tx_buf, 
                                            payload_length, 
                                            process_cbr_timeout);

    SET_TRANS_SELF_INITIATED(gtpc_trans);
    gtpc_trans->proc_context = (void *)proc_ctxt;
    proc_ctxt->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = sequence;
    gtpc_trans->peer_sockaddr = s11_mme_sockaddr;
    add_gtp_transaction(local_addr, port_num, sequence, gtpc_trans);

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_CBREQ, s11_mme_sockaddr.sin_addr.s_addr);
    proc_ctxt->state = CREATE_BER_REQ_SNT_STATE;

	return 0;
}

int
process_sgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg)
{
	uint8_t ebi_index;
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    create_bearer_rsp_t *cb_rsp = &msg->rx_msg.cb_rsp;
    ue_context_t *context = (ue_context_t *)proc->ue_context;

    if(cb_rsp->cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED) {
        proc_bearer_create_failed(proc, cb_rsp->cause.cause_value);
        return -1;
    }

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	if(bearer == NULL) {
         LOG_MSG(LOG_ERROR, "Received CBRsp and  bearer not found. Ignore CBRsp");
		/* TODO:
		 * This mean ebi we allocated and received doesnt match
		 * In correct design match the bearer in transtient struct from sgw-u teid
		 * */
		return -1;
	}

	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	bearer->s1u_enb_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s1u_enb_fteid.ipv4_address;
	bearer->s1u_enb_gtpu_teid = cb_rsp->bearer_contexts.s1u_enb_fteid.teid_gre_key;
	bearer->s1u_sgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s1u_sgw_fteid.ipv4_address;
	bearer->s1u_sgw_gtpu_teid = cb_rsp->bearer_contexts.s1u_sgw_fteid.teid_gre_key;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cb_rsp->header.teid.has_teid.seq) ;
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	pfcp_sess_mod_req.create_pdr_count = 0;
	pfcp_sess_mod_req.update_far_count = 0;

	if (cb_rsp->bearer_contexts.s1u_enb_fteid.header.len  != 0) {
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s1u_enb_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s1u_enb_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(cb_rsp->bearer_contexts.s1u_enb_fteid.interface_type);
		update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		pfcp_sess_mod_req.update_far_count++;
	}

	uint32_t sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);


	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXA_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_sgwc_pfcp_mod_post_cbrsp_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    bearer->pdn->trans_entry = trans_entry; 
    proc->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc;

    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;


	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

void
process_cbr_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p",data);
    return;
}


void
process_pfcp_sess_mod_rsp_post_cbr_handler( proc_context_t *proc)
{
    // TODO : check the response from data path for this pfcp message
    increment_stat(PROCEDURES_SPGW_DEDICATED_BEARER_ACTIVATION_PROC_SUCCESS);
    proc_bearer_create_complete(proc); 
}

void 
process_sgwc_pfcp_mod_post_cbrsp_timeout(void *data)
{
    transData_t *pfcp_trans = (transData_t *)data;
    proc_context_t *proc = (proc_context_t*)pfcp_trans->proc_context;
    proc_bearer_create_failed(proc, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}


#ifdef FUTURE_NEED
void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p",data);
    return;
}

int
process_pgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg)
{
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	uint8_t ebi_index;
    create_bearer_rsp_t *cb_rsp = &msg->rx_msg.cb_rsp;
    ue_context_t *context = (ue_context_t *)proc->ue_context;

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if (NULL == bearer)
	{
        LOG_MSG(LOG_ERROR, "CBRsp received at PGW but bearer not found");
		/* TODO: Invalid ebi index handling */
		return -1;
	}

	bearer->s5s8_sgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s58_u_sgw_fteid.ipv4_address;
	bearer->s5s8_sgw_gtpu_teid = cb_rsp->bearer_contexts.s58_u_sgw_fteid.teid_gre_key;

	bearer->s5s8_pgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s58_u_pgw_fteid.ipv4_address;
	bearer->s5s8_pgw_gtpu_teid = cb_rsp->bearer_contexts.s58_u_pgw_fteid.teid_gre_key;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cb_rsp->header.teid.has_teid.seq) ;
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	pfcp_sess_mod_req.create_pdr_count = 0;
	pfcp_sess_mod_req.update_far_count = 0;

#if 0
	if (cb_rsp->bearer_contexts.s58_u_sgw_fteid.header.len != 0) {
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s5s8_sgw_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s5s8_sgw_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(cb_rsp->bearer_contexts.s58_u_sgw_fteid.interface_type);
		update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		pfcp_sess_mod_req.update_far_count++;
	}
#endif

	uint32_t sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pgwc_create_bearer_rsp_pfcp_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    bearer->pdn->trans_entry = trans_entry; 
    proc->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc;

    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;

	//context->sequence = seq_no;
	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

int
process_cbresp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pgwc_create_bearer_rsp(&msg->rx_msg.cb_rsp);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p",unused_parama);
	return 0;
}

int
process_create_bearer_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_create_bearer_request(&msg->rx_msg.cb_req);
	if (ret) {
			LOG_MSG(LOG_ERROR, "Error: %d ", ret);
			return -1;
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p",unused_parama);
	return 0;
}

int
process_create_bearer_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_sgwc_create_bearer_rsp(&msg->rx_msg.cb_rsp);
	if (ret) {
			LOG_MSG(LOG_ERROR, "Error: %d ", ret);
			return -1;
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p",unused_parama);
	return 0;
}

void 
process_create_bearer_request_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p",data);
    return;
}

int
process_create_bearer_request(create_bearer_req_t *cbr)
{
	uint8_t ebi_index = 0;
	uint8_t new_ebi_index = 0;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	context  = (ue_context_t *)get_ue_context_by_sgw_s5s8_teid(cbr->header.teid.has_teid.teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Error: in getting ue context from teid %u ", cbr->header.teid.has_teid.teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	bearer = (eps_bearer_t *)calloc(1, sizeof(eps_bearer_t));
	if (bearer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate bearer ");
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}

	ebi_index = cbr->lbi.ebi_ebi - 5;
	new_ebi_index = ++(context->pdns[ebi_index]->num_bearer) - 1;

	bearer->pdn = context->pdns[ebi_index];
	pdn = context->pdns[ebi_index];
	context->eps_bearers[new_ebi_index] = bearer;
	pdn->eps_bearers[new_ebi_index] = bearer;

	s11_mme_sockaddr.sin_addr.s_addr =
		context->s11_mme_gtpc_ipv4.s_addr;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cbr->header.teid.has_teid.seq);
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	bearer->qos.arp.preemption_vulnerability = cbr->bearer_contexts.bearer_lvl_qos.pvi;
	bearer->qos.arp.priority_level = cbr->bearer_contexts.bearer_lvl_qos.pl;
	bearer->qos.arp.preemption_capability = cbr->bearer_contexts.bearer_lvl_qos.pci;
	bearer->qos.qci = cbr->bearer_contexts.bearer_lvl_qos.qci;
	bearer->qos.ul_mbr = cbr->bearer_contexts.bearer_lvl_qos.max_bit_rate_uplnk;
	bearer->qos.dl_mbr = cbr->bearer_contexts.bearer_lvl_qos.max_bit_rate_dnlnk;
	bearer->qos.ul_gbr = cbr->bearer_contexts.bearer_lvl_qos.guarntd_bit_rate_uplnk;
	bearer->qos.dl_gbr = cbr->bearer_contexts.bearer_lvl_qos.guarntd_bit_rate_dnlnk;

	bearer->s5s8_pgw_gtpu_ipv4.s_addr = cbr->bearer_contexts.s58_u_pgw_fteid.ipv4_address;
	bearer->s5s8_pgw_gtpu_teid = cbr->bearer_contexts.s58_u_pgw_fteid.teid_gre_key;

	fill_dedicated_bearer_info(bearer, context, pdn);

	pfcp_sess_mod_req.create_pdr_count = bearer->pdr_count;
	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cbr->header, bearer, pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXA_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_create_bearer_request_pfcp_timeout);
    pdn->trans_entry = trans_entry;

	context->sequence = seq_no;
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	resp = get_sess_entry_seid(context->pdns[ebi_index]->seid);
	if (resp == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to find ue context from seid");
		return -1;
	}

	memset(resp->eps_bearer_lvl_tft, 0, 257);
	memcpy(resp->eps_bearer_lvl_tft,
			cbr->bearer_contexts.tft.eps_bearer_lvl_tft,
			257);
	resp->tft_header_len = cbr->bearer_contexts.tft.header.len;
	resp->eps_bearer_id = new_ebi_index + 5;
	resp->msg_type = GTP_CREATE_BEARER_REQ;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = DED_BER_ACTIVATION_PROC;
	pdn->proc = DED_BER_ACTIVATION_PROC;

	return 0;
}
#endif

int
fill_pfcp_gx_sess_mod_req(proc_context_t *proc, 
                          pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		                  pdn_connection_t *pdn)
{
	uint8_t bearer_id = 0;
	uint32_t seq = 0;
	eps_bearer_t *bearer = NULL;

	memset(pfcp_sess_mod_req,0,sizeof(pfcp_sess_mod_req_t));
	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
					           HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

  /* PFCPCOMPLIANCE : no need to fill in the FSEID value in the modify */

	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);
	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

    /* Requirement - updating policy only on either default bearer to dedicated bearer */
	if ((cp_config->cp_type == PGWC) ||
		(cp_config->cp_type == SAEGWC))
	{
        eps_bearer_t *new_bearer = NULL;
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
        while (pcc_rule != NULL) {

			if(pcc_rule->action == RULE_ACTION_ADD) {
				/*
				 * Installing new rule
				 */
				 bearer = get_bearer(pdn, &pcc_rule->dyn_rule->qos);
                 if(bearer == NULL && new_bearer != NULL)
                    assert(0); // Only 1 bearer can be created in one iteration
				 if(bearer == NULL) {
                    LOG_MSG(LOG_DEBUG, "New dedicated Bearer should be created to accomodate-new rule with QCI/ARP ");
					/*
					 * create dedicated bearer
					 */
					bearer = (eps_bearer_t *)calloc(1, sizeof(eps_bearer_t));
					if(bearer == NULL) {
						LOG_MSG(LOG_ERROR, "Failure to allocate bearer");
						return 0;
						/* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
					}
                    new_bearer = bearer;
					bzero(bearer,  sizeof(eps_bearer_t));
					bearer->pdn = pdn;
                    proc->bearer_context = bearer;
                    /* GXCOMPLIANCE - MME assigns bearer id. Definately current way of using num_bearer is not correct.
                     * it will be problematic when bearers are deleted/created 
                     */
					bearer_id = get_new_bearer_id(pdn); 
					pdn->eps_bearers[bearer_id] = bearer;
					pdn->context->eps_bearers[bearer_id] = bearer;
					pdn->num_bearer++;
					set_s5s8_pgw_gtpu_teid_using_pdn(bearer, pdn);
                    // GXCONFUSION : find out if this is correct QoS  
					memcpy(&(bearer->qos), &(pcc_rule->dyn_rule->qos), sizeof(bearer_qos_ie));
                    // create qer and add it into the table, and update PDR  
					fill_dedicated_bearer_info(bearer, pdn->context, pdn);
				 } else if(new_bearer == bearer) {
                    // add this new rule in existing outstanding bearer
                    // possible only if more than 1 rule added while creating bearer 
                    LOG_MSG(LOG_DEBUG, "Existing Bearer found %d ", bearer->eps_bearer_id);
                 } else {
                    LOG_MSG(LOG_DEBUG, "Existing Bearer found %d, update bearer case  ", bearer->eps_bearer_id);
                 }

				 bearer->dynamic_rules[bearer->num_dynamic_filters] = pcc_rule->dyn_rule; 

				 fill_pfcp_entry(bearer, pcc_rule->dyn_rule, RULE_ACTION_ADD);

				 fill_create_pfcp_info(pfcp_sess_mod_req, pcc_rule->dyn_rule, bearer);
				 bearer->num_dynamic_filters++;

				//Adding rule and bearer id to a hash
				bearer_id_t *id;
				id = (bearer_id_t *)malloc(sizeof(bearer_id_t));
				memset(id, 0 , sizeof(bearer_id_t));
				rule_name_key_t key = {0};
				id->bearer_id = bearer_id;
				strncpy(key.rule_name, pcc_rule->dyn_rule->rule_name,
						strlen(pcc_rule->dyn_rule->rule_name));
				sprintf(key.rule_name, "%s%d", key.rule_name, pdn->call_id);
				if (add_rule_name_entry(key.rule_name, bearer_id) != 0) {
					LOG_MSG(LOG_ERROR,"Failed to add rule name entry %s ", key.rule_name);
					return 0;
				}
			} else if(pcc_rule->action == RULE_ACTION_MODIFY) {
                LOG_MSG(LOG_DEBUG, "policy rule action modify ");
				/*
				 * Currently not handling dynamic rule qos modificaiton
				 */
				bearer = get_bearer(pdn, &pcc_rule->dyn_rule->qos);
				if(bearer == NULL)
				{
					 LOG_MSG(LOG_ERROR, "Failure to find bearer ");
					 return 0;
					 /* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
				}
				fill_pfcp_entry(bearer, pcc_rule->dyn_rule, RULE_ACTION_MODIFY);
				fill_update_pfcp_info(pfcp_sess_mod_req, pcc_rule->dyn_rule);

			} else {
                LOG_MSG(LOG_ERROR,"unknown action What action it is ? %d ",pcc_rule->action);
            }
            
            TAILQ_REMOVE(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
            pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
		}

#if 0
		/* TODO: Remove Below section START after install, modify and remove support */
		if (pdn->policy.num_charg_rule_delete != 0) {
			memset(pfcp_sess_mod_req,0,sizeof(pfcp_sess_mod_req_t));
			seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

			set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
							           HAS_SEID, seq);

			pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

			//TODO modify this hard code to generic
			inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
			node_value = inet_addr(pAddr);

			set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);
		}
		/* TODO: Remove Below section END */
#endif

        pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
        while (pcc_rule != NULL) {
            TAILQ_REMOVE(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
            if(RULE_ACTION_DELETE == pcc_rule->action) {
                /* bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx + idx_offset].dyn_rule.qos);
                   if(NULL == bearer)
                   {
                   LOG_MSG(LOG_ERROR, "Failure to find bearer ");
                   return;
                   } */

                rule_name_key_t rule_name = {0};
                memset(rule_name.rule_name, '\0', sizeof(rule_name.rule_name));
                strncpy(rule_name.rule_name, pcc_rule->dyn_rule->rule_name,
                        strlen(pcc_rule->dyn_rule->rule_name));
                sprintf(rule_name.rule_name, "%s%d",
                        rule_name.rule_name, pdn->call_id);
                int8_t bearer_id = get_rule_name_entry(rule_name.rule_name);
                if (-1 == bearer_id) {
                    /* TODO: Error handling bearer not found */
                }

                if ((bearer_id + 5) == pdn->default_bearer_id) {
                    for (uint8_t iCnt = 0; iCnt < MAX_BEARERS; ++iCnt) {
                        if (NULL != pdn->eps_bearers[iCnt]) {
                            fill_remove_pfcp_info(pfcp_sess_mod_req, pdn->eps_bearers[iCnt]);
                        }
                    }
                } else {
                    fill_remove_pfcp_info(pfcp_sess_mod_req, pdn->eps_bearers[bearer_id]);
                }
            }
            pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
        }
	}
    return seq;
}

static int
add_qer_into_hash(qer_t *qer)
{
	int ret = -1;
	qer_t *qer_ctxt = NULL;
	qer_ctxt = (qer_t *)calloc(1, sizeof(qer_t));
	if (qer_ctxt == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate qer ");
		return ret;
	}

	qer_ctxt->qer_id = qer->qer_id;
	qer_ctxt->max_bitrate.ul_mbr = qer->max_bitrate.ul_mbr;
	qer_ctxt->max_bitrate.dl_mbr = qer->max_bitrate.dl_mbr;
	qer_ctxt->guaranteed_bitrate.ul_gbr = qer->guaranteed_bitrate.ul_gbr;
	qer_ctxt->guaranteed_bitrate.dl_gbr = qer-> guaranteed_bitrate.dl_gbr;

	ret = add_qer_entry(qer_ctxt->qer_id, qer_ctxt);

	if(ret != 0) {
		LOG_MSG(LOG_ERROR, "Adding qer entry Error: %d ", ret);
		return ret;
	}


	return ret;
}

// allocate pdr, link pdr to bearer & dynamic_rule  
int fill_pfcp_entry(eps_bearer_t *bearer, dynamic_rule_t *dyn_rule,
		enum rule_action_t rule_action)
{
	/*
	 * For ever PCC rule create 2 PDR and 2 QER and 2 FAR
	 * all these struture should be created and filled here
	 * also store its reference in rule itself
	 * May be pdr arrary in bearer not needed
	 */
	char mnc[4] = {0};
	char mcc[4] = {0};
	char nwinst[32] = {0};
	ue_context_t *context = bearer->pdn->context;
	pdn_connection_t *pdn = bearer->pdn;
	pdr_t *pdr_ctxt = NULL;
	int ret;
	uint16_t flow_len = 0;

	if (context->serving_nw.mnc_digit_3 == 15) {
		sprintf(mnc, "0%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2);
	} else {
		sprintf(mnc, "%u%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2,
				context->serving_nw.mnc_digit_3);
	}

	sprintf(mcc, "%u%u%u", context->serving_nw.mcc_digit_1,
			context->serving_nw.mcc_digit_2,
			context->serving_nw.mcc_digit_3);

	sprintf(nwinst, "mnc%s.mcc%s", mnc, mcc);

	for(int i =0; i < 2; i++)
	{

		pdr_ctxt = (pdr_t *)calloc(1, sizeof(pdr_t));
		if (pdr_ctxt == NULL) {
			LOG_MSG(LOG_ERROR, "Failure to allocate pdr ");
			return -1;
		}
		memset(pdr_ctxt,0,sizeof(pdr_t));

		pdr_ctxt->rule_id =  generate_pdr_id();
		pdr_ctxt->prcdnc_val =  dyn_rule->precedence;
		pdr_ctxt->far.far_id_value = generate_far_id();
		pdr_ctxt->session_id = pdn->seid;
		/*to be filled in fill_sdf_rule*/
		pdr_ctxt->pdi.sdf_filter_cnt = 0;
		dyn_rule->pdr[i] = pdr_ctxt;
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++)
		{
			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL)
			{
				flow_len = dyn_rule->flow_desc[itr].flow_desc_len;
				memcpy(&(pdr_ctxt->pdi.sdf_filter[pdr_ctxt->pdi.sdf_filter_cnt].flow_desc),
						&(dyn_rule->flow_desc[itr].sdf_flow_description),
						flow_len);
				pdr_ctxt->pdi.sdf_filter[pdr_ctxt->pdi.sdf_filter_cnt].len_of_flow_desc = flow_len;
				pdr_ctxt->pdi.sdf_filter_cnt++;
			}
		}

		if (i == SOURCE_INTERFACE_VALUE_ACCESS) {
			if (cp_config->cp_type == PGWC) {
				pdr_ctxt->pdi.local_fteid.teid = bearer->s5s8_pgw_gtpu_teid;
				pdr_ctxt->pdi.local_fteid.ipv4_address =
						bearer->s5s8_pgw_gtpu_ipv4.s_addr;
			} else {
				pdr_ctxt->pdi.local_fteid.teid = bearer->s1u_sgw_gtpu_teid;
				pdr_ctxt->pdi.local_fteid.ipv4_address =
						bearer->s1u_sgw_gtpu_ipv4.s_addr;
			}
			pdr_ctxt->far.actions.forw = 0;

			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
		} else {
			pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;
			pdr_ctxt->pdi.local_fteid.teid = 0;
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
			pdr_ctxt->far.actions.forw = 0;
			if(cp_config->cp_type == PGWC) {
				pdr_ctxt->far.outer_hdr_creation.ipv4_address =
					bearer->s5s8_sgw_gtpu_ipv4.s_addr;
				pdr_ctxt->far.outer_hdr_creation.teid =
					bearer->s5s8_sgw_gtpu_teid;
				pdr_ctxt->far.dst_intfc.interface_value =
					DESTINATION_INTERFACE_VALUE_ACCESS;
			}
		}

		if(rule_action == RULE_ACTION_ADD)
		{
			bearer->pdrs[bearer->pdr_count++] = pdr_ctxt;
		}

		ret = add_pdr_entry(pdr_ctxt->rule_id, pdr_ctxt);
		if ( ret != 0) {
			LOG_MSG(LOG_ERROR, "Adding pdr entry Error: %d ", ret);
			return -1;
		}

		pdr_ctxt->pdi.src_intfc.interface_value = i;
		strncpy((char * )pdr_ctxt->pdi.ntwk_inst.ntwk_inst, (char *)nwinst, 32);
		pdr_ctxt->qer.qer_id = bearer->qer_id[i].qer_id;
		pdr_ctxt->qer_id[0].qer_id = pdr_ctxt->qer.qer_id;
		pdr_ctxt->qer.max_bitrate.ul_mbr = dyn_rule->qos.ul_mbr;
		pdr_ctxt->qer.max_bitrate.dl_mbr = dyn_rule->qos.dl_mbr;
		pdr_ctxt->qer.guaranteed_bitrate.ul_gbr = dyn_rule->qos.ul_gbr;
		pdr_ctxt->qer.guaranteed_bitrate.dl_gbr = dyn_rule->qos.dl_gbr;

		ret = add_qer_into_hash(&pdr_ctxt->qer);

		if(ret != 0) {
			LOG_MSG(LOG_ERROR, "Adding qer entry Error: %d ", ret);
			return ret;
		}
		enum flow_status f_status = (enum flow_status)dyn_rule->flow_status;
		switch(f_status)
		{
			case FL_ENABLED_UPLINK:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_OPEN;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_CLOSED;
				break;

			case FL_ENABLED_DOWNLINK:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_CLOSED;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_OPEN;
				break;

			case FL_ENABLED:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_OPEN;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_OPEN;
				break;

			case FL_DISABLED:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_CLOSED;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_CLOSED;
				break;
			case FL_REMOVED:
				/*TODO*/
				break;
		}
		// URR support 
		pdr_ctxt->urr.urr_id = bearer->urr_id[i].urr_id;
		pdr_ctxt->urr_id[0].urr_id = pdr_ctxt->urr.urr_id;
	}
	return 0;
}

