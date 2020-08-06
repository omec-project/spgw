// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

// saegw, INITIAL_PDN_ATTACH_PROC, CS_REQ_SNT_STATE, CS_RESP_RCVD_EVNT, process_cs_resp_handler
// saegw, SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT -> process_cs_resp_handler 
// pgw - INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 
// pgw - SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw   INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 

#ifdef FUTURE_NEED_SGW
int handle_create_session_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
    ret = decode_create_sess_rsp((uint8_t *)gtpv2c_rx, &msg->gtpc_msg.cs_rsp);
    if(!ret)
        return -1;

	gtpc_delete_timer_entry(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid);

	if(msg->gtpc_msg.cs_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		cs_error_response(msg, msg->gtpc_msg.cs_rsp.cause.cause_value,
				        cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

    // Requirement : teid can be 0 from PDN GW 
	if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid, &context) != 0)
	{
		cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
					cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	uint8_t ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
	pdn_connection_t *pdn = GET_PDN(context, ebi_index);
	msg->state = pdn->state;
	msg->proc = pdn->proc;

	/*Set the appropriate event type.*/
	msg->event = CS_RESP_RCVD_EVNT;

	update_sys_stat(number_of_users, INCREMENT);
	update_sys_stat(number_of_active_session, INCREMENT);

	clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					gtpv2c_rx->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
#if 0
			gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

			/* Retrive UE Context */
			if (get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
				cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
																		  S5S8_IFACE);
				return -1;
			}

			msg->state = context->pdns[ebi_index]->state;
			msg->proc = context->pdns[ebi_index]->proc;

			/*Set the appropriate event type.*/
			msg->event = CS_RESP_RCVD_EVNT;

			clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					gtpv2c_rx->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));

#endif

    return 0;
}

void process_sgwc_s5s8_create_sess_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_sgwc_s5s8_create_sess_rsp(create_sess_rsp_t *cs_rsp)
{
	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};


	/*CLI logic : add PGWC entry when CSResponse received*/
	if ((add_node_conn_entry(ntohl(cs_rsp->pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc.ipv4_address),
			S5S8_SGWC_PORT_ID)) != 0) {
		clLog(clSystemLog, eCLSeverityDebug, "Fail to add connection entry for PGWC");
	}

	uint8_t ebi_index = cs_rsp->bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;

	ret = get_ue_context_by_sgw_s5s8_teid(cs_rsp->header.teid.has_teid.teid,
						&context);

	if (ret < 0 || !context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	pdn = context->eps_bearers[ebi_index]->pdn;
	{
		struct in_addr ip = {0};
        struct in_addr *temp;
		pdn->apn_restriction = cs_rsp->apn_restriction.rstrct_type_val;

		temp = (struct in_addr *)(&cs_rsp->paa.pdn_addr_and_pfx[0]);
        ip = *temp;

		pdn->ipv4.s_addr = htonl(ip.s_addr);
		pdn->s5s8_pgw_gtpc_ipv4.s_addr =
			cs_rsp->pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc.ipv4_address;
		pdn->s5s8_pgw_gtpc_teid =
			cs_rsp->pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc.teid_gre_key;
	}

	bearer = context->eps_bearers[ebi_index];
	{
		/* TODO: Implement TFTs on default bearers
		 *          if (create_s5s8_session_response.bearer_tft_ie) {
		 *                     }
		 *                            */
		/* TODO: Implement PGWC S5S8 bearer QoS */
		if (cs_rsp->bearer_contexts_created.bearer_lvl_qos.header.len) {
			bearer->qos.qci = cs_rsp->bearer_contexts_created.bearer_lvl_qos.qci;
			bearer->qos.ul_mbr =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_uplnk;
			bearer->qos.dl_mbr =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_dnlnk;
			bearer->qos.ul_gbr =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_uplnk;
			bearer->qos.dl_gbr =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk;
			bearer->qos.arp.preemption_vulnerability =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.pvi;
			bearer->qos.arp.spare1 =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.spare2;
			bearer->qos.arp.priority_level =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.pl;
			bearer->qos.arp.preemption_capability =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.pci;
			bearer->qos.arp.spare2 =
				cs_rsp->bearer_contexts_created.bearer_lvl_qos.spare3;
		}

		bearer->s5s8_pgw_gtpu_ipv4.s_addr =
			cs_rsp->bearer_contexts_created.s5s8_u_pgw_fteid.ipv4_address;
		bearer->s5s8_pgw_gtpu_teid =
			cs_rsp->bearer_contexts_created.s5s8_u_pgw_fteid.teid_gre_key;
		bearer->pdn = pdn;
	}

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];
	pfcp_sess_mod_req.update_far_count = 0;
	pfcp_sess_mod_req.update_far_count++;
	for(int itr=0 ; itr < pfcp_sess_mod_req.update_far_count; itr++){
		update_far[itr].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s5s8_pgw_gtpu_teid;
		update_far[itr].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s5s8_pgw_gtpu_ipv4.s_addr;
		update_far[itr].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(cs_rsp->bearer_contexts_created.s5s8_u_pgw_fteid.interface_type);
	}

	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, NULL,
			bearer, pdn, update_far, 0);

