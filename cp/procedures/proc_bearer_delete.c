// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED
#include "gx_interface.h"

uint8_t
process_delete_bearer_pfcp_sess_response(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
	ue_context_t *context = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);
	uint8_t bearer_id = UE_BEAR_ID(sess_id) - 5;

	context = (ue_context_t *)get_sess_entry_seid(sess_id);
	if (context == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", sess_id);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	context  = (ue_context_t *)get_ue_context(teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR,"Failed to update UE State for teid: %u", teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	context->pdns[bearer_id]->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	if (resp->msg_type == GX_RAR_MSG) {
		uint8_t lbi = 0;
		uint8_t bearer_count = 0;
		uint8_t eps_bearer_ids[MAX_BEARERS];

        if(cp_config->gx_enabled) {
            get_charging_rule_remove_bearer_info(
                    context->pdns[bearer_id],
                    &lbi, eps_bearer_ids, &bearer_count);
        }

		set_delete_bearer_request(gtpv2c_tx, context->sequence,
			context, lbi, eps_bearer_ids, bearer_count);

		resp->state = DELETE_BER_REQ_SNT_STATE;
		context->pdns[bearer_id]->state = DELETE_BER_REQ_SNT_STATE;

		if (SAEGWC == cp_config->cp_type) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		} else {
			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(context->pdns[bearer_id]->s5s8_sgw_gtpc_ipv4.s_addr);
		}

	} else if (resp->msg_type == GTP_DELETE_BEARER_REQ) {
		set_delete_bearer_request(gtpv2c_tx, context->sequence,
			context, resp->linked_eps_bearer_id,
			resp->eps_bearer_ids, resp->bearer_count);

		resp->state = DELETE_BER_REQ_SNT_STATE;
		context->pdns[bearer_id]->state = DELETE_BER_REQ_SNT_STATE;

		s11_mme_sockaddr.sin_addr.s_addr =
			htonl(context->s11_mme_gtpc_ipv4.s_addr);

	} else if (resp->msg_type == GTP_DELETE_BEARER_RSP) {
        if ((SAEGWC == cp_config->cp_type) ||
                (PGWC == cp_config->cp_type)) {
            if(cp_config->gx_enabled) {
                delete_dedicated_bearers(context->pdns[bearer_id],
                        resp->eps_bearer_ids, resp->bearer_count);

                gen_reauth_response(context, resp->eps_bearer_id - 5);

                resp->state = CONNECTED_STATE;
                resp->msg_type = GX_RAA_MSG;
                context->pdns[resp->eps_bearer_id - 5]->state = CONNECTED_STATE;

                s11_mme_sockaddr.sin_addr.s_addr =
                    htonl(context->s11_mme_gtpc_ipv4.s_addr);
            }
		} else {
			delete_dedicated_bearers(context->pdns[bearer_id],
				resp->eps_bearer_ids, resp->bearer_count);

			set_delete_bearer_response(gtpv2c_tx, context->sequence,
				resp->linked_eps_bearer_id,
				resp->eps_bearer_ids, resp->bearer_count,
				context->pdns[bearer_id]->s5s8_pgw_gtpc_teid);

			resp->state = CONNECTED_STATE;
			context->pdns[bearer_id]->state = CONNECTED_STATE;

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(context->pdns[bearer_id]->s5s8_pgw_gtpc_ipv4.s_addr);
		}
	}

	return 0;
}

/* dedicated bearer - deactivation PDN initiated deactivation */
int
process_pfcp_sess_mod_resp_dbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;
    ue_context_t  *ue = msg->ue_context; 

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_delete_bearer_pfcp_sess_response(
		msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
		gtpv2c_tx);
	if (ret != 0) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

    ue_context_t *temp = NULL;
	temp = (ue_context_t*)get_sess_entry_seid(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
    assert(temp == ue_context);

	if ((SAEGWC != cp_config->cp_type) &&
		((resp->msg_type == GTP_DELETE_BEARER_RSP) ||
		(resp->msg_type == GX_RAR_MSG))) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		        sizeof(struct sockaddr_in));

        increment_pgw_peer_stats(MSG_TX_GTPV2_DBRSP, my_sock.s5s8_recv_sockaddr);

		if (resp->msg_type != GTP_DELETE_BEARER_RSP) {
			add_gtpv2c_if_timer_entry(
				UE_SESS_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
				&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
				UE_BEAR_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
				S5S8_IFACE);
		}

	} else if (resp->msg_type != GX_RAA_MSG) {
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,
				sizeof(struct sockaddr_in));

		add_gtpv2c_if_timer_entry(
				UE_SESS_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
				&s11_mme_sockaddr, gtp_tx_buf, payload_length,
				UE_BEAR_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
				S11_IFACE);

        increment_mme_peer_stats(MSG_TX_GTPV2_DBREQ, s11_mme_sockaddr.sin_addr.s_addr);
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);

	return 0;
}

