// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef PACKET_FILTERS_H
#define PACKET_FILTERS_H

/**
 * @file
 *
 * Contains functions to initialize, manage, and install packet filters internal
 * to the Control Plane as well as calls to forward the installed packet filters
 * to the Data Plane.
 */

#include "ue.h"

#ifdef __cplusplus
extern "C" {
#endif
#define FIRST_FILTER_ID 1

extern uint16_t ulambr_idx;
extern uint16_t dlambr_idx;

/**
 * @brief  : Maintains packet filter information
 */
typedef struct pkt_fltr {
	uint8_t direction;
	uint8_t remote_ip_mask;
	uint8_t proto;
	uint8_t proto_mask;
	uint16_t remote_port_low;
	uint16_t remote_port_high;
	struct in_addr local_ip_addr;
	uint16_t local_port_low;
	uint16_t local_port_high;
	uint8_t local_ip_mask;
	struct in_addr remote_ip_addr;
} pkt_fltr_t;

/**
 * @brief  : Maintains packet filter information along with uplink and
 *           downlink info
 */
typedef struct packet_filter {
	pkt_fltr_t pkt_fltr;
	uint16_t ul_mtr_idx;
	uint16_t dl_mtr_idx;
} packet_filter_t;


/**
 * @brief  : Adds packet filter entry
 * @param  : index, index of array where packet filter needs to be added
 * @return : Returns nothing
 */
void
push_packet_filter(uint16_t index);

/**
 * @brief  : Adds sdf rule entry
 * @param  : index, index of array where sdf rule needs to be added
 * @return : Returns nothing
 */
void
push_sdf_rules(uint16_t index);

/**
 * @brief  : Installs a packet filter in the CP & DP.
 * @param  : new_packet_filter
 *           A packet filter yet to be installed
 * @return : - >= 0 - on success - indicates index of packet filter
 *           - < 0 - on error
 */
int
install_packet_filter(const packet_filter_t *new_packet_filter);

/**
 * @brief  : Returns the packet filter index.
 * @param  : pf, Packet filter
 * @return : Packet filter index matching packet filter 'pf'
 */
int
get_packet_filter_id(const pkt_fltr_t *pf);

/**
 * @brief  : Clears the packet filter at '*pf' to accept all packets.
 * @param  : pf, The packet filter to reset
 * @return : Returns nothing
 */
void
reset_packet_filter(pkt_fltr_t *pf);

/**
 * @brief  : Returns direction of packet filter (uplink and/or downlink).
 * @param  : index
 *           Packet filter index
 * @return : Direction as defined as tft packet filter direction in 3gpp 24.008
 *           table 10.5.162, one of:
 *           - TFT_DIRECTION_BIDIRECTIONAL
 *           - TFT_DIRECTION_UPLINK_ONLY
 *           - TFT_DIRECTION_DOWNLINK_ONLY
 */
uint8_t
get_packet_filter_direction(uint16_t index);

/**
 * @brief  : Returns the packet filter given it's index.
 * @param  : index, Index of packet filter
 * @return : Packet filter at index
 */
packet_filter_t *
get_packet_filter(uint16_t index);

/**
 * @brief  : Packet filter initialization function. Reads static file and populates
 *           packet filters accordingly.
 * @param  : No Param
 * @return : Returns nothing
 */
void
init_packet_filters(void);

#ifdef __cplusplus
}
#endif
#endif
