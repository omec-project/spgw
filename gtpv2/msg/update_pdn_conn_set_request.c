// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "pfcp_messages_decoder.h"
#ifdef FUTURE_NEED
int handle_update_pdn_conn_set_req(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    RTE_SET_USED(gtpv2c_rx);
	//TODO: TEID based lookup
	//msg->state = ;
	msg->proc = RESTORATION_RECOVERY_PROC; 
	//msg->event = UPD_PDN_CONN_SET_REQ_RCVD_EVNT;

	//LOG_MSG(LOG_DEBUG, "Callback called for "
	//		" Msg_Type:%s[%u],"
	//		"State:%s, Event:%s\n",
	//		gtp_type_str(msg->msg_type), msg->msg_type,
	//		get_state_string(msg->state), get_event_string(msg->event));
    return 0;
}

int 
handle_update_pdn_conn_set_req(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    //if ((ret = decode_upd_pdn_conn_set_req((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.upd_pdn_req) == 0))
    //    return -1;
}
#endif

