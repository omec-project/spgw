// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _PROC_BEARER_CREATE_H__
#define _PROC_BEARER_CREATE_H__

#include <stdint.h>
#include "gtp_ies.h"
#include "cp_proc.h"

/**
 * @brief  : Handles processing of pfcp session modification response in case create bearer request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_sess_mod_resp_cbr_handler(void *arg1, void *arg2);

/* Function */
/**
 * @brief  : Handles processing of create bearer response for sgwc
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_create_bearer_resp_handler(void *arg1, void *arg2);

uint8_t
process_pfcp_sess_mod_resp_cbr(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx);


void process_pfcp_sess_mod_resp_cbr_timeout(void *data);

int
process_pgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg);

int
process_sgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg);


void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data);

void
process_sgwc_create_bearer_rsp_pfcp_timeout(void *data);
#endif

