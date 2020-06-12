// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only
#ifndef __UP_RESTORATION_PEER__H
#define __UP_RESTORATION_PEER__H

#include "up_peer_struct.h"

uint8_t process_response(uint32_t dstIp);
/**
 * @brief  : Intialize peer node information
 * @param  : md, Peer node information
 * @param  : name, Peer node name
 * @param  : t1ms, periodic timer interval
 * @param  : t2ms, transmit timer interval
 * @return : Returns true in case of success , false otherwise
 */
bool initpeerData( peerData_t *md, const char *name, int t1ms, int t2ms );

/**
 * @brief  : Delete entry from connection table
 * @param  : ipAddr, key to search entry to be deleted
 * @return : Returns nothing
 */
void del_entry_from_hash(uint32_t ipAddr);

/**
 * @brief  : Add node entry
 * @param  : dstIp, Ip address to be added
 * @param  : sess_id, session id
 * @param  : portId, port number
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
add_node_conn_entry(uint32_t dstIp, uint64_t sess_id, uint8_t portId);

#endif
