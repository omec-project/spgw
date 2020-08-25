// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED
int
process_pfcp_sess_mod_resp_cbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_mod_resp(
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
			/* TODO for cbr
			 * mbr_error_response(&msg->gtpc_msg.mbr, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE); */
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
																			&resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if ((SAEGWC != cp_config->cp_type) && ((resp->msg_type == GTP_CREATE_BEARER_RSP) ||
			(resp->msg_type == GX_RAR_MSG))){
	    gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));
		if(resp->msg_type != GTP_CREATE_BEARER_RSP){
			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&(my_sock.s5s8_recv_sockaddr), gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
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
		if(resp->msg_type != GX_RAA_MSG) {
		    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
		            (struct sockaddr *) &s11_mme_sockaddr,
		            sizeof(struct sockaddr_in));

			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&s11_mme_sockaddr, gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S11_IFACE);

            increment_mme_peer_stats(MSG_TX_GTPV2_S11_CBREQ, s11_mme_sockaddr.sin_addr.s_addr);
		}
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_cbresp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_create_bearer_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_create_bearer_request(&msg->gtpc_msg.cb_req);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_create_bearer_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_sgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

void 
process_create_bearer_request_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_create_bearer_request(create_bearer_req_t *cbr)
{
	int ret;
	uint8_t ebi_index = 0;
	uint8_t new_ebi_index = 0;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ret = get_ue_context_by_sgw_s5s8_teid(cbr->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	bearer = rte_zmalloc_socket(NULL, sizeof(eps_bearer_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (bearer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate bearer "
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
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

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
	else
	{

        increment_userplane_stats(MSG_TX_PFCP_SXA_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_create_bearer_request_pfcp_timeout);
        pdn->trans_entry = trans_entry;
	}

	context->sequence = seq_no;
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
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

void
process_create_bearer_resp_and_send_raa( int sock )
{
	char *send_buf =  NULL;
	uint32_t buflen ;

	gx_msg *resp = malloc(sizeof(gx_msg));
	memset(resp, 0, sizeof(gx_msg));

	/* Filling Header value of RAA */
	resp->msg_type = GX_RAA_MSG ;
	//create_bearer_resp_t cbresp = {0};

	//fill_raa_msg( &(resp->data.cp_raa), &cbresp );

	/* Cal the length of buffer needed */
	buflen = gx_raa_calc_length (&resp->data.cp_raa);

	send_buf = malloc( buflen );
	memset(send_buf, 0, buflen);

	/* encoding the raa header value to buffer */
	memcpy( send_buf, &resp->msg_type, sizeof(resp->msg_type));

	if ( gx_raa_pack(&(resp->data.cp_raa), (unsigned char *)(send_buf + sizeof(resp->msg_type)), buflen ) == 0 )
		clLog(gxlogger, eCLSeverityDebug,"RAA Packing failure on sock [%d] \n", sock);

	//send_to_ipc_channel( sock, send_buf );
}


#endif
