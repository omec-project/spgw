// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _PROC_BEARER_CREATE_H__
#define _PROC_BEARER_CREATE_H__

#include <stdint.h>
#include "gtp_ies.h"
#include "proc_struct.h"
#include "ue.h"
#include "pdn.h"
#include "bearer.h"

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t*
alloc_bearer_create_proc(msg_info_t *msg);

void
bearer_create_event_handler(void *proc, void *msg_info);

void
proc_bearer_create_failed(proc_context_t *proc, uint8_t cause);

void 
proc_bearer_create_complete(proc_context_t *proc_context);

void 
pfcp_modify_session_pre_cbreq_timeout(void *);

void 
send_pfcp_modify_session_pre_cbreq(void *proc, void *msg);

int
fill_pfcp_gx_sess_mod_req(proc_context_t *proc,  pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn);

void 
process_pfcp_sess_mod_rsp_post_cbr_handler(proc_context_t *proc);

void 
process_sgwc_pfcp_mod_post_cbrsp_timeout(void *data);

/**
 * @brief  : Handles processing of pfcp session modification response in case create bearer request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_sess_mod_resp_pre_cbr_handler(void *arg1, void *arg2);

/* Function */
/**
 * @brief  : Handles processing of create bearer response for sgwc
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_create_bearer_resp_handler(void *arg1, void *arg2);

void 
process_cbr_timeout(void *data);

int
process_pgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg);

int
process_sgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg);


void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data);

void
process_sgwc_create_bearer_rsp_pfcp_timeout(void *data);

int
fill_pfcp_entry(eps_bearer_t *bearer, dynamic_rule_t *dyn_rule,
		enum rule_action_t rule_action);

/**
 * @brief  : Handles processing of create bearer response for pgwc
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_cbresp_handler(void *arg1, void *arg2);


#ifdef __cplusplus
}
#endif
#endif
