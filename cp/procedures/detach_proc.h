
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef __DETACH_PROC__H
#define __DETACH_PROC__H

proc_context_t*
alloc_detach_proc(msg_info_t *msg);

void
proc_detach_failure(msg_info_t *msg, uint8_t cause);

void proc_detach_complete(proc_context_t *proc_context);


void 
detach_event_handler(void *proc, uint32_t event, void *data);

int
process_ds_req_handler(void *data, void *unused_param);

/* Function */
/**
 * @brief  : Handles processing of pfcp session delete response
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_sess_del_resp_handler(void *arg1, void *arg2);

/* Function */
/**
 * @brief  : Handles processing of delete session response
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_ds_resp_handler(void *arg1, void *arg2);

/**
 * @brief  : Process pfcp session deletion request
 * @param  : ds_req, holds information in session deletion request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_sess_del_request(msg_info_t *msg, del_sess_req_t *ds_req);
void process_pfcp_sess_del_request_timeout(void *data);

#endif

