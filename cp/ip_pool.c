/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include "stdint.h"
#include "ip_pool.h"
#include "gtpv2c_ie.h"
#include "ue.h"
#include "cp_config_new.h"
#include "clogger.h"

/* ajay - only 1 ip pool is supported. Need better pool management  */
#define GET_UE_IP(ue_index) \
			(((cp_config->ip_pool_ip.s_addr | (~cp_config->ip_pool_mask.s_addr)) \
			  - htonl(ue_index)) - 0x01000000)


#define LDB_ENTRIES_DEFAULT (1024 * 1024 * 4)
/* TODO : Prio2 . Scaling needs change in this area. */
uint32_t
acquire_ip(struct in_addr *ipv4)
{
	static uint32_t next_ip_index;
	if (unlikely(next_ip_index == LDB_ENTRIES_DEFAULT)) {
		clLog(clSystemLog, eCLSeverityCritical, "IP Pool depleted\n");
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
	}
	ipv4->s_addr = GET_UE_IP(next_ip_index++);
	return 0;
}

