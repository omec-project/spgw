// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef CP_STATS_H
#define CP_STATS_H

#include <stdint.h>
#include <time.h>
#include <rte_common.h>
#define LAST_TIMER_SIZE 80
/**
 * @file
 *
 * Control Plane statistic declarations
 */

/**
 * @brief  : counters used to display statistics on the control plane
 */
struct cp_stats_t {

	uint64_t time;
	clock_t  execution_time;
	clock_t  reset_time;

	uint64_t create_session;
	uint64_t delete_session;
	uint64_t modify_bearer;
	uint64_t rel_access_bearer;
	uint64_t bearer_resource;
	uint64_t create_bearer;
	uint64_t delete_bearer;
	uint64_t ddn;
	uint64_t ddn_ack;
	uint64_t echo;
	uint64_t rx;
	uint64_t tx;
	uint64_t rx_last;
	uint64_t tx_last;

	char stat_timestamp[LAST_TIMER_SIZE];

};

extern struct cp_stats_t cp_stats;

extern int s11logger;
extern int s5s8logger;
extern int sxlogger;
extern int gxlogger;
extern int apilogger;
extern int epclogger;

/**
 * @brief  : Prints control plane signaling message statistics
 * @param  : Currently not being used
 * @return : Never returns/value ignored
 */
int
do_stats(__rte_unused void *ptr);

/**
 * @brief  : clears the control plane statistic counters
 * @param  : No param
 * @return : Returns nothing
 */
void
reset_cp_stats(void);

#endif
