// Copyright (c) 2019 Sprint
// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <time.h>
#include "csid_struct.h"
#include "cp_log.h"
#include "cp_init.h"
#include "cp_main.h"

#define NUM_OF_TABLES 7
#define NUM_OF_NODE 15
#define MAX_HASH_SIZE (1 << 12) /* Total entry : 4095 */

extern uint16_t local_csid;
/**
 * Add csid entry in csid hash table.
 *
 * @param struct peer_node_info csid_key
 * key.
 * @param csid
 * return 0 or 1.
 *
 */
int8_t
add_csid_entry(csid_key *key, uint16_t csid)
{
    assert(0);
}

/**
 * Compare the peer node information with exsting peer node entry.
 *
 * @param struct peer_node_info peer1
 * key.
 * @param struct peer_node_info peer
 * return 0 or -1.
 *
 */
int8_t
compare_peer_info(csid_key *peer1, csid_key *peer2)
{
	if ((peer1 == NULL) || (peer2 == NULL))
		return -1;

	/* Compare peer nodes information */
	if ((peer1->mme_ip == peer2->mme_ip) &&
			(peer1->sgwc_ip== peer2->sgwc_ip) &&
			(peer1->sgwu_ip == peer2->sgwu_ip) &&
			(peer1->pgwc_ip == peer2->pgwc_ip) &&
			(peer1->pgwu_ip == peer2->pgwu_ip)
			&& (peer1->enodeb_id == peer2->enodeb_id)
	   ) {
		LOG_MSG(LOG_DEBUG, "Peer node exsting entry is matched");
		return 0;
	}
	LOG_MSG(LOG_DEBUG, "Peer node exsting entry is not matched");
	return -1;

}
/**
 * Update csid key associated peer node with csid in csid hash table.
 *
 * @param struct peer_node_info csid_key
 * key.
 * @param struct peer_node_info csid_key
 * return csid or -1.
 *
 */
int16_t
update_csid_entry(csid_key *old_key, csid_key *new_key)
{
    assert(0);
}

/**
 * Get csid entry from csid hash table.
 *
 * @param struct peer_node_info csid_key
 * key.
 * return csid or -1
 *
 */
int16_t
get_csid_entry(csid_key *key)
{
    assert(0);
    return 0;
}
/**
 * Delete csid entry from csid hash table.
 *
 * @param struct peer_node_info csid_key
 * key.
 * return 0 or 1.
 *
 */
int8_t
del_csid_entry(csid_key *key)
{
	LOG_MSG(LOG_DEBUG, "Peer node CSID entry deleted %p ", key);
    assert(0);
	return 0;
}
/**
 * Add peer node csids entry in peer node csids hash table.
 *
 * @param local_csid
 * key.
 * @param struct fq_csid_info fq_csids
 * return 0 or 1.
 *
 */
int8_t
add_peer_csids_entry(uint16_t csid, fq_csids *csids)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "CSID entry added for CSID: %u, %p ", csid, csids);
	return 0;
}

/**
 * Get peer node csids entry from peer node csids hash table.
 *
 * @param local_csid
 * key.
 * return fq_csids or NULL
 *
 */
fq_csids*
get_peer_csids_entry(uint16_t csid)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "Entry found for CSID: %u", csid);
	return NULL;

}

/**
 * Delete peer node csid entry from peer node csid hash table.
 *
 * @param csid
 * key.
 * return 0 or 1.
 *
 */
int8_t
del_peer_csids_entry(uint16_t csid)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "Entry deleted for CSID:%u", csid);
	return 0;
}

			/********[ seids_by_csid_hash ]*********/
/**
 * Add session ids entry in sess csid hash table.
 *
 * @param csid
 * key.
 * @param struct sess_csid_info sess_csid
 * return 0 or 1.
 *
 */
int8_t
add_sess_csid_entry(uint16_t csid, sess_csid *seids)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "Session IDs entry added for CSID:%u  %p", csid, seids);
	return 0;
}

/**
 * Get session ids entry from sess csid hash table.
 *
 * @param local_csid
 * key.
 * return sess_csid or NULL
 *
 */
sess_csid*
get_sess_csid_entry(uint16_t csid)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "Entry Found for CSID:%u", csid);
	return NULL;

}

/**
 * Delete session ids entry from sess csid hash table.
 *
 * @param local_csid
 * key.
 * return 0 or 1.
 *
 */
int8_t
del_sess_csid_entry(uint16_t csid)
{
    assert(0);
	LOG_MSG(LOG_DEBUG, "Sessions IDs Entry deleted for CSID:%u", csid);
	return 0;
}

/**
 *Init the hash tables for FQ-CSIDs */
int8_t
init_fqcsid_hash_tables(void)
{
	return 0;
}
