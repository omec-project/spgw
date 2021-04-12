// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __IP_POOL_H
#define __IP_POOL_H

#include <netinet/in.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  : Simple ip-pool
 * @param  : ipv4
 *           ip address to be used for a new UE connection
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to
 *           3gpp specified cause error value
 */
uint32_t
acquire_ip(struct in_addr *ipv4);

void
release_ip(struct in_addr ipv4);

bool
reserve_ip_node(struct in_addr host);

bool 
release_ip_node(struct in_addr host);

#ifdef __cplusplus
}
#endif
#endif

