// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef _SM_STRUCT_API_H
#define _SM_STRUCT_API_H
#include "sm_struct.h"
#include "upf_struct.h"
#include "ue.h"

/**
 * @brief  : Update UE state in UE Context.
 * @param  : teid_key, key to search context
 * @param  : state, new state to be updated
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
update_ue_state(uint32_t teid_key, uint8_t state, uint8_t ebi_index);

/**
 * @brief  : Retrive UE state from UE Context.
 * @param  : teid_key, key for search
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_ue_state(uint32_t teid_key ,uint8_t ebi_index);

/**
 * Retrive ue context entry from Bearer table,using sgwc s5s8 teid.
 */
int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context_t **context);

/* This function use only in clean up while error */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context_t **context);

/**
 * @brief  : Get proc name from enum
 * @param  : value , enum value of procedure
 * @return : Returns procedure name
 */
const char * get_proc_string(int value);

/**
 * @brief  : Get state name from enum
 * @param  : value , enum value of state
 * @return : Returns state name
 */
const char * get_state_string(int value);

/**
 * @brief  : Get event name from enum
 * @param  : value , enum value of event
 * @return : Returns event name
 */
const char * get_event_string(int value);

/**
 * @brief  : Update Procedure according to indication flags
 * @param  : msg, message data
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_procedure(msg_info_t *msg);

void
start_upf_procedure(proc_context_t *proc, msg_info_t *msg);

upf_context_t*
get_upf_context(user_plane_profile_t *upf_profile); 

struct in_addr 
native_linux_name_resolve(const char *name);
#endif
