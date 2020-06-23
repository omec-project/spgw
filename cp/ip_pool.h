
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __IP_POOL_H
#define __IP_POOL_H

#include <netinet/in.h>
#include <stdbool.h>

struct ip_table
{
  struct ip_table *octet[256];
  char *ue_address; // address 
  bool used ; 
};


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

struct ip_table *create_ue_pool(struct in_addr network, struct in_addr mask);

void 
add_ipaddr_in_pool(struct ip_table *search_tree, struct in_addr host);

bool
reserve_ip_node(struct ip_table *search_tree , struct in_addr host);

bool 
release_ip_node(struct ip_table *search_tree , struct in_addr host);
#endif

