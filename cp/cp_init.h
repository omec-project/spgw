// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _CP_H_
#define _CP_H_

#include <pcap.h>
#include <byteswap.h>
#include <rte_version.h>
#include <stdbool.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include "cp_main.h"

#ifdef USE_REST
#include "timer.h"
#endif /* USE_REST */


/**
 * @brief  : core identifiers for control plane threads
 */
struct cp_params {
	unsigned stats_core_id;
#ifdef SIMU_CP
	unsigned simu_core_id;
#endif
};

extern int s11_pcap_fd;
extern int s5s8_sgwc_fd;
extern int s5s8_pgwc_fd;
extern int pfcp_sgwc_fd ;
extern struct cp_params cp_params;

/**
 * @brief  : Initializes Control Plane data structures, packet filters, and calls for the
 *           Data Plane to create required tables
 * @param  : void
 * @return : Void
 */
void
init_cp(void);

/**
 * @brief  : Initialize dp rule table
 * @param  : void
 * @return : Void
 */
void
init_dp_rule_tables(void);

/**
 * @brief  : Updates restart counter Value
 * @param  : No param
 * @return : Returns nothing
 */
uint8_t
update_rstCnt(void);

/**
 * @brief  : Add entry for recovery time into heartbeat recovery file
 * @param  : recov_time, recovery time
 * @return : Returns nothing
 */
void recovery_time_into_file(uint32_t recov_time);


#endif
