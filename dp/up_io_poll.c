/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#include "up_interface.h"
#include "vepc_udp.h"
#include "up_io_poll.h"

#include "pfcp_up_util.h"

//extern int errno;

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

int
udp_recv(void *msg_payload, uint32_t size,
		struct sockaddr_in *peer_addr)
{
	socklen_t addr_len = sizeof(*peer_addr);

	int bytes = recvfrom(my_sock.sock_fd, msg_payload, size, 0,
			(struct sockaddr *)peer_addr, &addr_len);
	/*if (bytes < size) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed recv msg !!!\n");
		return -1;
	}*/
	return bytes;
}

/**
 * @brief Function to Process msgs.
 *
 */
int iface_remove_que(enum cp_dp_comm id)
{

	RTE_SET_USED(id);
	struct sockaddr_in peer_addr;
	int bytes_rx = 0;
	if ((bytes_rx = udp_recv(pfcp_rx, 512,
			&peer_addr)) < 0) {
		perror("msgrecv");
		return -1;
	}
	process_pfcp_msg(pfcp_rx, &peer_addr);

	return 0;
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

	n = my_sock.sock_fd + 1;

	/* wait until either socket has data
	 *  ready to be recv()d (timeout 10.5 secs)
	 */
#ifdef NGCORE_SHRINK
	/* NGCORE is defined by default */
	tv.tv_sec = 1;
	tv.tv_usec = 500000;
#else
	tv.tv_sec = 10;
	tv.tv_usec = 500000;
#endif
	rv = select(n, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		/*TODO: Need to Fix*/
		//perror("select"); /* error occurred in select() */
	} else if (rv > 0) {
		/* one or both of the descriptors have data */
		if (FD_ISSET(my_sock.sock_fd, &readfds))
		{
				process_pfcp_msg(pfcp_rx, &peer_addr);
		}
	}
}
