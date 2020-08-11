// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <getopt.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_cfgfile.h>
#include "gtpv2_interface.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "cp_io_poll.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_decoder.h"
#include "pfcp_messages_encoder.h"
#include "cp_peer.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "sm_struct.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_internal.h"
#include "cp_timer.h"
#include "cp_init.h"
#include "util.h"

#ifdef USE_DNS_QUERY
#include "cdnshelper.h"
#endif /* USE_DNS_QUERY */

extern udp_sock_t my_sock;
uint8_t s11_rx_buf[MAX_GTPV2C_UDP_LEN];

uint32_t start_time;

/* S5S8 */
extern socklen_t s5s8_sockaddr_len;
extern uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];

struct cp_params cp_params;
extern struct cp_stats_t cp_stats;


uint16_t payload_length;
extern uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];

/* Requirement1 - multiple read to read all the messages from the sockets 
 */
int
msg_handler_s11(void)
{
    int ret = 0, bytes_rx = 0;
    msg_info_t msg = {0};
    struct sockaddr_in s11_peer_sockaddr = {0};
    socklen_t s11_peer_sockaddr_len = sizeof(s11_peer_sockaddr);

    bzero(&s11_rx_buf, sizeof(s11_rx_buf));
    bzero(&s11_tx_buf, sizeof(s11_tx_buf));
    gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *) s11_rx_buf;
    RTE_SET_USED(ret);

    bytes_rx = recvfrom(my_sock.sock_fd_s11,
            s11_rx_buf, MAX_GTPV2C_UDP_LEN, MSG_DONTWAIT,
            (struct sockaddr *) &s11_peer_sockaddr,
            &s11_peer_sockaddr_len);
    if (bytes_rx == 0) {
        clLog(clSystemLog, eCLSeverityCritical, "SGWC|SAEGWC_s11 recvfrom error:"
                "\n\ton %s:%u - %s\n",
                inet_ntoa(s11_peer_sockaddr.sin_addr),
                s11_peer_sockaddr.sin_port,
                strerror(errno));
        return -1;
    }

    if ((bytes_rx < 0) &&
            (errno == EAGAIN  || errno == EWOULDBLOCK))
        return -1;

    if (!gtpv2c_rx->gtpc.message_type) {
        return -1;
    }

    msg.peer_addr = s11_peer_sockaddr;

    if (bytes_rx > 0)
        ++cp_stats.rx;
  
    update_cli_stats((uint32_t)s11_peer_sockaddr.sin_addr.s_addr,
            gtpv2c_rx->gtpc.message_type,RCVD,S11);

    msg.source_interface = S11_IFACE;
	msg.msg_type = gtpv2c_rx->gtpc.message_type;

	if ((unsigned)bytes_rx != (ntohs(gtpv2c_rx->gtpc.message_len) + sizeof(gtpv2c_rx->gtpc))) {
		ret = GTPV2C_CAUSE_INVALID_LENGTH;
		/* According to 29.274 7.7.7, if message is request,
		 * reply with cause = GTPV2C_CAUSE_INVALID_LENGTH
		 *  should be sent - ignoring packet for now
		 */
		clLog(clSystemLog, eCLSeverityCritical, "GTPv2C Received UDP Payload:"
				"\n\t(%d bytes) with gtpv2c + "
				"header (%u + %lu) = %lu bytes\n",
				bytes_rx, ntohs(gtpv2c_rx->gtpc.message_len),
				sizeof(gtpv2c_rx->gtpc),
				ntohs(gtpv2c_rx->gtpc.message_len)
				+ sizeof(gtpv2c_rx->gtpc));
		return ret;
	}

	if(bytes_rx > 0) {
		if (gtpv2c_rx->gtpc.version < GTP_VERSION_GTPV2C) {
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}else if (gtpv2c_rx->gtpc.version > GTP_VERSION_GTPV2C) {
			send_version_not_supported(&msg.peer_addr, 
                                       cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
					                   gtpv2c_rx->teid.has_teid.seq);
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}
	}
    gtp_msg_handler[gtpv2c_rx->gtpc.message_type](&msg, gtpv2c_rx);

