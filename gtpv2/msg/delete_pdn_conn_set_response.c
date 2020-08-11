// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED
// pgw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT : process_del_pdn_conn_set_rsp 
// sgw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT process_del_pdn_conn_set_rsp 
int handle_delete_pdn_conn_set_rsp(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    RTE_SET_USED(gtpv2c_rx);

    if ((ret = decode_del_pdn_conn_set_rsp((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.del_pdn_rsp) == 0))
        return -1;

	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->state = DEL_PDN_CONN_SET_REQ_SNT_STATE;
	msg->event = DEL_PDN_CONN_SET_RESP_RCVD_EVNT;
    return 0;
}
#endif

