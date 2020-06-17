// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
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
#include "up_io_poll.h"

#include "pfcp_up_util.h"

//extern int errno;
extern udp_sock_t my_sock;

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

	int bytes = recvfrom(my_sock.sock_fd_pfcp, msg_payload, size, 0,
			(struct sockaddr *)peer_addr, &addr_len);
	/*if (bytes < size) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed recv msg !!!\n");
		return -1;
	}*/
	return bytes;
}

/**
 * @brief API to create udp socket.
 */
int create_udp_socket(struct in_addr recv_ip, uint16_t recv_port,
		udp_sock_t *sock)
{
	sock->sock_fd_pfcp = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock->sock_fd_pfcp == -1) {
		perror("socket error: ");
		close(sock->sock_fd_pfcp);
		return -1;
	}

	memset(&sock->my_addr, 0x0, sizeof(struct sockaddr_in));
	sock->my_addr.sin_family = AF_INET;
	sock->my_addr.sin_port = htons(recv_port);
	sock->my_addr.sin_addr.s_addr = recv_ip.s_addr;
	if (bind(sock->sock_fd_pfcp, (struct sockaddr *)&sock->my_addr,
			sizeof(struct sockaddr_in)) == -1)
		return -1;

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
	FD_SET(my_sock.sock_fd_pfcp, &readfds);

	n = my_sock.sock_fd_pfcp + 1;

	/* wait until either socket has data
	 *  ready to be recv()d (timeout 10.5 secs)
	 */
	/* NGCORE is defined by default */
	tv.tv_sec = 1;
	tv.tv_usec = 500000;
	rv = select(n, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		/*TODO: Need to Fix*/
		//perror("select"); /* error occurred in select() */
	} else if (rv > 0) {
		/* one or both of the descriptors have data */
		if (FD_ISSET(my_sock.sock_fd_pfcp, &readfds))
		{
				process_pfcp_msg(pfcp_rx, &peer_addr);
		}
	}
}

