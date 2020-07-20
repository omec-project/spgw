// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */

#ifndef __PFCP__TRANSACTIONS_H
#define __PFCP__TRANSACTIONS_H
#include "timer.h"
#include "trans_struct.h"
#include "upf_struct.h"

transData_t *start_pfcp_node_timer(void *upf, uint8_t *buf, uint16_t buf_len,timeout_handler_t cb);

transData_t *start_pfcp_session_timer(void *ue, uint8_t *buf, uint16_t buf_len, timeout_handler_t cb);

void stop_transaction_timer(transData_t *data);


void
pfcp_node_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t);

void
pfcp_session_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t);

void 
pfcp_timer_retry_send(int fd, transData_t *t_tx, struct sockaddr_in *peer);

#endif
