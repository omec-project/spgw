// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef __PROC_RAR_H__
#define __PROC_RAR_H__
#include "sm_struct.h"
#include "cp_proc.h"
#include "sm_enum.h"

proc_context_t* alloc_rar_proc(msg_info_t *msg);

void
rar_req_event_handler(void *proc, void *msg_info);

/* Function */
/**
 * @brief  : Handles processing of rar request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_rar_request_handler(void *arg1, void *arg2);

/**
 * @brief  : Parse GX RAR message.
 * @param  : rar holds data from gx rar message
 * @return : Returns 0 on success, -1 otherwise
 */
int8_t
parse_gx_rar_msg(msg_info_t *msg);

#endif
