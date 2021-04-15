// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEEDS
// pgw - RESTORATION_RECOVERY_PROC PGW_RSTRT_NOTIF_REQ_SNT_STATE PGW_RSTRT_NOTIF_ACK_RCVD_EVNT ==> process_pgw_rstrt_notif_ack 
// sgw - RESTORATION_RECOVERY_PROC PGW_RSTRT_NOTIF_REQ_SNT_STATE PGW_RSTRT_NOTIF_ACK_RCVD_EVNT ==> process_pgw_rstrt_notif_ack  
int handle_pgw_restart_notf_ack(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
	msg->state = PGW_RSTRT_NOTIF_REQ_SNT_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->event = PGW_RSTRT_NOTIF_ACK_RCVD_EVNT;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			" Msg_Type:%s[%u],"
			"State:%s, Event:%s\n",
			gtp_type_str(msg->msg_type), msg->msg_type,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

/* Function */
int
process_pgw_rstrt_notif_ack(void *data, void *unused_param)
{
#ifdef USE_CSID
	msg_info_t *msg = (msg_info_t *)data;

	if (msg->rx_msg.pgw_rstrt_notif_ack.cause.cause_value ==
			GTPV2C_CAUSE_REQUEST_ACCEPTED) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}
#else
#endif /* USE_CSID */
	return 0;
}

int
handle_pgw_restart_notf_ack(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    if ((ret = decode_pgw_rstrt_notif_ack((uint8_t *) gtpv2c_rx, &msg->rx_msg.pgw_rstrt_notif_ack) == 0))
        return -1;

}
#endif

