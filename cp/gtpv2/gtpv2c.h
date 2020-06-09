// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef GTPV2C_H
#define GTPV2C_H

/**
 * @file
 *
 * GTPv2C definitions and helper macros.
 *
 * GTP Message type definition and GTP header definition according to 3GPP
 * TS 29.274; as well as IE parsing helper functions/macros, and message
 * processing function declarations.
 *
 */

#include "ue.h"
#include "gtpv2c_ie.h"

#include <stddef.h>
#include <arpa/inet.h>
#include "../libgtpv2c/include/gtp_messages.h"
#define GTPC_UDP_PORT                                        (2123)
#define MAX_GTPV2C_UDP_LEN                                   (4096)


#pragma pack()

/* These IE functions/macros are 'safe' in that the ie's returned, if any, fall
 * within the memory range limit specified by either the gtpv2c header or
 * grouped ie length values */

extern struct in_addr s11_mme_ip;
extern struct sockaddr_in s11_mme_sockaddr;

extern struct in_addr s11_sgw_ip;
extern in_port_t s11_port;
extern struct sockaddr_in s11_sgw_sockaddr;
extern uint8_t s11_rx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t tx_buf[MAX_GTPV2C_UDP_LEN];

#ifdef USE_REST
//VS: ECHO BUFFERS
extern uint8_t echo_tx_buf[MAX_GTPV2C_UDP_LEN];
#endif /* USE_REST */

extern struct in_addr s5s8_sgwc_ip;
extern in_port_t s5s8_sgwc_port;
extern struct sockaddr_in s5s8_sgwc_sockaddr;

extern struct in_addr s5s8_pgwc_ip;
extern in_port_t s5s8_pgwc_port;
extern struct sockaddr_in s5s8_pgwc_sockaddr;
extern uint8_t pfcp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];

extern struct in_addr s1u_sgw_ip;
extern struct in_addr s5s8_sgwu_ip;
extern struct in_addr s5s8_pgwu_ip;

#endif /* GTPV2C_H */
