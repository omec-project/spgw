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
#include "cp_log.h"


void 
process_local_msg(void *data, uint16_t event)
{
    LOG_MSG(LOG_INFO,"Process local message event ");
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
    LOG_MSG(LOG_INIT, "Starting main-event handler thread ");
    while(1) {
        event  = (stack_event_t *)get_stack_unwind_event();
        if(event != NULL)
        {
            LOG_MSG(LOG_DEBUG,"handle stack unwind event %s ",event_names[event->event]);
            event->cb(event->data, event->event);
            free(event);
            continue;
        }
        usleep(10);
    }
    LOG_MSG(LOG_ERROR,"exiting event handler thread ");
    return NULL;
}
