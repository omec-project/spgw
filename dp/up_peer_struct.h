/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2019 Sprint
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __UP_PEER_STRUCT_H_
#define __UP_PEER_STRUCT_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <rte_ethdev.h>
#include "timer.h"

/**
 * @brief  : Maintains peer node related information for data plane
 */
typedef struct {
	/** UL || DL || Sx port id */
	uint8_t portId;
	/** In-activity Flag */
	uint8_t activityFlag;
	/** Number of Iteration */
	uint8_t itr;
	/** Iteration Counter */
	uint8_t itr_cnt;
	/** GTP-U response Counter */
	uint32_t rstCnt;
	/** Session Counter */
	uint32_t sess_cnt;
	/** Set of Session IDs */
	uint64_t sess_id[3200];
	/** src ipv4 address */
	uint32_t srcIP;
	/** dst ipv4 address */
	uint32_t dstIP;
	/** Recovery Time */
	uint32_t rcv_time;
	/** src ether address */
	struct ether_addr src_eth_addr;
	/** dst ether address */
	struct ether_addr dst_eth_addr;
	/** Periodic Timer */
	gstimerinfo_t  pt;
	/** Transmit Timer */
	gstimerinfo_t  tt;
	/** Name String */
	const char    *name;
	//struct rte_mbuf *buf;

} peerData_t;

#endif
