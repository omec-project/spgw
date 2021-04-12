// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef __SERVICE_REQUEST_PROC_H
#define __SERVICE_REQUEST_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

proc_context_t*
alloc_service_req_proc(msg_info_t *msg);

void 
service_req_event_handler(void *proc, void *msg);

void
proc_service_request_complete(proc_context_t *proc_context);

void
proc_service_request_success(proc_context_t *proc);

void
proc_service_request_failure(msg_info_t *msg, uint8_t cause);

/* Function */
/**
 * @brief  : Handles processing of modify bearer request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_mb_req_handler(proc_context_t *proc_context, msg_info_t *arg1);

void
process_pfcp_sess_mod_request_timeout(void *data);

void 
process_service_request_pfcp_mod_sess_rsp(proc_context_t *proc_context, msg_info_t *msg);

uint8_t
process_srreq_pfcp_sess_mod_resp(proc_context_t *proc_context, 
                                 uint64_t sess_id, 
                                 gtpv2c_header_t *gtpv2c_tx);

#ifdef __cplusplus
}
#endif
#endif
