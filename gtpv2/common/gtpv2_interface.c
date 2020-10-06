// Copyright (c) 2017 Intel Corporation
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtpv2_interface.h"
#include "clogger.h"
#include "gtp_messages_decoder.h"
#include "cp_interface.h"
#include "cp_config.h"
#include "gtp_ies.h"
#include "rte_common.h"
#include "util.h"
#include "cp_events.h"
#include "gtpv2_error_rsp.h"

gtp_handler gtp_msg_handler[256];

void init_gtp_interface(void)
{
    for(int i=0;i<256;i++)
        gtp_msg_handler[i] = handle_unknown_msg;

    gtp_msg_handler[GTP_ECHO_REQ] =  handle_echo_request;
    gtp_msg_handler[GTP_ECHO_RSP] =  handle_echo_response;
    gtp_msg_handler[GTP_CREATE_SESSION_REQ] =  handle_create_session_request;
    gtp_msg_handler[GTP_MODIFY_BEARER_REQ] = handle_modify_bearer_request; 
    gtp_msg_handler[GTP_DELETE_SESSION_REQ] = handle_delete_session_request;
    gtp_msg_handler[GTP_RELEASE_ACCESS_BEARERS_REQ] = handle_rab_request;
    gtp_msg_handler[GTP_DOWNLINK_DATA_NOTIFICATION_ACK] = handle_ddn_ack; 
#ifdef FUTURE_NEED
    gtp_msg_handler[GTP_CREATE_SESSION_RSP]= handle_create_session_response_msg;
    gtp_msg_handler[GTP_MODIFY_BEARER_RSP] = handle_modify_bearer_response_msg;
    gtp_msg_handler[GTP_DELETE_SESSION_RSP] = handle_delete_session_response_msg;
    gtp_msg_handler[GTP_DELETE_BEARER_REQ] =  handle_delete_bearer_request_msg;
    gtp_msg_handler[GTP_DELETE_BEARER_RSP] =  handle_delete_bearer_response_msg;
    gtp_msg_handler[GTP_UPDATE_BEARER_REQ] = handle_update_bearer_request_msg;
    gtp_msg_handler[GTP_UPDATE_BEARER_RSP] = handle_update_bearer_response_msg;
    gtp_msg_handler[GTP_CREATE_BEARER_REQ] =  handle_create_bearer_request_msg;
    gtp_msg_handler[GTP_CREATE_BEARER_RSP] = handle_create_bearer_response_msg;
	gtp_msg_handler[GTP_DELETE_BEARER_CMD] = handle_delete_bearer_cmd_msg;
	gtp_msg_handler[GTP_DELETE_PDN_CONNECTION_SET_REQ] = handle_delete_pdn_conn_set_req;
	gtp_msg_handler[GTP_DELETE_PDN_CONNECTION_SET_RSP] =  handle_delete_pdn_conn_set_rsp;
	gtp_msg_handler[GTP_UPDATE_PDN_CONNECTION_SET_REQ] =  handle_update_pdn_conn_set_req;
	gtp_msg_handler[GTP_UPDATE_PDN_CONNECTION_SET_RSP] = handle_update_pdn_conn_set_rsp;
	gtp_msg_handler[GTP_PGW_RESTART_NOTIFICATION_ACK]  =  handle_pgw_restart_notf_ack;
#endif
}

int 
handle_unknown_msg(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_s11_rx)
{
    msg_info_t *msg = *msg_p;
    RTE_SET_USED(msg);
	clLog(clSystemLog, eCLSeverityCritical, "Unhandled GTP message = %d\n", gtpv2c_s11_rx->gtpc.message_type);
    return -1;
}

void
process_gtp_msg(void *data, uint16_t event)
{
    assert(event == GTP_MSG_RECEIVED );
    msg_info_t *msg = (msg_info_t *)data;    
    gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *)msg->raw_buf;
    if (gtpv2c_rx->gtpc.version < GTP_VERSION_GTPV2C) {
        fprintf(stderr, "Discarding packet due to gtp version is not supported..");
        return;
    }else if (gtpv2c_rx->gtpc.version > GTP_VERSION_GTPV2C) {
        send_version_not_supported(&msg->peer_addr, 
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
                gtpv2c_rx->teid.has_teid.seq);
        fprintf(stderr, "Discarding packet due to gtp version is not supported..");
        return;
    }
    gtp_msg_handler[gtpv2c_rx->gtpc.message_type](&msg, gtpv2c_rx);
    free(msg->raw_buf);
    if(msg->refCnt == 0)
        free(msg);

}

/**
 * @brief  : Util to send or dump gtpv2c messages
 * @param  : gtpv2c_if_fd, interface indentifier
 * @param  : gtpv2c_tx_buf, buffer to store data for peer node
 * @param  : gtpv2c_pyld_len, data length
 * @param  : dest_addr, destination address
 * @param  : dest_addr_len, destination address length
 * @return : Void
 */
void
gtpv2c_send(int gtpv2c_if_fd, uint8_t *gtpv2c_tx_buf,
		uint16_t gtpv2c_pyld_len, struct sockaddr *dest_addr,
		socklen_t dest_addr_len)
{
	int bytes_tx;
	if (pcap_dumper) {
		dump_pcap(gtpv2c_pyld_len, gtpv2c_tx_buf);
	} else {
		bytes_tx = sendto(gtpv2c_if_fd, gtpv2c_tx_buf, gtpv2c_pyld_len, 0,
			(struct sockaddr *) dest_addr, dest_addr_len);

		clLog(clSystemLog, eCLSeverityDebug, "NGIC- main.c::gtpv2c_send()""\n\tgtpv2c_if_fd= %d\n", gtpv2c_if_fd);

	if (bytes_tx != (int) gtpv2c_pyld_len) {
			clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete GTPv2c Message:"
					"%u of %d tx bytes\n",
					gtpv2c_pyld_len, bytes_tx);
		}
	}
}