#if 0
    if (bytes_s11_rx > 0) {
        if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
            switch (gtpv2c_s11_rx->gtpc.type) {
                case GTP_BEARER_RESOURCE_CMD:
                    ret = process_bearer_resource_command(
                            gtpv2c_s11_rx, gtpv2c_s11_tx);

                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, "main.c::control_plane()::Error"
                                "\n\tcase SAEGWC:"
                                "\n\tprocess_bearer_resource_command "
                                "%s: (%d) %s\n",
                                gtp_type_str(gtpv2c_s11_rx->gtpc.type), ret,
                                (ret < 0 ? strerror(-ret) : cause_str(ret)));
                        /* Error handling not implemented */
                        return;
                    }
                    payload_length = ntohs(gtpv2c_s11_tx->gtpc.length)
                        + sizeof(gtpv2c_s11_tx->gtpc);
                    gtpv2c_send(my_sock.sock_fd_s11, s11_tx_buf, payload_length,
                            (struct sockaddr *) &s11_mme_sockaddr, s11_mme_sockaddr_len);
                    break;

                case GTP_CREATE_BEARER_RSP:
                    ret = process_create_bearer_response(gtpv2c_s11_rx);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, "main.c::control_plane()::Error"
                                "\n\tcase SAEGWC:"
                                "\n\tprocess_create_bearer_response "
                                "%s: (%d) %s\n",
                                gtp_type_str(gtpv2c_s11_rx->gtpc.type), ret,
                                (ret < 0 ? strerror(-ret) : cause_str(ret)));
                        /* Error handling not implemented */
                        return;
                    }
                    payload_length = ntohs(gtpv2c_s11_tx->gtpc.length)
                        + sizeof(gtpv2c_s11_tx->gtpc);
                    gtpv2c_send(my_sock.sock_fd_s11, s11_tx_buf, payload_length,
                            (struct sockaddr *) &s11_mme_sockaddr,
                            s11_mme_sockaddr_len);
                    break;

                case GTP_DELETE_BEARER_RSP:
                    ret = process_delete_bearer_response(gtpv2c_s11_rx);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, "main.c::control_plane()::Error"
                                "\n\tcase SAEGWC:"
                                "\n\tprocess_delete_bearer_response "
                                "%s: (%d) %s\n",
                                gtp_type_str(gtpv2c_s11_rx->gtpc.type), ret,
                                (ret < 0 ? strerror(-ret) : cause_str(ret)));
                        /* Error handling not implemented */
                        return;
                    }
                    payload_length = ntohs(gtpv2c_s11_tx->gtpc.length)
                        + sizeof(gtpv2c_s11_tx->gtpc);
                    gtpv2c_send(my_sock.sock_fd_s11, s11_tx_buf, payload_length,
                            (struct sockaddr *) &s11_mme_sockaddr,
                            s11_mme_sockaddr_len);
                    break;

                default:
                    //clLog(clSystemLog, eCLSeverityCritical, "main.c::control_plane::process_msgs-"
                    //		"\n\tcase: SAEGWC::spgw_cfg= %d;"
                    //		"\n\tReceived unprocessed s11 GTPv2c Message Type: "
                    //		"%s (%u 0x%x)... Discarding\n",
                    //		cp_config->cp_type, gtp_type_str(gtpv2c_s11_rx->gtpc.type),
                    //		gtpv2c_s11_rx->gtpc.type,
                    //		gtpv2c_s11_rx->gtpc.type);
                    //return;
                    break;
            }
        }
    }
