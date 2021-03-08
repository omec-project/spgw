// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "gtp_messages.h"
#include "util.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_session.h"

#ifdef FUTURE_NEED_SGW
// saegw DETACH_PROC DS_REQ_SNT_STATE DS_RESP_RCVD_EVNT => process_ds_resp_handler
// sgw DETACH_PROC DS_REQ_SNT_STATE DS_RESP_RCVD_EVNT : process_ds_resp_handler 
int handle_delete_session_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    uint8_t ebi_index = 5; // ajay - todo use transaction 

    ret = decode_del_sess_rsp((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.ds_rsp);
    if(ret == 0)
        return -1;


	gtpc_delete_timer_entry(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid);

	if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid, &context) != 0)
	 {

		ds_error_response(proc_context, msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	if(msg->gtpc_msg.ds_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		LOG_MSG(LOG_ERROR, "Cause Req Error : msg type :%u, cause ie : %u ",
				msg->msg_type, msg->gtpc_msg.ds_rsp.cause.cause_value);

		 ds_error_response(proc_context, msg, msg->gtpc_msg.ds_rsp.cause.cause_value,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}
	/*Set the appropriate procedure and state.*/
	msg->state = context->pdns[ebi_index]->state;
	msg->proc = context->pdns[ebi_index]->proc;
	/*Set the appropriate event type.*/
	msg->event = DS_RESP_RCVD_EVNT;

#if 0
	msg->state = DS_REQ_SNT_STATE;
	msg->proc =  DETACH_PROC;
	/*Set the appropriate event type.*/
	msg->event = DS_RESP_RCVD_EVNT;
#endif

	LOG_MSG(LOG_DEBUG, "Callback called for "
		"Msg_Type:%s[%u], Teid:%u, "
		"Procedure:%s, State:%s, Event:%s",
		gtp_type_str(msg->msg_type), msg->msg_type,
		msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid,
		get_proc_string(msg->proc),
		get_state_string(msg->state), get_event_string(msg->event));
#if 0
// SGW egress ???
			msg->state = context->pdns[ebi_index]->state;
			msg->proc = context->pdns[ebi_index]->proc;

			/*Set the appropriate event type.*/
			msg->event = DS_RESP_RCVD_EVNT;


#endif
    return 0;
}

int
process_sgwc_s5s8_delete_session_response(del_sess_rsp_t *dsr, uint8_t *gtpv2c_tx)
{
	uint16_t msg_len = 0;
	uint64_t seid = 0;
	ue_context_t *context = NULL;
	del_sess_rsp_t del_resp = {0};

	int ret = delete_sgwc_context(dsr->header.teid.has_teid.teid, &context, &seid);
	if (ret){
		return ret;
	}
	set_gtpv2c_header(&del_resp.header, dsr->header.gtpc.teid_flag, GTP_DELETE_SESSION_RSP,
								 context->s11_mme_gtpc_teid, dsr->header.teid.has_teid.seq);

	set_cause_accepted(&del_resp.cause, IE_INSTANCE_ZERO);

	msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;
	header->gtpc.message_len = htons(msg_len - 4);

	LOG_MSG(LOG_DEBUG, "s11_mme_sockaddr.sin_addr.s_addr :%s", 
			inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));

	/* Delete entry from session entry */
	if (del_sess_entry_seid(seid) != 0){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for Key sess ID:%lu", seid);
		return -1;
	}

	/* Delete UE context entry from UE Hash */
	if (ue_context_delete_entry_imsiKey(context->imsi) < 0){
	LOG_MSG(LOG_ERROR,
			"%s - Error on ue_context_by_fteid_hash deletion",
			strerror(ret));
	}

	free(context);
	return 0;
}


int
process_sgwc_s5s8_delete_session_response(del_sess_rsp_t *ds_resp)
{
	int ret = 0;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	//int ebi_index = 0;

	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
	fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	//int ret = delete_sgwc_context(ds_req->header.teid.has_teid.teid, &context, &seid);
	//if (ret)
	//	return ret;

	/* Retrieve the UE context */
	ret = get_ue_context_by_sgw_s5s8_teid(ds_resp->header.teid.has_teid.teid, &context);
	if (ret < 0) {
		LOG_MSG(LOG_ERROR, "Failed to get UE State for teid: %u",
				ds_resp->header.teid.has_teid.teid);
	}
	bearer = (eps_bearer_t *)get_bearer_by_teid(ds_resp->header.teid.has_teid.teid);
	if(bearer == NULL) {
	    LOG_MSG(LOG_ERROR, "Bearer Entry not found for teid:%x", ds_resp->header.teid.has_teid.teid);
	    return -1;
	}
	// ebi_index = UE_BEAR_ID(bearer->pdn->seid) -5;
	//pfcp_sess_del_req.header.seid_seqno.has_seid.seid =
	//	SESS_ID(ds_resp->header.teid.has_teid.teid,ds_resp->lbi.ebi_ebi);

	uint8_t pfcp_msg[512]={0};

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = bearer->pdn->dp_seid;
	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg,encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXA_SESSDELREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_sgwc_s5s8_delete_session_request_pfcp_timeout);
    bearer->pdn->trans_entry = trans_entry; 

	/* Update UE State */
	bearer->pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;

	/* VS: Stored/Update the session information. */
	resp = get_sess_entry_seid(bearer->pdn->seid);
	if (resp == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to get response entry in SM_HASH");
		return -1;
	}

	resp->msg_type = GTP_DELETE_SESSION_REQ;
	resp->state = PFCP_SESS_DEL_REQ_SNT_STATE;

	return 0;
}