int
process_pfcp_sess_del_resp_dbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_delete_bearer_pfcp_sess_response(
		msg->rx_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
		gtpv2c_tx);
	if (ret != 0) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);


	resp = get_sess_entry_seid(msg->rx_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
	if (resp == NULL) {
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu",
			msg->rx_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);

		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if ((SAEGWC != cp_config->cp_type) &&
		((resp->msg_type == GTP_DELETE_BEARER_RSP))) {
			gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
		            (struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		            sizeof(struct sockaddr_in));

		if (resp->msg_type == GTP_DELETE_BEARER_RSP) {
            increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_DBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
		}
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);

	return 0;
}

/*DELETE bearer commaand deactivation*/

/*
 * This handler is called when CCA-U is received on PGWC.
 * and PGWC will send session modification to PGWU.
 * On Combined, SAEGWC will send this to SAEGWU.
 *
*/

int del_bearer_cmd_ccau_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint32_t call_id = 0;
	pdn_connection_t *pdn = NULL;

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&msg->rx_msg.cca.session_id.val, &call_id);
	if (ret < 0) {
	        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s", 
	                       (char*) &msg->rx_msg.cca.session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn = (pdn_connection_t *)get_pdn_conn_entry(call_id);
	if (pdn == NULL)
	{
	      LOG_MSG(LOG_ERROR, "No valid pdn cntxt found for CALL_ID:%u", call_id);
	      return -1;
	}


	ret = process_sess_mod_req_del_cmd(pdn);
	if (ret != 0) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}
    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;
}

/* This handler is called when SGWC-PGWC-SAEWC receives
 * delete bearer response from MME-SGWC-MME
 *
 */

int
process_delete_bearer_response_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	ret = process_delete_bearer_resp(&msg->rx_msg.db_rsp, 1);
	if (ret != 0) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}
    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;
}

/*
 * This handler will be called when PFCP MOD is received from
 * PGWU on PGWC
 * On combined it will be recieved on SAEGWC
 * */

int
del_bearer_cmd_mbr_resp_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint8_t flag = -1;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;
	ret = process_pfcp_sess_mod_resp_del_cmd
			(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			 gtpv2c_tx ,&flag);

	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return ret;
	}
	if(flag == 0){
		return 0;
	}
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);


	if (PGWC == cp_config->cp_type ) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		        sizeof(struct sockaddr_in));
        increment_sgw_peer_stats(MSG_TX_GTPV2_DBREQ, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
	} else if (SAEGWC == cp_config->cp_type) {

        increment_mme_peer_stats(MSG_TX_GTPV2_DBREQ, s11_mme_sockaddr.sin_addr.s_addr);
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,  /* need change - future */
				sizeof(struct sockaddr_in));
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;
}

void
process_pfcp_sess_del_request_delete_bearer_rsp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p", data);
    return;
}

int
process_pfcp_sess_del_request_delete_bearer_rsp(del_bearer_rsp_t *db_rsp)
{
	int ret = 0;
	ue_context_t *context = NULL;
	uint32_t s5s8_pgw_gtpc_teid = 0;
	uint32_t s5s8_pgw_gtpc_ipv4 = 0;
	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
	uint64_t ebi_index = db_rsp->lbi.ebi_ebi - 5;

	ret = delete_context(db_rsp->lbi, db_rsp->header.teid.has_teid.teid,
		&context, &s5s8_pgw_gtpc_teid, &s5s8_pgw_gtpc_ipv4);
	if (ret)
		return ret;

	/* Fill pfcp structure for pfcp delete request and send it */
	fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = context->pdns[ebi_index]->dp_seid;

	uint8_t pfcp_msg[512]={0};

	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSDELREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pfcp_sess_del_request_delete_bearer_rsp_timeout);
    pdn->trans_entry = trans_entry;

	/* Update UE State */
	context->pdns[ebi_index]->state = PFCP_SESS_DEL_REQ_SNT_STATE;

	/* Lookup entry in hash table on the basis of session id*/
	context = (ue_context_t *)get_sess_entry_seid(context->pdns[ebi_index]->seid);
    if(context == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu",
			context->pdns[ebi_index]->seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	resp->eps_bearer_id = db_rsp->lbi.ebi_ebi;
	resp->msg_type = GTP_DELETE_BEARER_RSP;
	resp->state = PFCP_SESS_DEL_REQ_SNT_STATE;
	resp->proc = context->pdns[ebi_index]->proc;
	return 0;
}

