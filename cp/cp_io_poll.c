// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <rte_common.h>
#include <rte_eal.h>
#include <rte_malloc.h>
#include <rte_cfgfile.h>
#include <rte_errno.h>
#include <errno.h>

#include "cp_interface.h"
#include "cp_io_poll.h"
#include "pfcp_cp_util.h"
#include "../cp/cp_stats.h"
#include "../cp/cp_config.h"
#include "../cp/state_machine/sm_struct.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "cp_events.h"

udp_sock_t my_sock = {0};

/**
 * @brief Function to Poll message que.
 *
 */
void iface_process_ipc_msgs(void)
{
	int n = 0, rv = 0;
	fd_set readfds = {0};
	struct timeval tv = {0};
    int ret;
    stack_event_t *event;

	FD_ZERO(&readfds);

	/* Add PFCP_FD in the set */
	FD_SET(my_sock.sock_fd_pfcp, &readfds);

	/* Add S11_FD in the set */
	if (my_sock.sock_fd_s11) {
		FD_SET(my_sock.sock_fd_s11, &readfds);
	}
#ifdef FUTURE_NEED
	/* Add S5S8_FD in the set */
	if (my_sock.sock_fd_s5s8) {
		FD_SET(my_sock.sock_fd_s5s8, &readfds);
	}

	/* Add GX_FD in the set */
	if (my_sock.gx_app_sock) {
		FD_SET(my_sock.gx_app_sock, &readfds);
	}
#endif

	n = my_sock.select_max_fd;

	/* wait until either socket has data
	 *  ready to be recv()d (timeout 10.5 secs)
	 */
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	rv = select(n, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		/*TODO: Need to Fix*/
		//perror("select"); /* error occurred in select() */
	} else if (rv > 0) {
		/* one or both of the descriptors have data */
        if (FD_ISSET(my_sock.sock_fd_pfcp, &readfds)) {
            printf("N4 Packet read event received \n");
            ret = 0;
            while(ret == 0) {
                ret = msg_handler_sx_n4();
                /* After processing each message see if any stack unwind events */
                event  = (stack_event_t *)get_stack_unwind_event();
                while(event != NULL)
                {
                    printf("handle stack unwind event %d \n",event->event);
                    event->cb(event->data, event->event);
                    free(event);
                    event  = (stack_event_t *)get_stack_unwind_event();
                }
            }
            printf("N4 Packet read complete\n");
        }

        if ((cp_config->cp_type  == SGWC) || (cp_config->cp_type == SAEGWC)) {
            if (FD_ISSET(my_sock.sock_fd_s11, &readfds)) {
                printf("S11 Packet read event received \n");
                ret = 0;
                while(ret == 0) {
                    ret = msg_handler_s11();
                    /* After processing each message see if any stack unwind events */
                    event  = (stack_event_t *)get_stack_unwind_event();
                    while(event != NULL)
                    {
                        printf("handle stack unwind event %d \n",event->event);
                        event->cb(event->data, event->event);
                        free(event);
                        event  = (stack_event_t *)get_stack_unwind_event();
                    }
                }
                printf("S11 Packet read complete\n");
            }
        }

#ifdef FUTURE_NEED
		if (cp_config->cp_type != SAEGWC) {
			if (FD_ISSET(my_sock.sock_fd_s5s8, &readfds)) {
                ret = 0;
                while(ret == 0) {
					ret = msg_handler_s5s8(void);
                    /* After processing each message see if any stack unwind events */
                    event  = (stack_event_t *)get_stack_unwind_event();
                    while(event != NULL)
                    {
                        event->cb(event->data, event->event);
                        free(event);
                        event  = (stack_event_t *)get_stack_unwind_event();
                    }
                }
			}
		}
		/* Refer - cp/Makefile. For now this is disabled. */
		if ((cp_config->cp_type == PGWC) || (cp_config->cp_type == SAEGWC)) {
			if (FD_ISSET(my_sock.gx_app_sock, &readfds)) {
                ret = 0;
                while(ret == 0) {
					ret = msg_handler_gx();
                    /* After processing each message see if any stack unwind events */
                    event  = (stack_event_t *)get_stack_unwind_event();
                    while(event != NULL)
                    {
                        event->cb(event->data, event->event);
                        free(event);
                        event  = (stack_event_t *)get_stack_unwind_event();
                    }
                }
			}
		}
#endif
	}
}

