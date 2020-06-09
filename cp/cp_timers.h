// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only


#ifndef __CP_RESTORATION_PEER__H
#define __CP_RESTORATION_PEER__H

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


#endif
