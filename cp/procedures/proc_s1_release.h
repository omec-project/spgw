
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef __RAB_PROC__H
#define __RAB_PROC__H
#include "gtp_messages.h"

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t*
alloc_rab_proc(msg_info_t *msg);

void 
rab_event_handler(void *proc, void *msg);

void 
proc_rab_complete(proc_context_t *proc_context);

void 
proc_rab_failed(msg_info_t *msg, uint8_t cause);


int
process_rel_access_ber_req_handler(proc_context_t *proc_context, msg_info_t *data);

int
process_release_access_bearer_request(proc_context_t *proc_context, msg_info_t *data);

void
process_release_access_bearer_request_pfcp_timeout(void *data);

void 
process_rab_proc_pfcp_mod_sess_rsp(proc_context_t *proc_context, msg_info_t *msg);

uint8_t
process_rab_pfcp_sess_mod_resp(proc_context_t *proc_context, 
                               uint64_t sess_id, 
                               gtpv2c_header_t *gtpv2c_tx);

#ifdef __cplusplus
}
#endif
#endif
