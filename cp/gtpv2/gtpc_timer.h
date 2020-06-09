// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __GTPC_TIMER_H
#define __GTPC_TIMER_H


/**
 * @brief  : Returns peer data struct address and fill data.
 * @param  : iface, source interface type
 * @param  : peer_addr, peer node address
 * @param  : buf, holds timer data
 * @param  : buf_len, total length of data
 * @param  : itr, request_tries value in pfcp config
 * @param  : teid, teid value
 * @return : Returns pointer to filled timer entry structure
 */
peerData *
gtpc_fill_timer_entry_data(enum source_interface iface, struct sockaddr_in *peer_addr,
	uint8_t *buf, uint16_t buf_len, uint8_t itr, uint32_t teid,  uint8_t ebi_index);

/**
 * @brief  : add timer entry
 * @param  : conn_data, peer node connection information
 * @param  : timeout_ms, timeout
 * @param  : cb, timer callback
 * @return : Returns true or false
 */
bool
gtpc_add_timer_entry(peerData *conn_data, uint32_t timeout_ms,
				gstimercallback cb);

/**
 * @brief  : delete time entry
 * @param  : teid, teid value
 * @return : Returns nothing
 */
void
gtpc_delete_timer_entry(uint32_t teid);

/**
 * @brief  : timer callback
 * @param  : ti, timer information
 * @param  : data_t, Peer node connection information
 * @return : Returns nothing
 */
void
gtpc_peer_timer_callback(gstimerinfo_t *ti, const void *data_t);

/**
 * @brief  : Fills and adds timer entry, and starts periodic timer for gtpv2c messages
 * @param  : teid, teid value
 * @param  : peer_addr, peer node address
 * @param  : buf, holds timer data
 * @param  : buf_len, total length of data
 * @return : Returns nothing
 */
void
add_gtpv2c_if_timer_entry(uint32_t teid, struct sockaddr_in *peer_addr,
	uint8_t *buf, uint16_t buf_len, uint8_t ebi_index, enum source_interface iface);

void
delete_gtpv2c_if_timer_entry(uint32_t teid);

#endif
