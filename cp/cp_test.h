// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __CP_TEST__H_
#define __CP_TEST__H_
#include "cp_events.h"

void *test_event_thread(void*);
void test_event_handler(void*, uint16_t);
void init_gtp_mock_interface(void);

void handle_mock_create_bearer_request_msg(void *event);
void handle_unknown_mock_msg(void *event);

extern test_out_gtp_handler gtp_mock_handler[256];
#endif
