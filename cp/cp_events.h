// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __CP_EVENTS_H
#define __CP_EVENTS_H

#include "spgw_cpp_wrapper.h"

#define UPF_CONNECTION_SETUP_SUCCESS 0x01 
#define UPF_CONNECTION_SETUP_FAILED  0x02
#define PFCP_SETUP_TIMEOUT           0x03

typedef void (*stack_event_handler) (void *, uint16_t);
typedef struct stack_event {
    uint16_t event;
    void     *data;
    stack_event_handler cb;
}stack_event_t;

inline void 
queue_stack_unwind_event(uint16_t event, void *context, stack_event_handler cb)
{
    printf("Queue event %d \n",event);
    stack_event_t *event_p = (stack_event_t *)calloc(1, sizeof(stack_event_t));
    event_p->event = event;
    event_p->data = context;
    event_p->cb = cb;
    queue_stack_unwind_event_cpp((void*)event_p);
    return;
}
inline void*
get_stack_unwind_event(void)
{
    return get_stack_unwind_event_cpp();
}
#endif
