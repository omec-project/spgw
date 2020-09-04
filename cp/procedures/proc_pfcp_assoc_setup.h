// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PFCP_ASSOCIATION_SETUP_PROC_H 
#define __PFCP_ASSOCIATION_SETUP_PROC_H
proc_context_t*
alloc_pfcp_association_setup_proc(void *upf);

void 
pfcp_association_event_handler(void *proc, void *msg);

/**
 * @brief  : Handles association setuo request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */

int
association_setup_handler(proc_context_t *proc, msg_info_t *msg);

int
buffer_csr_request(proc_context_t *proc_context);

int 
handle_pfcp_association_setup_response(void *msg_t);

void
process_assoc_resp_timeout_handler(void *data1);

void 
upf_pfcp_setup_failure(void *data, uint16_t event);

void upf_pfcp_setup_success(void *data, uint16_t event);

int buffer_csr_request(proc_context_t *proc_context);
#endif
