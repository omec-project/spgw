// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only


#ifndef _SM_STRUCT_API_H
#define _SM_STRUCT_API_H
#include "sm_struct.h"

/**
 * @brief  : Create a session hash table to maintain the session information.
 * @param  : No param
 * @return : Returns nothing
 */
void
init_transaction_hash(void);

/**
 * @brief  : Add session entry in session table.
 * @param  : sess_id, session id
 * @param  : resp, structure to store session info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
add_sess_entry(uint64_t sess_id, struct resp_info *resp);

/**
 * @brief  : Retrive session entry from session table.
 * @param  : sess_id, session id
 * @param  : resp, structure to store session info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_sess_entry(uint64_t sess_id, struct resp_info **resp);



/**
 * @brief  : Delete session entry from session table.
 * @param  : sess_id, session id
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
del_sess_entry(uint64_t sess_id);

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
 * Retrive Bearer entry from Bearer table.
 */
int8_t
get_bearer_by_teid(uint32_t teid_key, eps_bearer_t **bearer);

/**
 * Retrive ue context entry from Bearer table,using sgwc s5s8 teid.
 */
int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context_t **context);

/**
 * @brief  : Retrive UE Context entry from UE Context table.
 * @param  : teid_key, key to search context
 * @param  : context, structure to store retrived context
 * @return : Returns 0 in case of success , -1 otherwise
 */
int8_t
get_ue_context(uint32_t teid_key, ue_context_t **context);

/* This function use only in clean up while error */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context_t **context);

/**
 * @brief  : Retrive PDN entry from PDN table.
 * @param  : teid_key, key for search
 * @param  : pdn, structure to store retrived pdn
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_pdn(uint32_t teid_key, pdn_connection_t **pdn);

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
 * @brief  : Update UE proc in UE Context.
 * @param  : teid_key, key for search
 * @param  : proc, procedure
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
update_ue_proc(uint32_t teid_key, uint8_t proc, uint8_t ebi_index);

/**
 * @brief  : Retrive UE Context.
 * @param  : teid_key, key for search
 * @param  : context, structure to store retrived ue context
 * @return : Returns 0 in case of success , -1 otherwise
 */
int8_t
get_ue_context(uint32_t teid_key, ue_context_t **context);

/**
 * @brief  : Update Procedure according to indication flags
 * @param  : msg, message data
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_procedure(msg_info *msg);

/**
 * @brief  : Find Pending CSR Procedure according to indication flags
 * @param  : csr, csr data
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_csr_proc(create_sess_req_t *csr);



#endif
