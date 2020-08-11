// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#ifdef FUTURE_NEED
// saegw - PDN_GW_INIT_BEARER_DEACTIVATION  DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_resp_handler  
// saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_response_handler 
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_resp_handler
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_response_handler
// sgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT : process_delete_bearer_resp_handler 
// sgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT - process_delete_bearer_response_handler 
int handle_delete_bearer_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    uint8_t ebi_index;
    ue_context_t *context = NULL;
    int ret = 0;

    if((ret = decode_del_bearer_rsp((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.db_rsp) == 0))
        return -1;

    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);


    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	if (msg->gtpc_msg.db_rsp.lbi.header.len) {
		ebi_index = msg->gtpc_msg.db_rsp.lbi.ebi_ebi - 5;
	} else {
		ebi_index = msg->gtpc_msg.db_rsp.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;
	}

	if(get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		return -1;
	}
	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid, ebi_index)) > 0){
		msg->state = ret;
	}else{
		return -1;
	}

	if(context->eps_bearers[ebi_index]->pdn->proc == MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC){
		msg->proc = context->eps_bearers[ebi_index]->pdn->proc;
	}else{
		msg->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		context->eps_bearers[ebi_index]->pdn->proc = msg->proc;
	}
	msg->event = DELETE_BER_RESP_RCVD_EVNT;
	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));


    process_delete_bearer_response_handler(msg, NULL);
    return 0;
}

int
process_delete_bearer_resp_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;

	if (msg->gtpc_msg.db_rsp.lbi.header.len != 0) {
		/* Delete Default Bearer. Send PFCP Session Deletion Request */
		process_pfcp_sess_del_request_delete_bearer_rsp(&msg->gtpc_msg.db_rsp);
	} else {
		/* Delete Dedicated Bearer. Send PFCP Session Modification Request */
		process_delete_bearer_resp(&msg->gtpc_msg.db_rsp , 0);
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);

	return 0;
}

void 
process_delete_bearer_resp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_delete_bearer_resp(del_bearer_rsp_t *db_rsp, uint8_t is_del_bearer_cmd)
{
	int ret;
	uint8_t ebi_index = 5;
	uint8_t bearer_cntr = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint8_t default_bearer_id = 0;
	eps_bearer_t *bearers[MAX_BEARERS];
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    RTE_SET_USED(is_del_bearer_cmd);

	ret = get_ue_context(db_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	s11_mme_sockaddr.sin_addr.s_addr =
		context->s11_mme_gtpc_ipv4.s_addr;

	if (db_rsp->lbi.header.len) {
		default_bearer_id = db_rsp->lbi.ebi_ebi;
		pdn = context->pdns[default_bearer_id];
		bearers[default_bearer_id - 5] = context->eps_bearers[default_bearer_id - 5];
		bearer_cntr = 1;
	} else {
		for (uint8_t iCnt = 0; iCnt < db_rsp->bearer_count; ++iCnt) {
			ebi_index = db_rsp->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
			bearers[iCnt] = context->eps_bearers[ebi_index - 5];
		}
		pdn = context->eps_bearers[ebi_index - 5]->pdn;
		bearer_cntr = db_rsp->bearer_count;

	}

	fill_pfcp_sess_mod_req_pgw_init_remove_pdr(&pfcp_sess_mod_req, pdn, bearers, bearer_cntr);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0) {
		clLog(sxlogger, eCLSeverityCritical,
			"%s : Error in sending MBR to SGW-U. err_no: %i\n",
			__func__, errno);
	} else {
		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_mod_req.header.message_type, SENT, SX);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_delete_bearer_resp_pfcp_timeout);
        pdn->trans_entry = trans_entry;
	}

#ifdef FUTURE_NEED
	context->sequence = db_rsp->header.teid.has_teid.seq;
#endif
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

#if 0
	if (get_sess_entry_seid(pdn->seid, &resp) != 0) {
		clLog(sxlogger, eCLSeverityCritical,
			"%s : Failed to add response in entry in SM_HASH\n",
			__func__);
		return -1;
	}

	if (db_rsp->lbi.header.len != 0) {
		resp->linked_eps_bearer_id = db_rsp->lbi.ebi_ebi;
		resp->bearer_count = 0;
	} else {
		resp->bearer_count = db_rsp->bearer_count;
		for (uint8_t iCnt = 0; iCnt < db_rsp->bearer_count; ++iCnt) {
			resp->eps_bearer_ids[iCnt] = db_rsp->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
		}
	}
	if(is_del_bearer_cmd == 0){
		resp->msg_type = GTP_DELETE_BEARER_RSP;
		resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
		resp->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		pdn->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
	}else{

		resp->msg_type = GTP_DELETE_BEARER_RSP;
		resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
		resp->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
		pdn->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
	}
#endif

	return 0;
}
#endif
