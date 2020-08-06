// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED
// saegw DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_create_bearer_resp_handler
// pgw - DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_cbresp_handler
// sgw  DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT ==> process_create_bearer_resp_handler 
int handle_create_bearer_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    int ret = 0;

    if((ret = decode_create_bearer_rsp((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.cb_rsp) == 0))
        return -1;


    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(context);
    RTE_SET_USED(msg);

    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
	uint8_t ebi_index = msg->gtpc_msg.cb_rsp.bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid ,ebi_index)) > 0){
		msg->state = ret;
	}else{
		return -1;
	}

	msg->proc = DED_BER_ACTIVATION_PROC;
	msg->event = CREATE_BER_RESP_RCVD_EVNT;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_pgwc_create_bearer_rsp(create_bearer_rsp_t *cb_rsp)
{
	uint8_t ret;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	uint8_t ebi_index;

	ret = get_ue_context(cb_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
					__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if (NULL == bearer)
	{
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

	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
	else
	{

		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_mod_req.header.message_type,SENT,SX);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pgwc_create_bearer_rsp_pfcp_timeout);
        bearer->pdn->trans_entry = trans_entry; 
	}

	context->sequence = seq_no;
	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if (get_sess_entry(context->pdns[0]->seid, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	resp->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	resp->msg_type = GTP_CREATE_BEARER_RSP;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_pgwc_create_bearer_rsp(create_bearer_rsp_t *cb_rsp)
{
	uint8_t ret;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	uint8_t ebi_index;

	ret = get_ue_context(cb_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
					__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if (NULL == bearer)
	{
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

	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
	else
	{

		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_mod_req.header.message_type,SENT,SX);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pgwc_create_bearer_rsp_pfcp_timeout);
        bearer->pdn->trans_entry = trans_entry; 
	}

	context->sequence = seq_no;
	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if (get_sess_entry(context->pdns[0]->seid, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	resp->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	resp->msg_type = GTP_CREATE_BEARER_RSP;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

void process_sgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_sgwc_create_bearer_rsp(create_bearer_rsp_t *cb_rsp)
{
	int ret;
	uint8_t ebi_index;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ret = get_ue_context(cb_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if(bearer == NULL)
	{
		/* TODO:
		 * This mean ebi we allocated and received doesnt match
		 * In correct design match the bearer in transtient struct from sgw-u teid
		 * */
		return -1;
	}

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

	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);


	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
	else
	{

		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_mod_req.header.message_type,SENT,SX);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_sgwc_create_bearer_rsp_pfcp_timeout);
        bearer->pdn->trans_entry = trans_entry; 
	}

	context->sequence = seq_no;
	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if (get_sess_entry(bearer->pdn->seid, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	resp->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	resp->msg_type = GTP_CREATE_BEARER_RSP;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

#endif
