// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __CP_APIS_H__
#define __CP_APIS_H__
#include "trans_struct.h"

void
gtpc_timer_retry_send(int fd, peerData_t *t_tx);

void
pfcp_timer_retry_send(int fd, peerData_t *t_tx);

void
pfcp_timer_retry_send_new(int fd, transData_t *t_tx);
#endif

