// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef PROC_H
#define PROC_H

#include "proc_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

void 
start_procedure(proc_context_t *proc);

void 
end_procedure(proc_context_t *proc);

proc_context_t*
get_first_procedure(void *ue_context);

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


#ifdef __cplusplus
}
#endif
#endif /* PROC_H */
