// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PROC_SGW_RELOCATION_H__
#define __PROC_SGW_RELOCATION_H__
#include "proc_struct.h"
#include "sm_struct.h"
#include "ue.h"
#include "pdn.h"
#include "bearer.h"

#ifdef __cplusplus
extern "C" {
#endif

proc_context_t* alloc_sgw_relocation_proc(msg_info_t *msg);
void sgw_relocation_event_handler(void *proc, void *msg);

/**
 * @brief  : Handles processing of session establishment response received in case of sgw relocation
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_sess_est_resp_sgw_reloc_handler(void *data, void *unused_param);

/**
 * @brief  : Handles processing of modify bearer request received in case of sgw relocation
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_mb_req_sgw_reloc_handler(void *arg1, void *arg2);
/**
 * @brief  : Handles processing of session modification response received in case of sgw relocation
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_sess_mod_resp_sgw_reloc_handler(void *arg1, void *arg2);

/**
 * @brief  : Process pfcp session modification request for handover scenario
 * @param  : mbr, holds information in session modification request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_sess_mod_req_handover(mod_bearer_req_t *mbr);


uint8_t
process_pfcp_sess_mod_resp_handover(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx);

/**
 * @brief  : Process modify bearer response received on s5s8 interface at sgwc
 * @param  : mb_rsp, buffer containing response data
 * @param  : gtpv2c_tx, gtpv2c message transmission buffer to response message
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_sgwc_s5s8_modify_bearer_response(mod_bearer_rsp_t *mb_rsp, gtpv2c_header_t *gtpv2c_tx);

/**
 * @brief  : Handles processing of modify bearer response in handover scenario
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_mbr_resp_handover_handler(void *arg1, void *arg2);

/**
 * @brief  : Process pfcp session modification request for handover scenario
 * @param  : mbr, holds information in session modification request
 * @return : Returns 0 in case of success , -1 otherwise
 */

int
send_pfcp_sess_mod_req_handover(pdn_connection_t *pdn, eps_bearer_t *bearer,
			mod_bearer_req_t *mbr);


void 
send_pfcp_sess_mod_req_handover_timeout(void *data);

#ifdef __cplusplus
}
#endif
#endif
