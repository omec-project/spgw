// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PROC_BEARER_DELETE
#define __PROC_BEARER_DELETE
#include <stdio.h>
#include "trans_struct.h"
#include "upf_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Handles processing of modification response received in case of delete request
 * @param  : arg1, data contained in message
 * @param  : arg2, optional parameter
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_mod_resp_delete_handler(void *arg1, void *arg2);

/* Function */
int process_delete_bearer_response_handler(void *arg1, void *arg2);


#ifdef __cplusplus
}
#endif
#endif // __PROC_BEARER_DELETE

