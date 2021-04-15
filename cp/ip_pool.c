// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <sys/types.h>
#include <sys/socket.h>
#include "stdint.h"
#include "assert.h"
#include "ip_pool.h"
#include "gtpv2_ie.h"
#include "ue.h"
#include "spgw_config_struct.h"
#include "util.h"
#include "cp_log.h"
#include "spgw_cpp_wrapper.h"

uint32_t
acquire_ip(struct in_addr *ipv4)
{
//#define TESTPOOL
#ifdef TESTPOOL
    // Test loop to allocate address and free it ..
    int i=0;
  start:
    i++;
#endif
    uint32_t addr = acquire_ip_cpp("ueIpPool");
    if(addr == 0) {
		LOG_MSG(LOG_ERROR, "IP Pool depleted");
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
    }
    ipv4->s_addr = htonl(addr); // give back in network order 
    LOG_MSG(LOG_INFO, "IP address assigned to call %s ", inet_ntoa(*ipv4));
#ifdef TESTPOOL
    if(i < 10) {
        release_ip(,"ueIpPool",*ipv4);
        goto start;
    }
#endif
    return 0;
}

void
release_ip(struct in_addr ipv4)
{
    ipv4.s_addr = htonl(ipv4.s_addr); // print
    LOG_MSG(LOG_INFO, "IP address released %s ", inet_ntoa(ipv4));
    ipv4.s_addr = htonl(ipv4.s_addr); // release api needs input in host order 
    release_ip_cpp("ueIpPool", ipv4);
}

bool
reserve_ip_node(struct in_addr host)
{
    return reserve_static_ip_cpp("staticUeIpPool", host);
}

bool 
release_ip_node(struct in_addr host)
{
    return release_static_ip_cpp("staticUeIpPool", host);
}
