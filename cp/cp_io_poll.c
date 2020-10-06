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
#include "cp_config.h"
#include "sm_struct.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "cp_config_apis.h"
#include "spgw_cpp_wrapper.h"
#include "cp_events.h"
#include "rte_common.h"

void* 
msg_handler_local(void *data)
{
    int bytes_rx;
    uint8_t rx_buf[128];
    RTE_SET_USED(data);
    while(1) {
        bytes_rx = recv(my_sock.sock_fd_local, rx_buf, sizeof(rx_buf), 0);
        if(bytes_rx != 0) {
            printf("Read the config read event \n");
            queue_stack_unwind_event(LOCAL_MSG_RECEIVED, (void *)NULL, process_local_msg);
        }
    }
    return NULL;
}

void 
process_local_msg(void *data, uint16_t event)
{
    printf("Process local message event \n");
    RTE_SET_USED(data);
    RTE_SET_USED(event);
    struct t2tMsg *evt  = (struct t2tMsg *)get_t2tMsg();
    while(evt != NULL) {
        update_subscriber_analyzer_config(evt->data, evt->event);
        free(evt);
        evt  = (struct t2tMsg *)get_t2tMsg();
    }
    return;
}

void* 
incoming_event_handler(void* data)
{
    RTE_SET_USED(data);
    stack_event_t *event;
    while(1) {
        event  = (stack_event_t *)get_stack_unwind_event();
        if(event != NULL)
        {
            clLog(clSystemLog, eCLSeverityCritical,"handle stack unwind event %s ",event_names[event->event]);
            event->cb(event->data, event->event);
            free(event);
            continue;
        }
        usleep(10);
    }
    return NULL;
}
