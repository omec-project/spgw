// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */

#ifndef __PFCP__TRANSACTIONS_H
#define __PFCP__TRANSACTIONS_H
#include "stdbool.h"
#include "cp_timer.h"
#include "trans_struct.h"
#include "upf_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
transData_t *start_response_wait_timer(void *ue, uint8_t *buf, uint16_t buf_len, timeout_handler_t cb);

transData_t *restart_response_wait_timer(transData_t *trans);

void stop_transaction_timer(transData_t *data);

bool is_transaction_timer_started(transData_t *data);


void
pfcp_node_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t);

void
transaction_retry_callback(gstimerinfo_t *ti, const void *data_t);

void 
pfcp_timer_retry_send(int fd, transData_t *t_tx, struct sockaddr_in *peer);

void 
cleanup_gtpc_trans(transData_t *gtpc_trans);

void 
cleanup_pfcp_trans(transData_t *pfcp_trans);

void 
delayed_free(transData_t *trans);

#ifdef __cplusplus
}
#endif
#endif
