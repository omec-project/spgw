// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0


#ifndef __CP_EVENTS_H
#define __CP_EVENTS_H

#include "spgw_cpp_wrapper.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "cp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPF_CONNECTION_SETUP_SUCCESS 0x01 
#define UPF_CONNECTION_SETUP_FAILED  0x02
#define PFCP_SETUP_TIMEOUT           0x03
#define GTP_MSG_RECEIVED             0x04
#define PFCP_MSG_RECEIVED            0x05
#define LOCAL_MSG_RECEIVED           0x06
#define GX_MSG_RECEIVED              0x07
#define PEER_TIMEOUT                 0x08
#define TEST_EVENTS                  0x09
#define GTP_OUT_PKTS                 0x0a
#define UPF_ASSOCIATION_SETUP        0x0b

#if 0
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
    "TEST_EVENTS",
    "GTP_OUT_PKTS",
    "UPF_ASSOCIATION_SETUP",
};
#endif

typedef void (*stack_event_handler) (void *, uint16_t);
typedef struct stack_event {
    uint16_t event;
    void     *data;
    stack_event_handler cb;
}stack_event_t;

static inline void 
queue_stack_unwind_event(uint16_t event, void *context, stack_event_handler cb)
{
    //LOG_MSG(LOG_DEBUG, "Queue event %s ",event_names[event]);
    stack_event_t *event_p = (stack_event_t *)calloc(1, sizeof(stack_event_t));
    event_p->event = event;
    event_p->data = context;
    event_p->cb = cb;
    if(event == TEST_EVENTS) {
        queue_test_stack_unwind_event_cpp((void*)event_p);
        return;
    }
    queue_stack_unwind_event_cpp((void*)event_p);
    return;
}

static inline void*
get_stack_unwind_event(void)
{
    return get_stack_unwind_event_cpp();
}

static inline void*
get_test_stack_unwind_event(void)
{
    return get_test_stack_unwind_event_cpp();
}

typedef struct outgoing_pkts_event {
    int     fd;
    uint8_t *payload;
    uint16_t payload_len;
    struct sockaddr dest_addr;
}outgoing_pkts_event_t;

typedef void (*test_out_pkt_handler) (void *event);
typedef void (*test_in_pkt_handler) (void *msg, uint16_t event);

static inline void
queue_gtp_out_event(int fd,
                    uint8_t *payload,
                    uint16_t payload_len,
                    struct sockaddr *dest_addr)
{
    //LOG_MSG(LOG_DEBUG, "Queue event GTP_OUT_PKTS ");
    outgoing_pkts_event_t *event_p = (outgoing_pkts_event_t*)calloc(1, sizeof(outgoing_pkts_event_t));
    event_p->payload = (uint8_t*)calloc(1, payload_len + 10);
    event_p->fd = fd;
    event_p->payload_len = payload_len;
    event_p->dest_addr = *dest_addr;
    memcpy(event_p->payload, payload, payload_len);
    queue_gtp_out_event_cpp((void*)event_p);
    return;
}

static inline void
queue_pfcp_out_event(int fd,
                    uint8_t *payload,
                    uint16_t payload_len,
                    struct sockaddr *dest_addr)
{
    //LOG_MSG(LOG_DEBUG,"Queue event PFCP_OUT_PKTS ");
    outgoing_pkts_event_t *event_p = (outgoing_pkts_event_t*)calloc(1, sizeof(outgoing_pkts_event_t));
    event_p->payload = (uint8_t*)calloc(1, payload_len + 10);
    event_p->fd = fd;
    event_p->payload_len = payload_len;
    event_p->dest_addr = *dest_addr;
    memcpy(event_p->payload, payload, payload_len);
    queue_pfcp_out_event_cpp((void*)event_p);
    return;
}

static inline void
queue_gx_out_event(int fd,
                    uint8_t *payload,
                    uint16_t payload_len)
{
    //LOG_MSG(LOG_DEBUG,"Queue event GX_OUT_PKTS ");
    outgoing_pkts_event_t *event_p = (outgoing_pkts_event_t*)calloc(1, sizeof(outgoing_pkts_event_t));
    event_p->payload = payload; 
    event_p->fd = fd;
    event_p->payload_len = payload_len;
    memcpy(event_p->payload, payload, payload_len);
    queue_gx_out_event_cpp((void*)event_p);
    return;
}

#ifdef __cplusplus
}
#endif
#endif
