// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PROC_INITIAL_ATTACH_H__
#define __PROC_INITIAL_ATTACH_H__
#include "proc_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t*
alloc_initial_proc(msg_info_t *msg);

void 
initial_attach_event_handler(void *proc, void *m); 

void 
proc_initial_attach_success(proc_context_t *proc_context);

void 
proc_initial_attach_complete(proc_context_t *proc_context);

void 
proc_initial_attach_failure(proc_context_t *proc_context, int cause);

int
handle_csreq_msg(proc_context_t *proc_context, msg_info_t *msg);

/**
 * @brief  : Process create session request, update ue context, bearer info
 * @param  : csr, holds information in csr
 * @param  : context, ue context structure pointer
 * @param  : upf_ipv4, upf ip
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_create_sess_req(proc_context_t *proc,
                        create_sess_req_t *csr,
					    ue_context_t **context,
                        pdn_connection_t **pdn_ctxt,
                        msg_info_t *msg);

int
process_sess_est_resp_handler(proc_context_t *p, msg_info_t *d);


int
process_sess_est_resp_timeout_handler(proc_context_t *proc, msg_info_t *data);

int
process_sess_est_resp_association_failed_handler(proc_context_t *proc, msg_info_t *data);

/**
 * @brief  : Handles processing of cca message
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int cca_msg_handler(proc_context_t *proc, msg_info_t *msg);

int
process_gx_ccai_reject_handler(proc_context_t *proc_context, msg_info_t *msg);
/**
 * @brief  : Process pfcp session establishment request
 * @return : Returns 0 in case of success , -1 otherwise
 */
transData_t *
process_pfcp_sess_est_request(proc_context_t *proc_context,  upf_context_t *upf_ctx);

void process_pfcp_sess_est_request_timeout(void *data);

/**
 * @brief  : Process pfcp session establishment response
 * @param  : pfcp_sess_est_rsp, structure to be filled
 * @param  : gtpv2c_tx, holds info in gtpv2c header
 * @retrun : Returns 0 in case of success
 */
int8_t
process_pfcp_sess_est_resp(msg_info_t *msg, 
                           pfcp_sess_estab_rsp_t *pfcp_sess_est_rsp, 
                           gtpv2c_header_t *gtpv2c_tx);

#ifdef __cplusplus
}
#endif
#endif
