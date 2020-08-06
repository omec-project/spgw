// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

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

#endif
