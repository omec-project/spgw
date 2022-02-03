// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SM_HAND_H__
#define __SM_HAND_H__ 
#include <stdio.h>
#include "trans_struct.h"
#include "upf_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function */
/**
 * @brief  : Handles processing in case of error
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_error_occured_handler(void *t1, void *t2);


/* Function */
/**
 * @brief  : Handles processing of pfd management request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int pfd_management_handler(void *arg1, void *arg2);

/* Function */

/* Function */
/**
 * @brief  : Handles processing of cca-t message
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int cca_t_msg_handler(void *arg1, void *arg2);

/* Function */
int process_pfcp_sess_mod_resp_dbr_handler(void *data, void *unused_param);

/* Function */
int process_delete_bearer_resp_handler(void *data, void *unused_param);

/* Function */
int process_pfcp_sess_del_resp_dbr_handler(void *data, void *unused_param);

/* Function */
int process_update_bearer_response_handler(void *arg1, void *arg2);

/* Function */
int process_delete_bearer_command_handler(void *arg1, void *arg2);

/* Function */
int del_bearer_cmd_ccau_handler(void *arg1, void *arg2);

/* Function */
int del_bearer_cmd_mbr_resp_handler(void *arg1, void *arg2);

/* Function */
int process_pfcp_sess_mod_resp_ubr_handler(void *arg1, void *arg2);

/* Function */
int cca_u_msg_handler_handover(void *arg1, void *argu2);


int
process_error_occured_handler_new(void *data, void *unused_param);

#ifdef __cplusplus
}
#endif
#endif