int
process_ds_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	if (cp_config->cp_type == SGWC) {
		ret = process_sgwc_s5s8_delete_session_response(&msg->gtpc_msg.ds_rsp);
		if (ret) {
			if(ret  != -1)
				ds_error_response(proc_context, msg, ret,
						           cp_config->cp_type != PGWC ? S11_IFACE :S5S8_IFACE);
			/* Error handling not implemented */
			LOG_MSG(LOG_ERROR, "Error: %d ", ret);
			return -1;
		}
	} else {
		/*Code should not reach here since this handler is only for SGWC*/
		return -1;
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;
}

//int
//process_sgwc_s5s8_delete_session_response(gtpv2c_header *gtpv2c_rx,
//	gtpv2c_header *gtpv2c_tx)
//{
//	uint16_t msg_len = 0;
//	uint64_t seid = 0;
//	ue_context_t *context = NULL;
//	del_sess_rsp_t del_resp = {0};
//
//	int ret = delete_sgwc_context(gtpv2c_rx, &context, &seid);
//	if (ret)
//		return ret;
//
//	gtpv2c_rx->teid_u.has_teid.seq = bswap_32(gtpv2c_rx->teid_u.has_teid.seq) >> 8 ;
//	/*VS: Encode the S11 delete session response message. */
//	set_gtpv2c_teid_header((gtpv2c_header *) &del_resp, GTP_DELETE_SESSION_RSP,
//			context->s11_mme_gtpc_teid, gtpv2c_rx->teid_u.has_teid.seq);
//	set_cause_accepted_ie((gtpv2c_header *) &del_resp, IE_INSTANCE_ZERO);
//
//	del_resp.cause.header.len = ntohs(del_resp.cause.header.len);
//	/*VS: Encode the S11 delete session response message. */
//	msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);
//
//	gtpv2c_tx->gtpc.length = htons(msg_len - 4);
//
//	s11_mme_sockaddr.sin_addr.s_addr =
//					htonl(context->s11_mme_gtpc_ipv4.s_addr);
//
//	LOG_MSG(LOG_DEBUG, "s11_mme_sockaddr.sin_addr.s_addr :%s", 
//				inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));
//
//	/* Delete entry from session entry */
//	if (del_sess_entry_seid(seid) != 0){
//		LOG_MSG(LOG_ERROR, "NO Session Entry Found for Key sess ID:%lu", seid);
//		return -1;
//	}
//
//	/* Delete UE context entry from UE Hash */
//	/*free(context);*/
//	return 0;
//}
#endif


