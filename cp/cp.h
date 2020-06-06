/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _CP_H_
#define _CP_H_

#include <pcap.h>
#include <byteswap.h>
#include <rte_version.h>
#include <stdbool.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include "main.h"
#include "gtpv2c.h"

#ifdef USE_REST
#include "../restoration/restoration_timer.h"
#endif /* USE_REST */

#include "gtp_messages.h"

#ifndef PERF_TEST
/** Temp. work around for support debug log level into DP, DPDK version 16.11.4 */
#if (RTE_VER_YEAR >= 16) && (RTE_VER_MONTH >= 11)
#undef RTE_LOG_LEVEL
#define RTE_LOG_LEVEL RTE_LOG_DEBUG
#define RTE_LOG_DP RTE_LOG
#elif (RTE_VER_YEAR >= 18) && (RTE_VER_MONTH >= 02)
#undef RTE_LOG_DP_LEVEL
#define RTE_LOG_DP_LEVEL RTE_LOG_DEBUG
#endif
#else /* Work around for skip LOG statements at compile time in DP, DPDK 16.11.4 and 18.02 */
#if (RTE_VER_YEAR >= 16) && (RTE_VER_MONTH >= 11)
#undef RTE_LOG_LEVEL
#define RTE_LOG_LEVEL RTE_LOG_WARNING
#define RTE_LOG_DP_LEVEL RTE_LOG_LEVEL
#define RTE_LOG_DP RTE_LOG
#elif (RTE_VER_YEAR >= 18) && (RTE_VER_MONTH >= 02)
#undef RTE_LOG_DP_LEVEL
#define RTE_LOG_DP_LEVEL RTE_LOG_WARNING
#endif
#endif /* PERF_TEST */

#define DEFAULT_STATS_PATH  "./logs/"

/**
 * Control-Plane rte logs.
 */
#define RTE_LOGTYPE_CP  RTE_LOGTYPE_USER1

/**
 * @brief  : core identifiers for control plane threads
 */
struct cp_params {
	unsigned stats_core_id;
#ifdef SIMU_CP
	unsigned simu_core_id;
#endif
};
extern pcap_dumper_t *pcap_dumper;
extern pcap_t *pcap_reader;

extern int s11_fd;
extern int s11_pcap_fd;
extern int s5s8_sgwc_fd;
extern int s5s8_pgwc_fd;
extern int pfcp_sgwc_fd ;
extern struct cp_params cp_params;


/**
 * @brief  : initializes data plane by creating and adding default entries to
 *           various tables including session, pcc, metering, etc
 * @param  : No param
 * @return : Returns Nothing
 */
void
initialize_tables_on_dp(void);


#ifdef DELETE_THIS
/**
 * @brief  : To Downlink data notification ack of user.
 * @param  : dp_id, table identifier.
 * @param  : ddn_ack, Downlink data notification ack information
 * @return : - 0 on success
 *           -1 on failure
 */
int
send_ddn_ack(struct dp_id dp_id,
			struct downlink_data_notification ddn_ack);
#endif

/**
 * @brief  : Initialize pfcp interface details
 * @param  : void
 * @return : Void
 */
void
init_pfcp(void);

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
#endif
