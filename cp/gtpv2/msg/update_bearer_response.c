// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
#ifdef FUTURE_NEED
// saegw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler  
// pgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler 
// sgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT - process_update_bearer_response_handler
int handle_update_bearer_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    int ret = 0;


    if((ret = decode_upd_bearer_rsp((uint8_t *) gtpv2c_rx,
                    &msg->rx_msg.ub_rsp) == 0))
        return -1;

	if(msg->rx_msg.ub_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
			ubr_error_response(msg, msg->rx_msg.ub_rsp.cause.cause_value,
									cp_config->cp_type == SGWC ? S5S8_IFACE : GX_IFACE);
			return -1;
	}

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	uint8_t ebi_index = msg->rx_msg.ub_rsp.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;

	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	context  = (ue_context_t *)get_ue_context(teid_key);

	if ( context == NULL) {
		LOG_MSG(LOG_ERROR, "Entry not found for teid:%x...", teid_key);
		return -1;
	}
	pdn = GET_PDN(context , ebi_index);
	LOG_MSG(LOG_DEBUG, "Teid:%u, State:%s",
			teid_key, get_state_string(pdn->state));

	msg->proc = UPDATE_BEARER_PROC;
	msg->event = UPDATE_BEARER_RSP_RCVD_EVNT;

    return 0;
}

int
process_s11_upd_bearer_response(upd_bearer_rsp_t *ub_rsp)
{
	uint8_t bearer_id = 0;
	upd_bearer_rsp_t ubr_rsp = {0};
	pdn_connection_t *pdn_cntxt = NULL;
	ue_context_t *context = NULL;
	uint16_t payload_length = 0;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	pdn_cntxt = (pdn_connection_t *)get_pdn_context(ub_rsp->header.teid.has_teid.teid);

	if ( pdn_cntxt == NULL) {
		LOG_MSG(LOG_ERROR,"Entry not found for teid:%x...", ub_rsp->header.teid.has_teid.teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	resp = get_sess_entry_seid(pdn_cntxt->seid);
	if (resp == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", pdn_cntxt->seid);
		return -1;
	}

	context = (ue_context_t *)get_ue_context(ub_rsp->header.teid.has_teid.teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Error: %d ");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if(ub_rsp->uli.header.len){
		memcpy(&ubr_rsp.uli, &ub_rsp->uli, sizeof(gtp_user_loc_info_ie_t));
	}

	set_gtpv2c_teid_header((gtpv2c_header_t *) &ubr_rsp, GTP_UPDATE_BEARER_RSP,
	    pdn_cntxt->s5s8_pgw_gtpc_teid, ub_rsp->header.teid.has_teid.seq);

	set_cause_accepted(&ubr_rsp.cause, IE_INSTANCE_ZERO);

	ubr_rsp.bearer_context_count = ub_rsp->bearer_context_count;
	for(uint8_t i = 0; i < ub_rsp->bearer_context_count; i++){

		bearer_id = ub_rsp->bearer_contexts[i].eps_bearer_id.ebi_ebi - 5;
		resp->list_bearer_ids[resp->num_of_bearers++] = bearer_id + 5;
		set_ie_header(&ubr_rsp.bearer_contexts[i].header, GTP_IE_BEARER_CONTEXT,
			IE_INSTANCE_ZERO, 0);
		/* TODO  Remove hardcoded ebi */
		set_ebi(&ubr_rsp.bearer_contexts[i].eps_bearer_id, IE_INSTANCE_ZERO,
									ub_rsp->bearer_contexts[i].eps_bearer_id.ebi_ebi);
		ubr_rsp.bearer_contexts[i].header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

		set_cause_accepted(&ubr_rsp.bearer_contexts[i].cause, IE_INSTANCE_ZERO);
		ubr_rsp.bearer_contexts[i].header.len += sizeof(uint16_t) + IE_HEADER_SIZE;

	}

	uint16_t msg_len = 0;
	msg_len = encode_upd_bearer_rsp(&ubr_rsp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
	//send S5S8 interface update bearer response.
	gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
   	      		(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
                sizeof(struct sockaddr_in));

	/* Update UE State */
	pdn_cntxt->state = CONNECTED_STATE;

	/* Update UE Proc */
	pdn_cntxt->proc = UPDATE_BEARER_PROC;

	/* Set GX rar message */
	resp->msg_type = GTP_UPDATE_BEARER_RSP;
	resp->state = CONNECTED_STATE;
	resp->proc =  UPDATE_BEARER_PROC;
	return 0;
}

void 
process_s5s8_upd_bearer_response_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p ", data);
    return;
}
int
process_s5s8_upd_bearer_response(upd_bearer_rsp_t *ub_rsp)
{
	uint8_t ebi_index = 0;
	pdn_connection_t *pdn_cntxt = NULL;
	ue_context_t *context = NULL;
	uint32_t seq = 0;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	pdn_cntxt = (pdn_connection_t *)get_pdn_context(ub_rsp->header.teid.has_teid.teid);

	if ( pdn_cntxt == NULL) {
		LOG_MSG(LOG_ERROR,"Entry not found for teid:%x...", 
										ub_rsp->header.teid.has_teid.teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	resp = get_sess_entry_seid(pdn_cntxt->seid);
	if (resp == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", pdn_cntxt->seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	context = (ue_context_t *)get_ue_context(ub_rsp->header.teid.has_teid.teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Error: ");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	//Start filling sess_mod_req

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req.header),
							PFCP_SESSION_MODIFICATION_REQUEST, HAS_SEID, seq);

	pfcp_sess_mod_req.header.seid_seqno.has_seid.seid = pdn_cntxt->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req.cp_fseid), pdn_cntxt->seid, node_value);

	for(uint8_t i = 0; i < ub_rsp->bearer_context_count; i++){

		ebi_index = ub_rsp->bearer_contexts[i].eps_bearer_id.ebi_ebi - 5;

		fill_update_pdr(&pfcp_sess_mod_req, context->eps_bearers[ebi_index]);

	}

	uint8_t pfcp_msg[512] = {0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *)pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded,  process_s5s8_upd_bearer_response_pfcp_timeout);
    pdn_cntxt->trans_entry = trans_entry;

	/* Update UE State */
	pdn_cntxt->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	/* Update UE Proc */
	pdn_cntxt->proc = UPDATE_BEARER_PROC;

	/* Set GX rar message */
	resp->msg_type = GTP_UPDATE_BEARER_RSP;
	resp->state =  PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc =  UPDATE_BEARER_PROC;

	return 0;

}

/*UPDATE bearer */
int process_update_bearer_response_handler(void *data, void *unused_param)
{
	int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_s5s8_upd_bearer_response(&msg->rx_msg.ub_rsp);
	if(ret && ret != -1)
		ubr_error_response(msg, ret, GX_IFACE);

	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return -1;
	}

	return 0;
}

#endif
