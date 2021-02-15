// Copyright (c) 2019 Sprint
// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <time.h>
#include <rte_hash_crc.h>

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
	int ret = 0;
	uint16_t *tmp = NULL;

	/* Lookup for CSID entry. */
	ret = rte_hash_lookup_data(csid_by_peer_node_hash,
				key, (void **)&tmp);

	if ( ret < 0) {
		tmp = rte_zmalloc_socket(NULL, sizeof(uint16_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (tmp == NULL) {
			LOG_MSG(LOG_ERROR, "Failed to allocate the memory for csid");
		}
		*tmp = csid;

		/* CSID Entry add if not present */
		ret = rte_hash_add_key_data(csid_by_peer_node_hash,
						key, tmp);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add entry for csid : %u"
					"\n\tError= %s", *tmp,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		*tmp = csid;
	}

	LOG_MSG(LOG_DEBUG, " CSID entry added for csid:%u", *tmp);
	return 0;
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
	int ret = 0;
	uint16_t *csid = NULL;

	/* Check peer node CSID entry is present or Not */
	ret = rte_hash_lookup_data(csid_by_peer_node_hash,
					old_key, (void **)&csid);
	if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "CSID Entry not found for peer_info ");
			return -1;
	} else {
		/* Peer node CSID Entry is present. Delete the CSID Entry */
		ret = rte_hash_del_key(csid_by_peer_node_hash, old_key);
		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "Failed to delete csid entry");
			return -1;
		}
		/* CSID Entry add if not present */
		ret = rte_hash_add_key_data(csid_by_peer_node_hash,
						new_key, csid);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add entry for csid : %u Error= %s", *csid, rte_strerror(abs(ret)));
			return -1;
		}
	}

	LOG_MSG(LOG_DEBUG, "Key updated for CSID:%u", *csid);
	return *csid;
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
	int ret = 0;
	csid_t *csid = NULL;

	/* Check csid  entry is present or Not */
	ret = rte_hash_lookup_data(csid_by_peer_node_hash,
				key, (void **)&csid);

	if ( ret < 0) {
		LOG_MSG(LOG_DEBUG, "Entry not found in peer node hash table..");

		/* Allocate the memory for local CSID */
		csid = rte_zmalloc_socket(NULL, sizeof(csid_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (csid == NULL) {
			LOG_MSG(LOG_ERROR, "Failed to allocate the memory for csid");
		}

		/* Assign the local csid */
		csid->local_csid = ++local_csid;

		/* CSID Entry add if not present */
		ret = rte_hash_add_key_data(csid_by_peer_node_hash,
						key, csid);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add entry for csid : %u Error= %s",
					csid->local_csid, rte_strerror(abs(ret)));
			return -1;
		}
	}

	LOG_MSG(LOG_DEBUG, "CSID : %u", csid->local_csid);
	return csid->local_csid;

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
	int ret = 0;
	uint16_t *csid = NULL;

	/* Check peer node CSID entry is present or Not */
	ret = rte_hash_lookup_data(csid_by_peer_node_hash,
					key, (void **)&csid);
	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "Failed to delete csid entry");
		return -1;
	}
	/* Peer node CSID Entry is present. Delete the CSID Entry */
	ret = rte_hash_del_key(csid_by_peer_node_hash, key);

	/* Free data from hash */
	rte_free(csid);

	LOG_MSG(LOG_DEBUG, "Peer node CSID entry deleted");

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
	int ret = 0;
	fq_csids *tmp = NULL;

	/* Lookup for local CSID entry. */
	ret = rte_hash_lookup_data(peer_csids_by_csid_hash,
				&csid, (void **)&tmp);

	if ( ret < 0) {
		/* Local CSID Entry not present. Add CSID Entry */
		ret = rte_hash_add_key_data(peer_csids_by_csid_hash,
						&csid, csids);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add entry for CSID: %u Error= %s",
					csid, rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, csids, sizeof(fq_csids));
	}

	LOG_MSG(LOG_DEBUG, "CSID entry added for CSID: %u", csid);
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
	int ret = 0;
	fq_csids *tmp = NULL;

	ret = rte_hash_lookup_data(peer_csids_by_csid_hash,
				&csid, (void **)&tmp);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "Entry not found for CSID: %u", csid);
		return NULL;
	}

	LOG_MSG(LOG_DEBUG, "Entry found for CSID: %u", csid);
	return tmp;

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
	int ret = 0;
	fq_csids *tmp = NULL;

	/* Check local CSID entry is present or Not */
	ret = rte_hash_lookup_data(peer_csids_by_csid_hash,
					&csid, (void **)&tmp);
	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "Entry not found for CSID: %u", csid);
		return -1;
	}
	/* Local CSID Entry is present. Delete local csid Entry */
	ret = rte_hash_del_key(peer_csids_by_csid_hash, &csid);

	/* Free data from hash */
	rte_free(tmp);
	tmp = NULL;

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
	int ret = 0;
	sess_csid *tmp = NULL;

	/* Lookup for csid entry. */
	ret = rte_hash_lookup_data(seids_by_csid_hash,
				&csid, (void **)&tmp);

	if ( ret < 0) {
		/* CSID Entry not present. Add CSID Entry in table */
		ret = rte_hash_add_key_data(seids_by_csid_hash,
						&csid, seids);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add Session IDs entry for CSID = %u Error= %s",
					csid, rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, seids, sizeof(sess_csid));
	}

	LOG_MSG(LOG_DEBUG, "Session IDs entry added for CSID:%u", csid);
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
	int ret = 0;
	sess_csid *tmp = NULL;

	/* Retireve CSID entry */
	ret = rte_hash_lookup_data(seids_by_csid_hash,
				&csid, (void **)&tmp);

	if ( ret < 0) {
		LOG_MSG(LOG_DEBUG, "Entry not found for CSID: %u", csid);

		/* Allocate the memory for session IDs */
		tmp = rte_zmalloc_socket(NULL, sizeof(sess_csid),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (tmp == NULL) {
			LOG_MSG(LOG_ERROR, "Failed to allocate the memory for csid");
			return NULL;
		}

		/* CSID Entry not present. Add CSID Entry in table */
		ret = rte_hash_add_key_data(seids_by_csid_hash,
						&csid, tmp);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add Session IDs entry for CSID = %u"
					"\n\tError= %s", csid,
					rte_strerror(abs(ret)));
			return NULL;
		}
	}

	LOG_MSG(LOG_DEBUG, "Entry Found for CSID:%u", csid);

	return tmp;

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
	int ret = 0;
	sess_csid *tmp = NULL;

	/* Check CSID entry is present or Not */
	ret = rte_hash_lookup_data(seids_by_csid_hash,
					&csid, (void **)&tmp);
	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "Entry not found for CSID:%u", csid);
		return -1;
	}

	/* CSID Entry is present. Delete Session Entry */
	ret = rte_hash_del_key(seids_by_csid_hash, &csid);

	/* Free data from hash */
	rte_free(tmp);

	LOG_MSG(LOG_DEBUG, "Sessions IDs Entry deleted for CSID:%u", csid);
	return 0;
}

