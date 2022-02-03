// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "cp_interface.h"
#include "cp_io_poll.h"
#include "spgw_config_struct.h"
#include "sm_struct.h"
#include "cp_config_apis.h"
#include "upf_apis.h"
#include "spgw_cpp_wrapper.h"
#include "cp_events.h"
#include "cp_log.h"


void 
process_local_msg(void *data, uint16_t event)
{
    LOG_MSG(LOG_DEBUG,"Process local message event start");
    struct t2tMsg *evt  = (struct t2tMsg *)get_t2tMsg();
    while(evt != NULL) {
        LOG_MSG(LOG_NEVER,"data %p event %d ", data, event);
        cp_config->subscriber_rulebase = (spgw_config_profile_t *)evt->data;
        free(evt);
        evt  = (struct t2tMsg *)get_t2tMsg();
    }
    schedule_pfcp_association(10, NULL);
    LOG_MSG(LOG_DEBUG,"Process local message event done");
    return;
}

void* 
incoming_event_handler(void* data)
{
    stack_event_t *event;
    LOG_MSG(LOG_INIT, "Starting main-event handler thread ");
    while(1) {
        event  = (stack_event_t *)get_stack_unwind_event();
        if(event != NULL)
        {
            //LOG_MSG(LOG_DEBUG,"handle stack unwind event %s ",event_names[event->event]);
            event->cb(event->data, event->event);
            free(event);
            continue;
        }
        usleep(100); // every pkt 0.1 ms default scheduling delay
    }
    LOG_MSG(LOG_ERROR,"exiting event handler thread %p", data);
    return NULL;
}
