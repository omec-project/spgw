// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _INTERFACE_DP_H_
#define _INTERFACE_DP_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of CP/DP module constructor and communication interface type.
 */
#include <stdint.h>
#include <inttypes.h>

#include <rte_hash.h>

#include "vepc_cp_dp_api.h"

/**
 * @brief  : Process PFCP message.
 * @param  : buf_rx
 *           buf - message buffer.
 * @param  : bytes_rx
 *           received message buffer size
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_msg(uint8_t *buf_rx,
		struct sockaddr_in *peer_addr);

/**
 * @brief  : Initialize iface message passing
 *           This function is not thread safe and should only be called once by DP.
 * @param  : No param
 * @return : Returns nothing
 */
void iface_module_constructor(void);

/**
 * @brief  : Functino to handle signals.
 * @param  : signo
 *           signal number signal to be handled
 * @return : Returns nothing
 */
void sig_handler(int signo);

#endif /* _INTERFACE_H_ */
