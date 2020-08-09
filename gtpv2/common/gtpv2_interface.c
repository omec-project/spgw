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
#include "cp_stats.h"
#include "gtp_ies.h"
#include "rte_common.h"
#include "util.h"

extern struct cp_stats_t cp_stats;
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
handle_unknown_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_s11_rx)
{
    RTE_SET_USED(msg);
	clLog(clSystemLog, eCLSeverityCritical, "Unhandled GTP message = %d\n", gtpv2c_s11_rx->gtpc.message_type);
    return -1;
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


void
stats_update(uint8_t msg_type)
{
    switch (cp_config->cp_type) {
        case SGWC:
        case SAEGWC:
            switch (msg_type) {
                case GTP_CREATE_SESSION_REQ:
                    cp_stats.create_session++;
                    break;
                case GTP_DELETE_SESSION_REQ:
                    cp_stats.delete_session++;
                    break;
                case GTP_MODIFY_BEARER_REQ:
                    cp_stats.modify_bearer++;
                    break;
                case GTP_RELEASE_ACCESS_BEARERS_REQ:
                    cp_stats.rel_access_bearer++;
                    break;
                case GTP_BEARER_RESOURCE_CMD:
                    cp_stats.bearer_resource++;
                    break;

                case GTP_DELETE_BEARER_RSP:
                    cp_stats.delete_bearer++;
                    return;
                case GTP_DOWNLINK_DATA_NOTIFICATION_ACK:
                    cp_stats.ddn_ack++;
                    break;
                case GTP_ECHO_REQ:
                    cp_stats.echo++;
                    break;
            }
            break;

        case PGWC:
            switch (msg_type) {
                case GTP_CREATE_SESSION_REQ:
                    cp_stats.create_session++;
                    break;

                case GTP_DELETE_SESSION_REQ:
                    cp_stats.delete_session++;
                    break;
            }
            break;
        default:
            rte_panic("main.c::control_plane::cp_stats-"
                    "Unknown spgw_cfg= %d.", cp_config->cp_type);
            break;
    }
}

