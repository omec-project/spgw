// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

#include "cp_stats.h"
#include "cp.h"
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "gw_adapter.h"


struct cp_stats_t cp_stats;

/**
 * @brief  : callback used to display rx packets per second
 * @param  : void
 * @return : number of packets received by the control plane s11 interface
 */
static uint64_t
rx_pkts_per_sec(void)
{
	uint64_t ret = cp_stats.rx - cp_stats.rx_last;

	cp_stats.rx_last = cp_stats.rx;
	return ret;
}

/**
 * @brief  : callback used to display tx packets per second
 * @param  : void
 * @return : number of packets transmitted by the control plane s11 interface
 */
static uint64_t
tx_pkts_per_sec(void)
{
	uint64_t ret = cp_stats.tx - cp_stats.tx_last;

	cp_stats.tx_last = cp_stats.tx;
	return ret;
}

/**
 * @brief  : callback used to display control plane uptime
 * @param  : void
 * @return : control plane uptime in seconds
 */
static uint64_t
stats_time(void)
{
	uint64_t ret = cp_stats.time;

	cp_stats.time++;
	oss_reset_time++;
	return ret;
}

/**
 * @brief  : statistics entry used to simplify statistics by providing a common
 *           interface for statistic values or calculations and their names
 */
struct stat_entry_t {
	enum {VALUE, LAMBDA} type;
	uint8_t spacing;	/** variable length stat entry specifier */
	union {
		uint64_t *value;	/** value used by stat */
		uint64_t (*lambda)(void);	/** stat callback function */
	};
	const char *top;	/** top collumn stat name string */
	const char *bottom;	/** bottom collumn stat name string */
};

#define DEFINE_VALUE_STAT(spacing, function, top, bottom) \
	{VALUE, spacing, {.value = function}, top, bottom}
#define DEFINE_LAMBDA_STAT(spacing, function, top, bottom) \
	{LAMBDA, spacing, {.lambda = function}, top, bottom}
#define PRINT_STAT_ENTRY_HEADER(entry_index, header) \
		printf("%*s ",\
			stat_entries[entry_index].spacing, \
			stat_entries[entry_index].header)

/**
 * @brief  : statistic entry definitions
 */
struct stat_entry_t stat_entries[] = {
	DEFINE_LAMBDA_STAT(5, stats_time, "", "time"),
	DEFINE_VALUE_STAT(8, &cp_stats.rx, "rx", "pkts"),
	DEFINE_VALUE_STAT(8, &cp_stats.tx, "tx", "pkts"),
	DEFINE_LAMBDA_STAT(8, rx_pkts_per_sec, "rx pkts", "/sec"),
	DEFINE_LAMBDA_STAT(8, tx_pkts_per_sec, "tx pkts", "/sec"),
	DEFINE_VALUE_STAT(8, &cp_stats.create_session, "create", "session"),
	DEFINE_VALUE_STAT(8, &cp_stats.modify_bearer, "modify", "bearer"),
	DEFINE_VALUE_STAT(8, &cp_stats.bearer_resource, "b resrc", "cmd"),
	DEFINE_VALUE_STAT(8, &cp_stats.create_bearer, "create", "bearer"),
	DEFINE_VALUE_STAT(8, &cp_stats.delete_bearer, "delete", "bearer"),
	DEFINE_VALUE_STAT(8, &cp_stats.delete_session, "delete", "session"),
	DEFINE_VALUE_STAT(8, &cp_stats.echo, "", "echo"),
	DEFINE_VALUE_STAT(8, &cp_stats.rel_access_bearer, "rel acc", "bearer"),
	DEFINE_VALUE_STAT(8, &cp_stats.ddn, "",	"ddn"),
	DEFINE_VALUE_STAT(8, &cp_stats.ddn_ack, "ddn", "ack"),
};


/**
 * @brief  : prints out statistics entries
 * @param  : void
 * @return : void
 */
static inline void
print_stat_entries(void) {
	unsigned i;

	if (!(cp_stats.time % 32)) {
		puts("");
		for (i = 0; i < RTE_DIM(stat_entries); ++i)
			PRINT_STAT_ENTRY_HEADER(i, top);
		puts("");
		for (i = 0; i < RTE_DIM(stat_entries); ++i)
			PRINT_STAT_ENTRY_HEADER(i, bottom);
		puts("");
	}

	for (i = 0; i < RTE_DIM(stat_entries); ++i) {
		printf("%*"PRIu64" ", stat_entries[i].spacing,
				(stat_entries[i].type == VALUE) ?
					*stat_entries[i].value :
					(*stat_entries[i].lambda)());
	}
	puts("");
}


int
do_stats(__rte_unused void *ptr)
{
	while (1) {
		print_stat_entries();

		sleep(1);
	}

	return 0;
}

void
reset_cp_stats(void) {
	memset(&cp_stats, 0, sizeof(cp_stats));
}