int
process_pfcp_sess_mod_resp_del_cmd(uint64_t sess_id, gtpv2c_header_t *gtpv2c_rx ,uint8_t *is_del_cmd)
{
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);

	/* Retrive the session information based on session id. */
	resp = get_sess_entry_seid(sess_id, &resp); 
	if (resp == NULL ){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", sess_id);
		return -1;
	}

	if(resp->msg_type == GTP_DELETE_BEARER_CMD){
		context = (ue_context_t *)get_ue_context(teid);
		if (context == NULL) {
			LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
		}
		resp->state = DELETE_BER_REQ_SNT_STATE;
		ebi_index = resp->eps_bearer_id;
		bearer = context->eps_bearers[ebi_index];
		bearer->pdn->state = DELETE_BER_REQ_SNT_STATE;
		set_delete_bearer_request(gtpv2c_rx, context->sequence,
				context, 0, resp->eps_bearer_ids, resp->bearer_count);

		if(cp_config->cp_type == PGWC) {
			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(bearer->pdn->s5s8_sgw_gtpc_ipv4.s_addr);
		} else if (cp_config->cp_type == SAEGWC) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		}
		resp->proc = context->eps_bearers[ebi_index]->pdn->proc;
	}else {
		*is_del_cmd = 0;

		/* Get ue context and update state to connected state */
		context = (ue_context_t *) get_ue_context(teid);
		if (context == NULL) {
			LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
		}

		uint8_t ebi_index = UE_BEAR_ID(sess_id);
		context->pdns[ebi_index - 5]->state = CONNECTED_STATE;

		/*  update state to connected state in resp */
		resp->state = CONNECTED_STATE;
	}
	return 0;
}

void
process_pfcp_sess_mod_del_cmd_timeout(void *data)
{

}

int
process_sess_mod_req_del_cmd(pdn_connection_t *pdn)
{
	ue_context_t *context = NULL;
	eps_bearer_t *bearers[MAX_BEARER];
	int ebi = 0;
	struct resp_info *resp = NULL;
	int teid = UE_SESS_ID(pdn->seid);
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	int ebi_index = 0;

	context = (ue_context_t *)get_ue_context(teid);

	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
	}

	resp = get_sess_entry_seid(pdn->seid);
	if (resp == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", pdn->seid);
		return -1;
	}
	ebi_index = resp->eps_bearer_id;
	s11_mme_sockaddr.sin_addr.s_addr =
		context->s11_mme_gtpc_ipv4.s_addr;

	for (uint8_t iCnt = 0; iCnt < resp->bearer_count; ++iCnt) {
		ebi = resp->eps_bearer_ids[iCnt];
		bearers[iCnt] = context->eps_bearers[ebi - 5];

	}

	fill_pfcp_sess_mod_req_pgw_del_cmd_update_far(&pfcp_sess_mod_req ,pdn, bearers, resp->bearer_count);

	uint8_t pfcp_msg[size]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pfcp_sess_mod_del_cmd_timeout);
    pdn->trans_entry = trans_entry;

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	/* Update the sequence number */
	context->sequence = resp->gtpc_msg.del_bearer_cmd.header.teid.has_teid.seq;


//	resp->gtpc_msg.del_bearer_cmd = *del_bearer_cmd;
//	resp->eps_bearer_id = ebi_index;
	resp->s5s8_pgw_gtpc_ipv4 = htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);
	resp->msg_type = GTP_DELETE_BEARER_CMD;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = pdn->proc;
//	resp->bearer_count = del_bearer_cmd->bearer_count;
//	for (uint8_t iCnt = 0; iCnt < del_bearer_cmd->bearer_count; ++iCnt) {
//		resp->eps_bearer_ids[iCnt] = del_bearer_cmd->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
//	}
	return 0;
}

