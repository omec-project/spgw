// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
#ifndef __DETACH_PROC__H
#define __DETACH_PROC__H

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t*
alloc_detach_proc(msg_info_t *msg);

void
proc_detach_failure(proc_context_t *proc, msg_info_t *msg, uint8_t cause);

void 
proc_detach_complete(proc_context_t *proc, msg_info_t *msg);

void 
detach_event_handler(void *proc, void *msg);

int
process_ds_req_handler(proc_context_t *proc_context, msg_info_t *data);

/* Function */
/**
 * @brief  : Handles processing of pfcp session delete response
 * @param  : arg1, data contained in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_sess_del_resp_handler(proc_context_t *proc_context, msg_info_t *arg1);

/* Function */
/**
 * @brief  : Process pfcp session deletion request
 * @param  : ds_req, holds information in session deletion request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_sess_del_request(proc_context_t *proc_context, msg_info_t *msg);

void process_pfcp_sess_del_request_timeout(void *data);

void
fill_pfcp_sess_mod_req_delete( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, ue_context_t *context, pdn_connection_t *pdn);


#ifdef __cplusplus
}
#endif
#endif

