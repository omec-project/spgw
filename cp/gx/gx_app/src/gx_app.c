// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include "ipc_api.h"
#include "gx.h"
#include "cp_log.h"

extern int done ;
int g_gx_client_sock = 0;

void hexDump(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	// Output description if given.
	if (desc != NULL)
		printf ("%s:\n", desc);

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).
		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		} else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}

int
recv_msg_handler( int sock )
{

	int bytes_recv = 0;
	gx_msg *req = NULL;
	char buf[BUFFSIZE] = {0};

	bytes_recv = recv_from_ipc_channel(sock, buf);
	hexDump(NULL, buf, bytes_recv);
    LOG_MSG(LOG_DEBUG, "Received bytes(%d) from spgwc app ",bytes_recv);
	if(bytes_recv > 0) {
		gx_msg *req = (gx_msg*)buf;
		switch (req->msg_type){
			case GX_CCR_MSG:
				gx_send_ccr(&(req->data.ccr));
				break;

			case GX_RAA_MSG:
				gx_send_raa(&buf[0]);
				break;

			default:
				LOG_MSG(LOG_ERROR,"Unknown message received from CP app - %d",
						req->msg_type);
		}
	}
}


int unixsock()
{
	int ret = 0;
	int n, rv;
	fd_set readfds;
	struct timeval tv;

	/* clear the set ahead of time */
	FD_ZERO(&readfds);

	struct sockaddr_un gx_app_sockaddr = {0};
	struct sockaddr_un cp_app_sockaddr = {0};

	g_gx_client_sock = create_ipc_channel();

	connect_to_ipc_channel( g_gx_client_sock, cp_app_sockaddr, "/tmp/sock_server");

	while(1) {
		/* add our descriptors to the set */
		FD_SET(g_gx_client_sock, &readfds);

		n = g_gx_client_sock + 1;

		/* wait until either socket has data
		 *  ready to be recv()d (timeout 10.5 secs)
		 */
		tv.tv_sec  = 10;
		tv.tv_usec = 500000;
		rv = select(n, &readfds, NULL, NULL, &tv);
		if (rv == -1)	{
			if( errno == EINTR && done == 1  )
				break;
			perror("select");	/* error occurred in select() */
		} else if (rv > 0) {
			if (FD_ISSET(g_gx_client_sock, &readfds)){
				ret = recv_msg_handler(g_gx_client_sock);
			}
		}
	}
	return 0;
}
