// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0


#ifndef __CP_PEER_STRUCT_H
#define __CP_PEER_STRUCT_H 

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "cp_timer.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Maintains peer node related information for control plane
 */
typedef struct peerData {
	/** S11 || S5/S8 || Sx port id */
	uint8_t portId;
	/** In-activity Flag */
	uint8_t activityFlag;
	/** Number of Iteration */
	uint8_t itr;
	/** Iteration Counter */
	uint8_t itr_cnt;
	/** Dst Addr */
	uint32_t dstIP;
	/* Dst port */
	uint16_t dstPort;
	/** Recovery Time */
	uint32_t rcv_time;
	/** Periodic Timer */
	gstimerinfo_t  pt;
	/** Transmit Timer */
	gstimerinfo_t  tt;
	const char    *name;
	/* Teid */
	uint32_t teid;
	/*ebi ID */
	uint8_t ebi_index;
	uint16_t buf_len;
	uint8_t buf[1024];
} peerData_t;

struct peer_data {
    gstimerinfo_t *ti;
    struct sockaddr_in dest_addr;
};


#ifdef __cplusplus
}
#endif

#endif
