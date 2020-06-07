/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __CP_APIS_H__
#define __CP_APIS_H__

void
gtpc_timer_retry_send(int fd, peerData *t_tx);

void
pfcp_timer_retry_send(int fd, peerData *t_tx);
#endif