#endif

    switch (cp_config->cp_type) {
        case SGWC:
        case SAEGWC:
            if (bytes_rx > 0) {
                ++cp_stats.tx;
                switch (gtpv2c_rx->gtpc.message_type) {
                    case GTP_CREATE_SESSION_REQ:
                        cp_stats.create_session++;
                        break;
                    case GTP_DELETE_SESSION_REQ:
                        cp_stats.delete_session++;
                        break;
                    case GTP_MODIFY_BEARER_REQ:
                        cp_stats.modify_bearer++;
                        //clLog(clSystemLog, eCLSeverityDebug,"VS:MBR[%u]:cnt: %u\n", gtpv2c_s11_rx->gtpc.type, ++cnt);
                        break;
                    case GTP_RELEASE_ACCESS_BEARERS_REQ:
                        cp_stats.rel_access_bearer++;

                        break;
                    case GTP_BEARER_RESOURCE_CMD:
                        cp_stats.bearer_resource++;
                        break;
                    case GTP_CREATE_BEARER_RSP:
                        cp_stats.create_bearer++;
                        break;
                    case GTP_DELETE_BEARER_RSP:
                        cp_stats.delete_bearer++;
                        break;
                    case GTP_DOWNLINK_DATA_NOTIFICATION_ACK:
                        cp_stats.ddn_ack++;
                        break;
                }
            }
            break;
        default:
            rte_panic("main.c::control_plane::cp_stats-"
                    "Unknown spgw_cfg= %u.", cp_config->cp_type);
            break;
    }
    return 0;
}

#ifdef FUTURE_NEED
int
msg_handler_s5s8(void)
{
	int ret = 0;
	int bytes_s5s8_rx = 0;
	msg_info_t msg = {0};


	bzero(&s5s8_rx_buf, sizeof(s5s8_rx_buf));
	gtpv2c_header_t *gtpv2c_s5s8_rx = (gtpv2c_header_t *) s5s8_rx_buf;

	bzero(&s5s8_tx_buf, sizeof(s5s8_tx_buf));
	gtpv2c_header_t *gtpv2c_s5s8_tx = (gtpv2c_header_t *) s5s8_tx_buf;

	bytes_s5s8_rx = recvfrom(my_sock.sock_fd_s5s8, s5s8_rx_buf,
			MAX_GTPV2C_UDP_LEN, MSG_DONTWAIT,
			(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
			&s5s8_sockaddr_len);

	if (bytes_s5s8_rx == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "s5s8 recvfrom error:"
				"\n\ton %s:%u - %s\n",
				inet_ntoa(my_sock.s5s8_recv_sockaddr.sin_addr),
				my_sock.s5s8_recv_sockaddr.sin_port,
				strerror(errno));
	}

	if (
		(bytes_s5s8_rx < 0) &&
		(errno == EAGAIN  || errno == EWOULDBLOCK)
		)
		return -1;

	if (!gtpv2c_s5s8_rx->gtpc.message_type) {
		return -1;
	}

#if 0
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == PGWC)) {
		if ((bytes_s5s8_rx > 0) &&
			 (unsigned)bytes_s5s8_rx != (
			 ntohs(gtpv2c_s5s8_rx->gtpc.message_len)
			 + sizeof(gtpv2c_s5s8_rx->gtpc))
			) {
			ret = GTPV2C_CAUSE_INVALID_LENGTH;
			/* According to 29.274 7.7.7, if message is request,
			 * reply with cause = GTPV2C_CAUSE_INVALID_LENGTH
			 *  should be sent - ignoring packet for now
			 */
			clLog(clSystemLog, eCLSeverityCritical, "SGWC|PGWC_s5s8 Received UDP Payload:"
					"\n\t(%d bytes) with gtpv2c + "
					"header (%u + %lu) = %lu bytes\n",
					bytes_s5s8_rx, ntohs(gtpv2c_s5s8_rx->gtpc.message_len),
					sizeof(gtpv2c_s5s8_rx->gtpc),
					ntohs(gtpv2c_s5s8_rx->gtpc.message_len)
					+ sizeof(gtpv2c_s5s8_rx->gtpc));
		}
	}
#endif
	if (bytes_s5s8_rx > 0)
		++cp_stats.rx;


	if (cp_config->cp_type == PGWC)
		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
					gtpv2c_s5s8_rx->gtpc.message_type,RCVD,S5S8);

	if (cp_config->cp_type == SGWC && (gtpv2c_s5s8_rx->gtpc.message_type == GTP_ECHO_REQ ||
		gtpv2c_s5s8_rx->gtpc.message_type == GTP_ECHO_RSP ||
		gtpv2c_s5s8_rx->gtpc.message_type == GTP_CREATE_BEARER_REQ))

		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
					gtpv2c_s5s8_rx->gtpc.message_type,RCVD,S5S8);
