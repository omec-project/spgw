// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PROC_SESSION_REPORT_H
#define __PROC_SESSION_REPORT_H

proc_context_t*
alloc_session_report_proc(msg_info_t **msg);

void
session_report_event_handler(void *proc, void *msg);

/**
 * @brief  : Handles processing of report request
 * @param  : arg1, data contained in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_rpt_req_handler(proc_context_t *proc_context, msg_info_t *arg1);

void
proc_session_report_failure(msg_info_t *msg, uint8_t cause);

void
proc_session_report_success(msg_info_t *msg);

void
proc_session_report_complete(proc_context_t *proc_context);

void
process_ddn_ack_rsp(proc_context_t *proc_context, msg_info_t *msg);

void ddn_indication_timeout(void *);

#endif 
