// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef __NW_INIT_DETACH_PROC__H
#define __NW_INIT_DETACH_PROC__H

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t*
alloc_nw_init_detach_proc(msg_info_t *msg);

void 
nw_init_detach_event_handler(void *proc, void *msg_info);

void
proc_nw_init_detach_success(proc_context_t *proc_context);

void
proc_nw_init_detach_failure(proc_context_t *proc_context);

void 
proc_nw_init_detach_complete(proc_context_t *proc_context);


void
proc_nw_init_detach_pfcp_sess_del_request_timeout(void *data);

int
send_nw_init_detach_pfcp_sess_del_request(proc_context_t *proc_context, msg_info_t *msg);

int
process_nw_init_detach_sess_del_resp_handler(proc_context_t *proc_context, msg_info_t *msg);

int
generate_nw_init_detach_dbreq(proc_context_t *proc_context);

void
process_nw_init_detach_dbrsp_handler(proc_context_t *proc_context, msg_info_t *msg);

int
generate_ccrt(proc_context_t *proc_context);

void
process_nw_init_detach_cca(proc_context_t *proc, msg_info_t *msg);

void nw_init_dbreq_timeout(void *data);

#ifdef __cplusplus
}
#endif
#endif

