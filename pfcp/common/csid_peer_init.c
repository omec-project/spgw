// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <time.h>
#include "csid_struct.h"
#include "cp_log.h"
#include "cp_init.h"
#include "cp_main.h"

/**
 * Add local csid entry by peer csid in peer csid hash table.
 *
 * @param csid_t peer_csid_key
 * key.
 * @param csid_t local_csid
 * @param ifce S11/Sx/S5S8
 * return 0 or 1.
 *
 */
int8_t
add_peer_csid_entry(uint16_t *key, csid_t *csid, uint8_t iface)
{
	LOG_MSG(LOG_DEBUG, "CSID entry added for csid: %p %p %d ", key, csid, iface);
	return 0;
}

/**
 * Get local csid entry by peer csid from csid hash table.
 *
 * @param csid_t csid_key
 * key.
 * @param iface
 * return csid or -1
 *
 */
csid_t*
get_peer_csid_entry(uint16_t *key, uint8_t iface)
{
	LOG_MSG(LOG_DEBUG, "CSID : %p iface = %d ", key, iface);
	return NULL;

}

/**
 * Delete local csid entry by peer csid from csid hash table.
 *
 * @param csid_t csid_key
 * key.
 * @param iface
 * return 0 or 1.
 *
 */
int8_t
del_peer_csid_entry(uint16_t *key, uint8_t iface)
{
	LOG_MSG(LOG_DEBUG, "Peer node CSID entry deleted %p %d", key, iface);
	return 0;
}

/**
 * Add peer node csids entry by peer node address in peer node csids hash table.
 *
 * @param node address
 * key.
 * @param fqcsid_t csids
 * return 0 or 1.
 *
 */
int8_t
add_peer_addr_csids_entry(uint32_t node_addr, fqcsid_t *csids)
{
	LOG_MSG(LOG_DEBUG, "CSID entry added for node address: %u %p ", node_addr, csids);
	return 0;
}

/**
 * Get peer node csids entry by peer node addr from peer node csids hash table.
 *
 * @param node address
 * key.
 * @param is_mod
 * return fqcsid_t or NULL
 *
 */
fqcsid_t*
get_peer_addr_csids_entry(uint32_t node_addr, uint8_t is_mod)
{
	LOG_MSG(LOG_DEBUG, "Entry found for Node address: %u %d ",node_addr, is_mod);
	return NULL;
}

/**
 * Delete peer node csid entry by peer node addr from peer node csid hash table.
 *
 * @param node_address
 * key.
 * return 0 or 1.
 *
 */
int8_t
del_peer_addr_csids_entry(uint32_t node_addr)
{
	LOG_MSG(LOG_DEBUG, "Entry deleted for node addr: %u",node_addr);
	return 0;
}