#if 0
	if (((cp_config->cp_type == PGWC) && (bytes_s5s8_rx > 0)) &&
		  (gtpv2c_s5s8_rx->gtpc.version != GTP_VERSION_GTPV2C)
		) {
		clLog(clSystemLog, eCLSeverityCritical, "PFCP Discarding packet from %s:%u - "
				"Expected S5S8_IP = %s\n",
				inet_ntoa(s5s8_recv_sockaddr.sin_addr),
				ntohs(s5s8_recv_sockaddr.sin_port),
				inet_ntoa(cp_config->s5s8_ip));
		return;
		}
#endif
	if(gtpv2c_s5s8_rx->gtpc.message_type == GTP_ECHO_REQ){
		if (bytes_s5s8_rx > 0) {
			ret = process_echo_req(gtpv2c_s5s8_rx, gtpv2c_s5s8_tx, S5S8_IFACE);
			if(ret != 0){
				return 0;
			}
			++cp_stats.tx;
		}
		return 0;
	}else if(gtpv2c_s5s8_rx->gtpc.message_type == GTP_ECHO_RSP){
		if (bytes_s5s8_rx > 0) {
			ret = process_echo_resp(gtpv2c_s5s8_rx, S5S8_IFACE);
			if(ret != 0){
				return 0;
			}
			++cp_stats.tx;
		}
		return 0;
	}else {

        /* Reset periodic timers */
        process_response(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);

        if ((ret = gtpc_pcnd_check(gtpv2c_s5s8_rx, &msg, bytes_s5s8_rx)) != 0)
		{
			/*CLI: update csr, dsr, mbr rej response*/
			if(cp_config->cp_type == SGWC)
				update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
							gtpv2c_s5s8_rx->gtpc.message_type,REJ,S5S8);
			return 0;
		}

        // validate message content - validate the presence of IEs
        ret = validate_gtpv2_message_content(&msg);
        if(ret !=  0) 
        {
            printf("Message validation failed \n");
            // validatation failed;
            return 0;
        }

	if (cp_config->cp_type == SGWC)
		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
					gtpv2c_s5s8_rx->gtpc.message_type,ACC,S5S8);

	if (cp_config->cp_type == SGWC)
	{
		if (gtpv2c_s5s8_rx->gtpc.message_type == GTP_CREATE_SESSION_RSP )
		{
			add_node_conn_entry(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr, S5S8_SGWC_PORT_ID);
		}
		if (gtpv2c_s5s8_rx->gtpc.message_type == GTP_MODIFY_BEARER_RSP)
		{
			add_node_conn_entry(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr, S5S8_SGWC_PORT_ID);
		}
	}
        /* Event set depending on msg type 
         * Proc  found from user context and message content, message type 
         * State is on the pdn connection (for now)..Need some more attention here later   
         */
        msg.source_interface = S5S8_IFACE;
        process_gtp_message(gtpv2c_s5s8_rx, &msg);

	}

	if (bytes_s5s8_rx > 0)
		++cp_stats.tx;

	switch (cp_config->cp_type) {
	case SGWC:
			break; //do not update console stats for SGWC
	case PGWC:
		if (bytes_s5s8_rx > 0) {
			switch (gtpv2c_s5s8_rx->gtpc.message_type) {
			case GTP_CREATE_SESSION_REQ:
				cp_stats.create_session++;
				break;
			case GTP_MODIFY_BEARER_REQ:
				cp_stats.modify_bearer++;
				break;
			case GTP_DELETE_SESSION_REQ:
				cp_stats.delete_session++;
				break;
			}
		}
		break;
	default:
		rte_panic("main.c::control_plane::cp_stats-"
				"Unknown spgw_cfg= %u.", cp_config->cp_type);
		break;
	}
    return 0;
}
#endif