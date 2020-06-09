// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _UP_ETHER_H_
#define _UP_ETHER_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of dataplane ethernet constructor.
 */
#include <stdint.h>
#include <rte_mbuf.h>
#include <rte_ether.h>

#include "up_main.h"
#include "pfcp_up_struct.h"
#define ETH_TYPE_IPv4 0x0800

/**
 * @brief  : Function to return pointer to L2 headers.
 * @param  : m, mbuf pointer
 * @return : Returns address to l2 hdr
 */
static inline struct ether_hdr *get_mtoeth(struct rte_mbuf *m)
{
	return (struct ether_hdr *)rte_pktmbuf_mtod(m, unsigned char *);
}

/**
 * @brief  : Function to construct L2 headers.
 * @param  : m, mbuf pointer
 * @param  : portid, port id
 * @param  : pdr, pointer to pdr session info
 * @return : Returns 0 in case of success , -1(ARP lookup fail) otherwise
 */
int construct_ether_hdr(struct rte_mbuf *m, uint8_t portid,
		pdr_info_t **pdr);

#endif				/* _ETHER_H_ */
