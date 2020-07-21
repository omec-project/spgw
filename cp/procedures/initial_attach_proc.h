// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PROC_APIS_H__
#define __PROC_APIS_H__
#include "cp_proc.h"

proc_context_t*
alloc_initial_proc(msg_info_t *msg);

void 
initial_attach_event_handler(void *proc, uint32_t event, void *data);

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
process_create_sess_req(create_sess_req_t *csr,
					ue_context_t **context, 
                    pdn_connection_t **pdn_ctxt,
                    msg_info_t *msg);

int
process_sess_est_resp_handler(void *data, void *unused_param);

#endif
