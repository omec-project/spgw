// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

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
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "proc_detach.h"
#include "pfcp_messages_encoder.h"
#include "cp_transactions.h"
#include "gtp_ies.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "assert.h"
#include "proc.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_detach_proc(msg_info_t *msg)
{
    proc_context_t *detach_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(detach_proc->proc_name, "UE_DETACH");
    detach_proc->proc_type = msg->proc; 
    detach_proc->handler = detach_event_handler;
    detach_proc->ue_context = msg->ue_context;
    detach_proc->pdn_context = msg->pdn_context;
    msg->proc_context = detach_proc;
    SET_PROC_MSG(detach_proc, msg);
    return detach_proc;
}

void 
detach_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case DS_REQ_RCVD_EVNT: {
            process_ds_req_handler(proc_context, msg);
            break;
        } 
        case CCA_RCVD_EVNT: {
        }
        case PFCP_SESS_DEL_RESP_RCVD_EVNT: {
            process_pfcp_sess_del_resp_handler(proc_context, msg);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

// saegw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT => process_ds_req_handler
// saegw - DETACH_PROC IDEL_STATE  DS_REQ_RCVD_EVNT => process_ds_req_handler
// pgw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler
// pgw - DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler 
// sgw DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 
// sgw DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 
int
process_ds_req_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
    gtpv2c_header_t *dsr_header = &msg->rx_msg.dsr.header;
	LOG_MSG(LOG_DEBUG, "Callback called for "
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, Event:%s",
					gtp_type_str(msg->msg_type), msg->msg_type,
					dsr_header->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_event_string(msg->event));


	ret = process_pfcp_sess_del_request(proc_context, msg);

	if (ret) {
		if(ret != -1) {
            proc_detach_failure(proc_context, msg, ret);
        }
		return ret;
	}

	return 0;
}

int
process_pfcp_sess_del_resp_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	uint16_t payload_length = 0;
	uint16_t msglen = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
    pfcp_sess_del_rsp_t *pfcp_sess_del_resp = &msg->rx_msg.pfcp_sess_del_resp;
    ue_context_t *context = (ue_context_t *)proc_context->ue_context;
    pdn_connection_t *pdn = (pdn_connection_t*)proc_context->pdn_context;
    

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

		proc_detach_failure(proc_context, msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
		return -1;
	}

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

    ret = process_pfcp_sess_del_resp(
            pfcp_sess_del_resp->header.seid_seqno.has_seid.seid,
            gtpv2c_tx, &ccr_request, &msglen, proc_context);

	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		proc_detach_failure(proc_context, msg, ret); 
		return -1;
	}


    /* Lookup value in hash using session id and fill pfcp response and delete entry from hash*/
    // FIXME - need call level gx enable/disable 
    if(cp_config->gx_enabled) {
        buffer = (char *)calloc(1, msglen + sizeof(ccr_request.msg_type)+sizeof(ccr_request.seq_num));
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
    }


	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

	if ((cp_config->cp_type == PGWC) ) {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
        struct sockaddr_in peer_addr = gtpc_trans->peer_sockaddr; 
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		        sizeof(struct sockaddr_in));

        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_DSRSP, peer_addr.sin_addr.s_addr);
        decrement_stat(NUM_UE_PGW_ACTIVE_SUBSCRIBERS);
        increment_stat(PROCEDURES_PGW_MME_INIT_DETACH_SUCCESS);
	} else {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
        struct sockaddr_in peer_addr = gtpc_trans->peer_sockaddr; 
		/* Send response on s11 interface */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &peer_addr,
				sizeof(struct sockaddr_in));

		/*CLI:CSResp sent cnt*/
        increment_mme_peer_stats(MSG_TX_GTPV2_S11_DSRSP, peer_addr.sin_addr.s_addr);
        decrement_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
        increment_stat(PROCEDURES_SPGW_MME_INIT_DETACH_SUCCESS);
        decrement_ue_info_stats(SUBSCRIBERS_INFO_SPGW_PDN, context->imsi64, pdn->ipv4.s_addr);

	}
    proc_detach_complete(proc_context, msg);
	return 0;
}