/**
 *Init the hash tables for FQ-CSIDs */
int8_t
init_fqcsid_hash_tables(void)
{
	struct rte_hash_parameters
		pfcp_hash_params[NUM_OF_TABLES] = {
		{	.name = "CSID_BY_PEER_NODE_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(csid_key),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "PEER_CSIDS_BY_CSID_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "SEIDS_BY_CSID_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "LOCAL_CSIDS_BY_NODE_ADDR_HASH",
			.entries = NUM_OF_NODE,
			.key_len = sizeof(uint32_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "LOCAL_CSIDS_BY_MMECSID_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "LOCAL_CSIDS_BY_PGWCSID_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "LOCAL_CSIDS_BY_SGWCSID_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		}
	};

	csid_by_peer_node_hash = rte_hash_create(&pfcp_hash_params[0]);
    assert(csid_by_peer_node_hash != NULL);

	peer_csids_by_csid_hash = rte_hash_create(&pfcp_hash_params[1]);
    assert(peer_csids_by_csid_hash != NULL);

	seids_by_csid_hash = rte_hash_create(&pfcp_hash_params[2]);
    assert(seids_by_csid_hash != NULL);

	local_csids_by_node_addr_hash = rte_hash_create(&pfcp_hash_params[3]);
    assert(local_csids_by_node_addr_hash != NULL);

	local_csids_by_mmecsid_hash = rte_hash_create(&pfcp_hash_params[4]);
    assert(local_csids_by_mmecsid_hash != NULL);

	local_csids_by_pgwcsid_hash = rte_hash_create(&pfcp_hash_params[5]);
    assert(local_csids_by_pgwcsid_hash != NULL);

	local_csids_by_sgwcsid_hash = rte_hash_create(&pfcp_hash_params[6]);
    assert(local_csids_by_sgwcsid_hash != NULL);

	return 0;
}

