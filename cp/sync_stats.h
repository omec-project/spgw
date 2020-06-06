/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __SYNC_STATS_H
#define __SYNC_STATS_H
#ifdef SYNC_STATS
#include <time.h>
#define STATS_HASH_SIZE     (1 << 21)
#define ACK       1
#define RESPONSE  2


typedef long long int _timer_t;

/**
 * @brief  : statstics struct of control plane
 */
struct sync_stats {
	uint64_t op_id;
	uint64_t session_id;
	uint64_t req_init_time;
	uint64_t ack_rcv_time;
	uint64_t resp_recv_time;
	uint64_t req_resp_diff;
	uint8_t type;
};

extern struct sync_stats stats_info;
extern _timer_t _init_time;
struct rte_hash *stats_hash;
extern uint64_t entries;
extern uint64_t op_id;

/* ================================================================================= */
/**
 * @file
 * This file contains function prototypes of cp request and response
 * statstics with sync way.
 */

/**
 * @brief  : Open Statstics record file.
 * @param  : No param
 * @return : Returns nothing
 */
void
stats_init(void);

/**
 * @brief  : Maintain stats in hash table.
 * @param  : sync_stats, sync_stats information
 * @return : Returns nothing
 */
void
add_stats_entry(struct sync_stats *stats);

/**
 * @brief  : Update the resp and ack time in hash table.
 * @param  : key, key for lookup entry in hash table
 * @param  : type, Update ack_recv_time/resp_recv_time
 * @return : Returns nothing
 */
void
update_stats_entry(uint64_t key, uint8_t type);

/**
 * @brief  : Retrive entries from stats hash table
 * @param  : void
 * @return : Void
 */
void
retrive_stats_entry(void);

/**
 * @brief  : Export stats reports to file.
 * @param  : sync_stats, sync_stats information
 * @return : Void
 */
void
export_stats_report(struct sync_stats stats_info);

/**
 * @brief  : Close current stats file and redirects any remaining output to stderr
 * @param  : void
 * @return : Void
 */
void
close_stats(void);
/**
 * @brief  : Initialize statistics hash table
 * @param  : void
 * @return : Void
 */
void
init_stats_hash(void);

#endif /* SYNC_STATS */
#endif
