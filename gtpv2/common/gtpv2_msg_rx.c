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
#include "cp_io_poll.h"
#include "cdnshelper.h"

uint8_t s11_rx_buf[MAX_GTPV2C_UDP_LEN];

uint32_t start_time;

/* S5S8 */
extern uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];

/* Requirement1 - multiple read to read all the messages from the sockets 
 */
int
msg_handler_s11(void)
{
    int ret = 0, bytes_rx = 0;
    msg_info_t *msg = calloc(1, sizeof(msg_info_t));
    struct sockaddr_in peer_sockaddr = {0};
    socklen_t peer_sockaddr_len = sizeof(peer_sockaddr);

    bzero(&s11_rx_buf, sizeof(s11_rx_buf));
    gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *) s11_rx_buf;

    bytes_rx = recvfrom(my_sock.sock_fd_s11,
            s11_rx_buf, MAX_GTPV2C_UDP_LEN, MSG_DONTWAIT,
            (struct sockaddr *) &peer_sockaddr,
            &peer_sockaddr_len);
    if (bytes_rx == 0) {
        clLog(clSystemLog, eCLSeverityCritical, "SGWC|SAEGWC_s11 recvfrom error:"
                "\n\ton %s:%u - %s\n",
                inet_ntoa(peer_sockaddr.sin_addr),
                peer_sockaddr.sin_port,
                strerror(errno));
        return -1;
    }

    if ((bytes_rx < 0) && (errno == EAGAIN  || errno == EWOULDBLOCK))
        return -1;

    if (!gtpv2c_rx->gtpc.message_type) {
        return -1;
    }

    msg->peer_addr = peer_sockaddr;
    msg->source_interface = S11_IFACE;
	msg->msg_type = gtpv2c_rx->gtpc.message_type;

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
			send_version_not_supported(&msg->peer_addr, 
                                       cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
					                   gtpv2c_rx->teid.has_teid.seq);
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}
	}
    gtp_msg_handler[gtpv2c_rx->gtpc.message_type](&msg, gtpv2c_rx);

    if(msg->refCnt == 0)
        free(msg);

    return 0;
}

int
msg_handler_s5s8(void)
{
	int ret = 0;
	int bytes_s5s8_rx = 0;
    msg_info_t *msg = calloc(1, sizeof(msg_info_t *)); 
    struct sockaddr_in peer_sockaddr = {0};
    socklen_t peer_sockaddr_len = sizeof(peer_sockaddr);
    gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *) s5s8_rx_buf;


	bzero(&s5s8_rx_buf, sizeof(s5s8_rx_buf));
	gtpv2c_header_t *gtpv2c_s5s8_rx = (gtpv2c_header_t *) s5s8_rx_buf;

	bytes_s5s8_rx = recvfrom(my_sock.sock_fd_s5s8, s5s8_rx_buf,
			MAX_GTPV2C_UDP_LEN, MSG_DONTWAIT,
			(struct sockaddr *) &peer_sockaddr,
            &peer_sockaddr_len);

	if (bytes_s5s8_rx == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "s5s8 recvfrom error:"
				"\n\ton %s:%u - %s\n",
				inet_ntoa(peer_sockaddr.sin_addr),
				peer_sockaddr.sin_port,
				strerror(errno));
        return -1;
	}

	if ((bytes_s5s8_rx < 0) && (errno == EAGAIN  || errno == EWOULDBLOCK))
		return -1;

	if (!gtpv2c_s5s8_rx->gtpc.message_type) {
		return -1;
	}
    msg->peer_addr = peer_sockaddr;
    msg->source_interface = S5S8_IFACE;
	msg->msg_type = gtpv2c_rx->gtpc.message_type;


    if ((unsigned)bytes_s5s8_rx != (ntohs(gtpv2c_s5s8_rx->gtpc.message_len) + sizeof(gtpv2c_s5s8_rx->gtpc))) {
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
        return ret;
    }
    if(bytes_s5s8_rx > 0) {
		if (gtpv2c_rx->gtpc.version < GTP_VERSION_GTPV2C) {
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}else if (gtpv2c_rx->gtpc.version > GTP_VERSION_GTPV2C) {
			send_version_not_supported(&msg->peer_addr, 
                                       cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
					                   gtpv2c_rx->teid.has_teid.seq);
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}
	}
    gtp_msg_handler[gtpv2c_rx->gtpc.message_type](&msg, gtpv2c_rx);
    if(msg->refCnt == 0)
        free(msg);
    return 0;
}
