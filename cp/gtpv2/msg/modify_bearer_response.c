// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED_SGW
// sgw : SGW_RELOCATION_PROC DDN_ACK_RCVD_STATE MB_RESP_RCVD_EVNT => process_mbr_resp_handover_handler  
int handle_modify_bearer_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

    if((ret = decode_mod_bearer_rsp((uint8_t *) gtpv2c_rx,
                    &msg->rx_msg.mb_rsp) == 0)) {
        return -1;

	gtpc_delete_timer_entry(msg->rx_msg.mb_rsp.header.teid.has_teid.teid);

	if(msg->rx_msg.mb_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		cs_error_response(msg, msg->rx_msg.mb_rsp.cause.cause_value,
				cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	if(get_ue_context_by_sgw_s5s8_teid(msg->rx_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0)
	{
		cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		process_error_occured_handler(&msg, NULL);

		return -1;
	}
	uint8_t ebi_index = msg->rx_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi - 5;
	pdn = GET_PDN(context, ebi_index);
	msg->state = pdn->state;
	msg->proc = pdn->proc;
	msg->event = MB_RESP_RCVD_EVNT;
    return 0;
}
#endif


