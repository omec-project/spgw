// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __CP_TEST__H_
#define __CP_TEST__H_
#include "cp_events.h"

#ifdef __cplusplus
extern "C" {
#endif
void init_mock_test(void);
void *test_event_thread(void*);
void test_event_handler(void*, uint16_t);
void init_gtp_mock_interface(void);
void init_pfcp_mock_interface(void);
void init_gx_mock_interface(void);

void handle_mock_create_bearer_request_msg(void *event);
void handle_unknown_mock_msg(void *event);

extern test_out_pkt_handler gtp_out_mock_handler[256];
extern test_out_pkt_handler pfcp_out_mock_handler[256];
extern test_out_pkt_handler gx_out_mock_handler[256];

extern test_in_pkt_handler gtp_in_mock_handler[256];
extern test_in_pkt_handler pfcp_in_mock_handler[256];
extern test_in_pkt_handler gx_in_mock_handler[256];

void handle_mock_rar_request_msg(void *, uint16_t);

#ifdef __cplusplus
}
#endif
#endif