#ifdef USE_CSID
	fqcsid_t *tmp = NULL;
	/* PGW FQ-CSID */
	if (cs_rsp->pgw_fqcsid.header.len) {
		/* Stored the PGW CSID by PGW Node address */
		//tmp = get_peer_addr_csids_entry(ntohl(cs_rsp->pgw_fqcsid.node_address),
		tmp = get_peer_addr_csids_entry(cs_rsp->pgw_fqcsid.node_address,
				ADD);

		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = ntohl(cs_rsp->pgw_fqcsid.node_address);

		for(uint8_t itr = 0; itr < cs_rsp->pgw_fqcsid.number_of_csids; itr++) {
			uint8_t match = 0;
			for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
				if (tmp->local_csid[itr1] == cs_rsp->pgw_fqcsid.pdn_csid[itr])
					match = 1;
			}

			if (!match) {
				tmp->local_csid[tmp->num_csid++] =
					cs_rsp->pgw_fqcsid.pdn_csid[itr];
			}
		}
		memcpy(context->pgw_fqcsid, tmp, sizeof(fqcsid_t));
	} else {
		tmp = get_peer_addr_csids_entry(pdn->s5s8_pgw_gtpc_ipv4.s_addr,
				ADD);
		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = pdn->s5s8_pgw_gtpc_ipv4.s_addr;
		memcpy(context->pgw_fqcsid, tmp, sizeof(fqcsid_t));
	}

	/* Link local CSID with PGW CSID */
	if ((context->pgw_fqcsid)->num_csid) {
		csid_t *tmp1 = NULL;
		tmp1 = get_peer_csid_entry(
				&(context->pgw_fqcsid)->local_csid[(context->pgw_fqcsid)->num_csid - 1],
				S5S8_SGWC_PORT_ID);
		if (tmp1 == NULL) {
			clLog(apilogger, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}

		/* Link local csid with MME CSID */
		if (tmp1->local_csid == 0) {
			tmp1->local_csid = (context->sgw_fqcsid)->local_csid[(context->sgw_fqcsid)->num_csid - 1];
		} else if (tmp1->local_csid != local_csid){
			/* TODO: handle condition like single MME CSID link with multiple local CSID  */
		}

		/* Update the Node Addr */
		tmp1->node_addr = (context->sgw_fqcsid)->node_addr;
	}

	/* Set PGW FQ-CSID */
	if ((context->pgw_fqcsid)->num_csid) {
		set_fq_csid_t(&pfcp_sess_mod_req.pgw_c_fqcsid, context->pgw_fqcsid);
	}

#endif /* USE_CSID */

	if(pfcp_sess_mod_req.create_pdr_count){
		for(int itr = 0; itr < pfcp_sess_mod_req.create_pdr_count; itr++) {
			pfcp_sess_mod_req.create_pdr[itr].pdi.ue_ip_address.ipv4_address =
				(pdn->ipv4.s_addr);
			pfcp_sess_mod_req.create_pdr[itr].pdi.src_intfc.interface_value =
				SOURCE_INTERFACE_VALUE_ACCESS;
		}
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
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_sgwc_s5s8_create_sess_rsp_pfcp_timeout);
        pdn->trans_entry = trans_entry; 
	}

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	/* Lookup Stored the session information. */
	if (get_sess_entry(context->pdns[ebi_index]->seid, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s %s %d Failed to add response in entry in SM_HASH\n", __file__
				,__func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Set create session response */
	//resp->sequence = cs_rsp->header.teid.has_teid.seq;
	resp->eps_bearer_id = cs_rsp->bearer_contexts_created.eps_bearer_id.ebi_ebi;
	//resp->s11_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
	//resp->context = context;
	resp->msg_type = GTP_CREATE_SESSION_RSP;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

void
fill_pgwc_create_session_response(create_sess_rsp_t *cs_resp,
		uint32_t sequence, ue_context_t *context, uint8_t ebi_index)
{

	set_gtpv2c_header(&cs_resp->header, 1, GTP_CREATE_SESSION_RSP,
			context->pdns[ebi_index]->s5s8_sgw_gtpc_teid, sequence);

	set_cause_accepted(&cs_resp->cause, IE_INSTANCE_ZERO);

	set_ipv4_fteid(
			&cs_resp->pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc,
			GTPV2C_IFTYPE_S5S8_PGW_GTPC, IE_INSTANCE_ONE,
			context->pdns[ebi_index]->s5s8_pgw_gtpc_ipv4,
			context->pdns[ebi_index]->s5s8_pgw_gtpc_teid);

	/* TODO: Added Temp Fix for the UE IP*/
	struct in_addr ipv4 = {0};
	context->pdns[ebi_index]->ipv4.s_addr = htonl(context->pdns[ebi_index]->ipv4.s_addr);
	ipv4.s_addr = context->pdns[ebi_index]->ipv4.s_addr;
	set_ipv4_paa(&cs_resp->paa, IE_INSTANCE_ZERO, ipv4);
			//context->pdns[ebi_index]->ipv4);

	set_apn_restriction(&cs_resp->apn_restriction, IE_INSTANCE_ZERO,
			context->pdns[ebi_index]->apn_restriction);

	set_ebi(&cs_resp->bearer_contexts_created.eps_bearer_id,
			IE_INSTANCE_ZERO,
			context->eps_bearers[ebi_index]->eps_bearer_id);
	set_cause_accepted(&cs_resp->bearer_contexts_created.cause,
			IE_INSTANCE_ZERO);
	set_ie_header(&cs_resp->bearer_contexts_created.bearer_lvl_qos.header,
			GTP_IE_BEARER_QLTY_OF_SVC, IE_INSTANCE_ZERO,
			sizeof(gtp_bearer_qlty_of_svc_ie_t) - sizeof(ie_header_t));
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pvi =
		context->eps_bearers[ebi_index]->qos.arp.preemption_vulnerability;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.spare2 = 0;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pl =
		context->eps_bearers[ebi_index]->qos.arp.priority_level;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pci =
		context->eps_bearers[ebi_index]->qos.arp.preemption_capability;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.spare3 = 0;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.qci =
		context->eps_bearers[ebi_index]->qos.qci;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_uplnk =
		context->eps_bearers[ebi_index]->qos.ul_mbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_dnlnk =
		context->eps_bearers[ebi_index]->qos.dl_mbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_uplnk =
		context->eps_bearers[ebi_index]->qos.ul_gbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk =
		context->eps_bearers[ebi_index]->qos.dl_gbr;

	context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4.s_addr =
		        htonl(context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4.s_addr);
	set_ipv4_fteid(&cs_resp->bearer_contexts_created.s5s8_u_pgw_fteid,
			GTPV2C_IFTYPE_S5S8_PGW_GTPU, IE_INSTANCE_TWO,
			context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4,
			context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_teid);

	set_ie_header(&cs_resp->bearer_contexts_created.header,
			GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO,
			(cs_resp->bearer_contexts_created.eps_bearer_id.header.len
			 + sizeof(ie_header_t)
			 + cs_resp->bearer_contexts_created.cause.header.len
			 + sizeof(ie_header_t)
			 + cs_resp->bearer_contexts_created.s5s8_u_pgw_fteid.header.len
			 + sizeof(ie_header_t))
			 + cs_resp->bearer_contexts_created.bearer_lvl_qos.header.len
			 + sizeof(ie_header_t));
}

int
process_cs_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_sgwc_s5s8_create_sess_rsp(&msg->gtpc_msg.cs_rsp);
	if (ret) {
			if(ret != -1){
				cs_error_response(msg, ret, S11_IFACE);
				process_error_occured_handler_new(data, unused_param);
			}
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

#endif
