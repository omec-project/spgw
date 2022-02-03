// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef __PROC_RAR_H__
#define __PROC_RAR_H__
#include "sm_struct.h"
#include "proc_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
proc_context_t* alloc_rar_proc(msg_info_t *msg);

void
rar_req_event_handler(void *proc, void *msg_info);

void
add_rar_child_proc(void *rar_proc, void *child_proc);

void
done_rar_child_proc(void *proc);

void 
send_raa(msg_info_t *msg);

void
proc_rar_failed(msg_info_t *msg, uint8_t cause);

void 
proc_rar_complete(proc_context_t *proc_context);
/* Function */
/**
 * @brief  : Handles processing of rar request
 * @param  : arg1, data contained in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_rar_request_handler(void *arg1);

/**
 * @brief  : Parse GX RAR message.
 * @param  : rar holds data from gx rar message
 * @return : Returns 0 on success, -1 otherwise
 */
int8_t
parse_gx_rar_msg(msg_info_t *msg);

#ifdef __cplusplus
}
#endif
#endif
