/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __GTPV2_EVT_HANDLER_H
#define __GTPV2_EVT_HANDLER_H
#include "sm_struct.h"
int handle_create_session_request_msg(msg_info *msg);
int handle_modify_bearer_request_msg(msg_info *msg);
int handle_delete_session_request_msg(msg_info *msg);
#endif
