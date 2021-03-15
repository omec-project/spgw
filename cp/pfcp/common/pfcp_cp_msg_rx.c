// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <byteswap.h>
#include "pfcp_cp_util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "pfcp_cp_interface.h"
#include "cp_peer.h"
#include "sm_structs_api.h"
#include "pfcp.h"
#include "sm_struct.h"
#include "spgw_config_struct.h"
#include "gtpv2_error_rsp.h"
#include "cp_config_defs.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include <unistd.h>
#include "cp_io_poll.h"
#include "cp_events.h"
#include "cp_test.h"
#include "cp_log.h"

uint8_t pfcp_rx[1024]; /* TODO: Decide size */

 
/* TODO: Parse byte_rx to msg_handler_sx_n4 */
void*
msg_handler_pfcp(void *data)
{
    LOG_MSG(LOG_INIT,"Starting pfcp message handler thread ");
    while (1) {
        socklen_t addr_len = sizeof(struct sockaddr_in);
        int bytes_pfcp_rx = 0;
        msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t)); 
        struct sockaddr_in peer_addr = {0};

        bytes_pfcp_rx = recvfrom(my_sock.sock_fd_pfcp, pfcp_rx, sizeof(pfcp_rx), 0, (struct sockaddr *)(&peer_addr), &addr_len);

        if ((bytes_pfcp_rx < 0) && (errno == EAGAIN  || errno == EWOULDBLOCK)) {
            LOG_MSG(LOG_ERROR, "SAEGWC pfcp recvfrom error: %s", strerror(errno));
            continue;
        }

        pfcp_header_t *pfcp_header = (pfcp_header_t *) pfcp_rx;
        msg->magic_head = MSG_MAGIC;
        msg->magic_tail = MSG_MAGIC;
        msg->msg_type = pfcp_header->message_type;
        msg->peer_addr = peer_addr;
        msg->source_interface = PFCP_IFACE; 
        msg->raw_buf = calloc(1, bytes_pfcp_rx);
        memcpy(msg->raw_buf, pfcp_header, bytes_pfcp_rx);
		if(pfcp_in_mock_handler[msg->msg_type] != NULL) {
			queue_stack_unwind_event(PFCP_MSG_RECEIVED, (void *)msg, pfcp_in_mock_handler[msg->msg_type]);
		} else {
			queue_stack_unwind_event(PFCP_MSG_RECEIVED, (void *)msg, process_pfcp_msg);
		}
    }
    LOG_MSG(LOG_ERROR,"exiting pfcp message handler thread data = %p ", data);
	return NULL;
}
