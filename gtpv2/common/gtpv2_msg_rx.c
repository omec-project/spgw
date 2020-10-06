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
#include "cp_events.h"

uint8_t gtp_rx_buf[MAX_GTPV2C_UDP_LEN];

uint32_t start_time;

/* Requirement1 - multiple read to read all the messages from the sockets 
 */
void*
msg_handler_gtp(void *data)
{
    RTE_SET_USED(data);
    while(1) {
        int ret = 0, bytes_rx = 0;
        msg_info_t *msg = calloc(1, sizeof(msg_info_t));
        struct sockaddr_in peer_sockaddr = {0};
        socklen_t peer_sockaddr_len = sizeof(peer_sockaddr);

        bzero(&gtp_rx_buf, sizeof(gtp_rx_buf));
        gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *) gtp_rx_buf;

        bytes_rx = recvfrom(my_sock.sock_fd_s11, gtp_rx_buf, MAX_GTPV2C_UDP_LEN, 0,
                (struct sockaddr *) &peer_sockaddr,
                &peer_sockaddr_len);

        if (bytes_rx == 0) {
            clLog(clSystemLog, eCLSeverityCritical, "SGWC|SAEGWC_s11 recvfrom error:"
                    "\n\ton %s:%u - %s\n",
                    inet_ntoa(peer_sockaddr.sin_addr),
                    peer_sockaddr.sin_port,
                    strerror(errno));
            continue; 
        }

        if ((bytes_rx < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            continue; 
        }

        if (!gtpv2c_rx->gtpc.message_type) {
            continue; 
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
            RTE_SET_USED(ret);
            continue; 
        }

        msg->raw_buf = calloc(1, bytes_rx);
        memcpy(msg->raw_buf, gtp_rx_buf, bytes_rx);
        queue_stack_unwind_event(GTP_MSG_RECEIVED, (void *)msg, process_gtp_msg);
    }
    return 0;
}


