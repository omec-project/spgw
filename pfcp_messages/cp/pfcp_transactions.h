// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */

#ifndef __PFCP__TRANSACTIONS_H
#define __PFCP__TRANSACTIONS_H
#include "timer.h"
#include "trans_struct.h"
#include "upf_struct.h"

transData_t *create_transaction(upf_context_t *upf_ctxt, uint8_t *buf, uint16_t buf_len);

bool start_transaction_timer(transData_t *trans, uint32_t timeout_ms, gstimercallback cb);

transData_t *get_pfcp_transaction(void);

void delete_pfcp_transaction(void);

void transaction_timeout_callback(gstimerinfo_t *ti, const void *data_t);
#endif