/**
 * @brief  : Generate reauth response
 * @param  : context , pointer to ue context structure
 * @param  : ebi_index, index in array where eps bearer is stored
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
gen_reauth_response(ue_context_t *context, uint8_t ebi_index)
{
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg raa = {0};
	pdn_connection_t *pdn = NULL;
	uint16_t msg_type_ofs = 0;
	uint16_t msg_body_ofs = 0;
	uint16_t rqst_ptr_ofs = 0;
	uint16_t msg_len_total = 0;

	pdn = context->eps_bearers[ebi_index]->pdn;

	raa.data.cp_raa.session_id.len = strlen(pdn->gx_sess_id);
	memcpy(raa.data.cp_raa.session_id.val, pdn->gx_sess_id, raa.data.cp_raa.session_id.len);

	raa.data.cp_raa.presence.session_id = PRESENT;

	/* VS: Set the Msg header type for CCR */
	raa.msg_type = GX_RAA_MSG;

	/* Result code */
	raa.data.cp_raa.result_code = 2001;
	raa.data.cp_raa.presence.result_code = PRESENT;

	/* Update UE State */
	pdn->state = RE_AUTH_ANS_SNT_STATE;

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_raa_calc_length(&raa.data.cp_raa);
	msg_body_ofs = sizeof(raa.msg_type) + sizeof(raa.seq_num);
	rqst_ptr_ofs = msg_len + msg_body_ofs;
	msg_len_total = rqst_ptr_ofs + sizeof(pdn->rqst_ptr);

	buffer = (char *)calloc(1, msg_len_total);
	if (buffer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
		return -1;
	}

	memcpy(buffer + msg_type_ofs, &raa.msg_type, sizeof(raa.msg_type));

	//if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + sizeof(raa.msg_type)), msg_len) == 0 )
	if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + msg_body_ofs), msg_len) == 0 )
		LOG_MSG(LOG_DEBUG,"RAA Packing failure");

	//memcpy((unsigned char *)(buffer + sizeof(raa.msg_type) + msg_len), &(context->eps_bearers[1]->rqst_ptr),
	memcpy((unsigned char *)(buffer + rqst_ptr_ofs), &(pdn->rqst_ptr),
			sizeof(pdn->rqst_ptr));
#if 0
	LOG_MSG(LOG_DEBUG,"While packing RAA %p %p", (void*)(context->eps_bearers[1]->rqst_ptr),
			*(void**)(buffer+rqst_ptr_ofs));

	LOG_MSG(LOG_DEBUG,"msg_len_total [%d] msg_type_ofs[%d] msg_body_ofs[%d] rqst_ptr_ofs[%d]",
			msg_len_total, msg_type_ofs, msg_body_ofs, rqst_ptr_ofs);
#endif
	/* VS: Write or Send CCR msg to Gx_App */
	gx_send(my_sock.gx_app_sock, buffer, // need change
			msg_len_total);
			//msg_len + sizeof(raa.msg_type) + sizeof(unsigned long));

	return 0;
}

int
delete_dedicated_bearers(pdn_connection_t *pdn,
		uint8_t bearer_ids[], uint8_t bearer_cntr)
{
	eps_bearer_t *ded_bearer = NULL;

	/* Delete multiple dedicated bearer of pdn */
	for (int iCnt = 0; iCnt < bearer_cntr; ++iCnt) {

		uint8_t ebi = bearer_ids[iCnt] - 5;

		/* Fetch dynamic rules from bearer and delete from hash */
		ded_bearer = pdn->eps_bearers[ebi];

		/* Traverse all dynamic filters from bearer */
		for (uint8_t index = 0; index < ded_bearer->num_dynamic_filters; ++index) {
			rule_name_key_t rule_name = {0};
			strncpy(rule_name.rule_name,
					ded_bearer->dynamic_rules[index]->rule_name,
					strlen(ded_bearer->dynamic_rules[index]->rule_name));
			sprintf(rule_name.rule_name, "%s%d",
					rule_name.rule_name, pdn->call_id);

			/* Delete rule name from hash */
			if (del_rule_name_entry(rule_name.rule_name)) {
				/* TODO: Error handling rule not found */
				return -1;
			}
		}

		/* Delete PDR, QER of bearer */
		if (del_rule_entries(pdn->context, ebi)) {
			/* TODO: Error message handling in case deletion failed */
			return -1;
		}

		pdn->num_bearer--;
		free(pdn->eps_bearers[ebi]);
		pdn->eps_bearers[ebi] = NULL;
		pdn->context->eps_bearers[ebi] = NULL;
	}

	return 0;
}
#endif
