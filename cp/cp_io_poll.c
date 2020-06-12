/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <rte_common.h>
#include <rte_eal.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_cfgfile.h>
#include <rte_errno.h>
#include <errno.h>

#include "cp_interface.h"
#include "cp_io_poll.h"
#include "pfcp_cp_common.h"

#include "pfcp_cp_util.h"
#include "../cp/cp.h"
#include "gx_app_interface.h"
#include "../cp/cp_stats.h"
#include "../cp/cp_config.h"
#include "../cp/state_machine/sm_struct.h"

#ifdef GX_BUILD
extern int gx_app_sock;
#endif /* GX_BUILD */
udp_sock_t my_sock;

void iface_ipc_register_msg_cb(int msg_id,
				int (*msg_cb)(struct msgbuf *msg_payload))
{
	struct ipc_node *node;

	node = &basenode[msg_id];
	node->msg_id = msg_id;
	node->msg_cb = msg_cb;
}

/********************************** DP API ************************************/
void iface_init_ipc_node(void)
{
	basenode = rte_zmalloc("iface_ipc", sizeof(struct ipc_node) * MSG_END,
			RTE_CACHE_LINE_SIZE);
	if (basenode == NULL)
		exit(0);
}

/**
 * @brief Function to Poll message que.
 *
 */
void iface_process_ipc_msgs(void)
{
	int n = 0, rv = 0;
	fd_set readfds = {0};
	struct timeval tv = {0};
	struct sockaddr_in peer_addr = {0};

	FD_ZERO(&readfds);

	/* Add PFCP_FD in the set */
	FD_SET(my_sock.sock_fd, &readfds);

	int max = 0;

	/* Add S11_FD in the set */
	if ((cp_config->cp_type  == SGWC) || (cp_config->cp_type == SAEGWC)) {
		FD_SET(my_sock.sock_fd_s11, &readfds);
	}

	/* Add S5S8_FD in the set */
	if (cp_config->cp_type != SAEGWC) {
		FD_SET(my_sock.sock_fd_s5s8, &readfds);
	}

#ifdef GX_BUILD
	/* Add GX_FD in the set */
	if ((cp_config->cp_type == PGWC ) || (cp_config->cp_type == SAEGWC)) {
		FD_SET(gx_app_sock, &readfds);
	}
#endif /* GX_BUILD */

	/* Set the MAX FD's stored into the set */
	if (cp_config->cp_type == SGWC) {
		max = (my_sock.sock_fd > my_sock.sock_fd_s11 ?
				my_sock.sock_fd : my_sock.sock_fd_s11);
		max = (max > my_sock.sock_fd_s5s8 ? max : my_sock.sock_fd_s5s8);
	}
	if (cp_config->cp_type == SAEGWC) {
		max = (my_sock.sock_fd > my_sock.sock_fd_s11 ?
				my_sock.sock_fd : my_sock.sock_fd_s11);
#ifdef GX_BUILD
		max = (gx_app_sock > max ? gx_app_sock : max);
#endif /* GX_BUILD */
	}
	if (cp_config->cp_type == PGWC) {
		max = (my_sock.sock_fd > my_sock.sock_fd_s5s8 ?
				my_sock.sock_fd : my_sock.sock_fd_s5s8);
#ifdef GX_BUILD
		max = (gx_app_sock > max ? gx_app_sock : max);
#endif /* GX_BUILD */
	}

	n = max + 1;
	/* wait until either socket has data
	 *  ready to be recv()d (timeout 10.5 secs)
	 */
	tv.tv_sec = 1;
	tv.tv_usec = 500000;
	rv = select(n, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		/*TODO: Need to Fix*/
		//perror("select"); /* error occurred in select() */
	} else if (rv > 0) {
		/* one or both of the descriptors have data */
		if (FD_ISSET(my_sock.sock_fd, &readfds))
		{
				msg_handler_sx_n4(pfcp_rx, &peer_addr);
		}

		if ((cp_config->cp_type  == SGWC) || (cp_config->cp_type == SAEGWC)) {
			if (FD_ISSET(my_sock.sock_fd_s11, &readfds)) {
					msg_handler_s11();
			}
		}

		if (cp_config->cp_type != SAEGWC) {
			if (FD_ISSET(my_sock.sock_fd_s5s8, &readfds)) {
					msg_handler_s5s8();
			}
		}

#ifdef GX_BUILD
		/* Refer - cp/Makefile. For now this is disabled. */
		if ((cp_config->cp_type == PGWC) || (cp_config->cp_type == SAEGWC)) {
			if (FD_ISSET(gx_app_sock, &readfds)) {
					msg_handler_gx();
			}
		}
#endif  /* GX_BUILD */

	}
}