void
process_pfcp_sess_del_request_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    ue_context_t *context = (ue_context_t *)proc_context->ue_context;
    assert(proc_context->gtpc_trans != NULL);
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
    msg_info_t *msg = (msg_info_t*)calloc(1, sizeof(msg_info_t));
    msg->msg_type = PFCP_SESSION_DELETION_RESPONSE;
    msg->proc_context = proc_context;

    proc_detach_failure(proc_context, msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_pfcp_sess_del_request(proc_context_t *proc_context, msg_info_t *msg) 
{

	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t s5s8_pgw_gtpc_teid = 0;
	uint32_t s5s8_pgw_gtpc_ipv4 = 0;
	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
    del_sess_req_t *ds_req = &msg->rx_msg.dsr;
	uint64_t ebi_index = ds_req->lbi.ebi_ebi - 5;
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
	uint8_t pfcp_msg[512]={0};

    pdn = (pdn_connection_t *)proc_context->pdn_context;

	/* Lookup and get context of delete request */
	ret = delete_context(ds_req->lbi, ds_req->header.teid.has_teid.teid,
		&context, &s5s8_pgw_gtpc_teid, &s5s8_pgw_gtpc_ipv4);
	if (ret) {
		return ret;
    }

    assert(proc_context->ue_context == context);

	pdn = GET_PDN(context, ebi_index);

	/* Fill pfcp structure for pfcp delete request and send it */
	sequence = fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = pdn->dp_seid;


	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSDELREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, process_pfcp_sess_del_request_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;
    trans_entry->peer_sockaddr = context->upf_context->upf_sockaddr; 

    // link new transaction and proc context 
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;
    assert(proc_context->gtpc_trans != NULL);
    assert(proc_context->pfcp_trans != NULL);

	return 0;
}

// case 1: failed to delete context locally for whatever reason or failed to encode pfcp message  
// case 2: Received pfcp del response with -ve casue 
// case 3: failed to process pfcp del response 
// case 3: PFCP delete message timedout 
void
proc_detach_failure(proc_context_t *proc_context, msg_info_t *msg, uint8_t cause)
{
    increment_stat(PROCEDURES_SPGW_MME_INIT_DETACH_FAILURE);

    ds_error_response(proc_context, msg,
                      cause,
                      cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    proc_detach_complete(proc_context, msg);
}

/* free transactions. delink from procedure. Free procedure and de-link from subscriber */
void 
proc_detach_complete(proc_context_t *proc_context, msg_info_t *msg)
{
    end_procedure(proc_context);
    LOG_MSG(LOG_NEVER, "msg = %p", msg);
    return;
}

int8_t
process_pfcp_sess_del_resp(uint64_t sess_id, 
                           gtpv2c_header_t *gtpv2c_tx,
		                   gx_msg *ccr_request, 
                           uint16_t *msglen,
                           proc_context_t *proc_context )
{
	uint8_t ebi_index = 0;
	uint16_t msg_len = 0;
	ue_context_t *context = (ue_context_t *)proc_context->ue_context;
	del_sess_rsp_t del_resp = {0};
	pdn_connection_t *pdn =  NULL;

	pdn = (pdn_connection_t *)proc_context->pdn_context; 
    ebi_index = pdn->default_bearer_id - 5;

	/* Update the UE state */
	pdn->state = PFCP_SESS_DEL_RESP_RCVD_STATE;
	if (cp_config->gx_enabled) {

		/* Retrive Gx_context based on Sess ID. */
		ue_context_t *temp_context  = (ue_context_t *)get_ue_context_from_gxsessid((uint8_t *)pdn->gx_sess_id); 
		if (temp_context == NULL) {
			LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
			return -1;
		}
        assert(temp_context == context);

		/* VS: Set the Msg header type for CCR-T */
		ccr_request->msg_type = GX_CCR_MSG ;

		/* VS: Set Credit Control Request type */
		ccr_request->data.ccr.presence.cc_request_type = PRESENT;
		ccr_request->data.ccr.cc_request_type = TERMINATION_REQUEST ;

		/* VG: Set Credit Control Bearer opertaion type */
		ccr_request->data.ccr.presence.bearer_operation = PRESENT;
		ccr_request->data.ccr.bearer_operation = TERMINATION ;

		/* VS: Fill the Credit Crontrol Request to send PCRF */
		if(fill_ccr_request(&ccr_request->data.ccr, context, ebi_index, pdn->gx_sess_id) != 0) {
			LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
			return -1;
		}
		/* Update UE State */
		pdn->state = CCR_SNT_STATE;

		/* VS: Calculate the max size of CCR msg to allocate the buffer */
		*msglen = gx_ccr_calc_length(&ccr_request->data.ccr);

	}

    /* Block to create DSRSP message */
    {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 

        if(cp_config->cp_type == PGWC) {
            /* Fill gtpv2c structure for sending on s11 interface */
            set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
                    pdn->s5s8_sgw_gtpc_teid, gtpc_trans->sequence);
        } else {
            /* Fill gtpv2c structure for sending on s11 interface */
            set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
                    context->s11_mme_gtpc_teid, gtpc_trans->sequence);
        }

        set_cause_accepted(&del_resp.cause, IE_INSTANCE_ZERO);

        /*Encode the S11 delete session response message. */
        msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);

        gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
    }
    LOG_MSG(LOG_NEVER, " sess_id = %lu ", sess_id);
	return 0;
}

void
fill_pfcp_sess_mod_req_delete( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, ue_context_t *context, pdn_connection_t *pdn)
{
	uint32_t seq = 0;
	pdr_t *pdr_ctxt = NULL;
	eps_bearer_t *bearer;

#if 0
	int ret = 0;
	upf_context_t *upf_ctx = NULL;
	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		LOG_MSG(LOG_ERROR, "Error: %d , ue_context = %p ", ret, context);
		return;
	}
#endif
    LOG_MSG(LOG_NEVER, "context = %p ", context);

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	/*SP: Adding FAR IE*/
	pfcp_sess_mod_req->update_far_count = 0;
	for (int index = 0; index < pdn->num_bearer; index++){
		bearer = pdn->eps_bearers[index];
		if(bearer){
			for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
	}
	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}
    LOG_MSG(LOG_NEVER, "header = %p", header);
}
