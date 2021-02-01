// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEEDS
// saegw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT => process_del_pdn_conn_set_rsp  
// saegw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT => process_del_pdn_conn_set_req  
// pgw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT ==> process_del_pdn_conn_set_req
// sgw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT process_del_pdn_conn_set_req

int handle_delete_pdn_conn_set_req(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    RTE_SET_USED(gtpv2c_rx);
	msg->state = DEL_PDN_CONN_SET_REQ_RCVD_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->event = DEL_PDN_CONN_SET_REQ_RCVD_EVNT;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			" Msg_Type:%s[%u],"
			"State:%s, Event:%s",
			gtp_type_str(msg->msg_type), msg->msg_type,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

int handle_delete_pdn_conn_set_req(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx) 
{
    int ret;

    if ((ret = decode_del_pdn_conn_set_req((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.del_pdn_req) == 0))
        return -1;
}
#endif
