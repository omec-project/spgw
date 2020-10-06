// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __CP_EVENTS_H
#define __CP_EVENTS_H

#include "spgw_cpp_wrapper.h"

#define UPF_CONNECTION_SETUP_SUCCESS 0x01 
#define UPF_CONNECTION_SETUP_FAILED  0x02
#define PFCP_SETUP_TIMEOUT           0x03
#define GTP_MSG_RECEIVED             0x04
#define PFCP_MSG_RECEIVED            0x05
#define LOCAL_MSG_RECEIVED           0x06
#define GX_MSG_RECEIVED              0x07
#define PEER_TIMEOUT                 0x08

static const char *event_names[] = { 
    "UNKNOWN",
    "UPF_CONNECTION_SETUP_SUCCESS", 
    "UPF_CONNECTION_SETUP_FAILED",
    "PFCP_SETUP_TIMEOUT",
    "GTP_MSG_RECEIVED",
    "PFCP_MSG_RECEIVED",
    "LOCAL_MSG_RECEIVED",
    "GX_MSG_RECEIVED",
    "PEER_TIMEOUT",
};

typedef void (*stack_event_handler) (void *, uint16_t);
typedef struct stack_event {
    uint16_t event;
    void     *data;
    stack_event_handler cb;
}stack_event_t;

static inline void 
queue_stack_unwind_event(uint16_t event, void *context, stack_event_handler cb)
{
    printf("Queue event %s \n",event_names[event]);
    stack_event_t *event_p = (stack_event_t *)calloc(1, sizeof(stack_event_t));
    event_p->event = event;
    event_p->data = context;
    event_p->cb = cb;
    queue_stack_unwind_event_cpp((void*)event_p);
    return;
}

static inline void*
get_stack_unwind_event(void)
{
    return get_stack_unwind_event_cpp();
}
#endif
