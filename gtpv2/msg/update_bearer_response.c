// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifdef FUTURE_NEED
// saegw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler  
// pgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler 
// sgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT - process_update_bearer_response_handler
int handle_update_bearer_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    int ret = 0;

    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);

    if((ret = decode_upd_bearer_rsp((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.ub_rsp) == 0))
        return -1;

	if(msg->gtpc_msg.ub_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
			ubr_error_response(msg, msg->gtpc_msg.ub_rsp.cause.cause_value,
									cp_config->cp_type == SGWC ? S5S8_IFACE : GX_IFACE);
			return -1;
	}

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	uint8_t ebi_index = msg->gtpc_msg.ub_rsp.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;
	//Vikrant Which ebi to be selected as multiple bearer in request
	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid ,ebi_index)) > 0){
			msg->state = ret;
	}else{
		return -1;
	}
	msg->proc = UPDATE_BEARER_PROC;
	msg->event = UPDATE_BEARER_RSP_RCVD_EVNT;

    return 0;
}

int
process_s11_upd_bearer_response(upd_bearer_rsp_t *ub_rsp)
{
	int ret = 0;
	uint8_t bearer_id = 0;
	upd_bearer_rsp_t ubr_rsp = {0};
	pdn_connection_t *pdn_cntxt = NULL;
	ue_context_t *context = NULL;
	uint16_t payload_length = 0;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = get_pdn(ub_rsp->header.teid.has_teid.teid, &pdn_cntxt);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,"%s:Entry not found for teid:%x...\n", __func__, ub_rsp->header.teid.has_teid.teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if (get_sess_entry(pdn_cntxt->seid, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, pdn_cntxt->seid);
		return -1;
	}

	ret = get_ue_context(ub_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
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
       			s5s8_sockaddr_len);

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
    RTE_SET_USED(data);
    return;
}
int
process_s5s8_upd_bearer_response(upd_bearer_rsp_t *ub_rsp)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	pdn_connection_t *pdn_cntxt = NULL;
	ue_context_t *context = NULL;
	uint32_t seq = 0;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ret = get_pdn(ub_rsp->header.teid.has_teid.teid, &pdn_cntxt);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,"%s:Entry not found for teid:%x...\n", __func__,
										ub_rsp->header.teid.has_teid.teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if (get_sess_entry(pdn_cntxt->seid, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
										__func__, __LINE__, pdn_cntxt->seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	ret = get_ue_context(ub_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
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

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
	else
	{
		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_mod_req.header.message_type,SENT,SX);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded,  process_s5s8_upd_bearer_response_pfcp_timeout);
        pdn_cntxt->trans_entry = trans_entry;
	}
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
	if (SGWC == cp_config->cp_type) {

		ret = process_s11_upd_bearer_response(&msg->gtpc_msg.ub_rsp);
		if(ret && ret != -1)
				ubr_error_response(msg, ret, S5S8_IFACE);
	} else {

		ret = process_s5s8_upd_bearer_response(&msg->gtpc_msg.ub_rsp);
		if(ret && ret != -1)
				ubr_error_response(msg, ret, GX_IFACE);
	}
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

#endif
