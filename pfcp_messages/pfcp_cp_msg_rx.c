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
#include "gw_adapter.h"
#include "clogger.h"
#include "pfcp_cp_interface.h"
#include "cp_peer.h"
#include "sm_structs_api.h"
#include "pfcp.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "cp_config.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config_defs.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include <unistd.h>

extern udp_sock_t my_sock;
extern struct rte_hash *heartbeat_recovery_hash;
uint8_t pfcp_rx[1024]; /* TODO: Decide size */

 
/* TODO: Parse byte_rx to msg_handler_sx_n4 */
int
msg_handler_sx_n4(void)
{
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int bytes_pfcp_rx = 0;
	msg_info_t msg = {0};
	struct sockaddr_in peer_addr = {0};

	bytes_pfcp_rx = recvfrom(my_sock.sock_fd_pfcp, pfcp_rx, 512, MSG_DONTWAIT,
			(struct sockaddr *)(&peer_addr), &addr_len);

    if ((bytes_pfcp_rx < 0) &&
            (errno == EAGAIN  || errno == EWOULDBLOCK)) {
        return -1; // Read complete data 
    }

    if (bytes_pfcp_rx == 0) {
        clLog(clSystemLog, eCLSeverityCritical, "SGWC|SAEGWC_s11 recvfrom error: %s",
                strerror(errno));
        return -1;
    }

	pfcp_header_t *pfcp_header = (pfcp_header_t *) pfcp_rx;
	msg.msg_type = pfcp_header->message_type;
    msg.peer_addr = peer_addr;
    msg.rx_interface = PGW_SXB; 
    printf("pfcp_header->message_type = %d \n",pfcp_header->message_type);
    printf("pfcp_header->message_type = %p \n",pfcp_msg_handler[pfcp_header->message_type]);
    sleep(4);
    pfcp_msg_handler[pfcp_header->message_type](&msg, pfcp_header);
	return 0;
}
