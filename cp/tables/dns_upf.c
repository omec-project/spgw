// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "tables/tables.h"
#include <rte_hash.h>
#include <rte_jhash.h>
#include "clogger.h"
#include "rte_lcore.h"
#include "rte_debug.h"
#include "rte_errno.h"
#include <rte_hash_crc.h>
#include "gen_utils.h"
#include "ue.h"


/* upflist returned via DNS query */
struct rte_hash *upflist_by_ue_hash;

void
create_upf_by_ue_hash(void)
{
	struct rte_hash_parameters rte_hash_params = {
			.name = "upflist_by_ue_hash",
	    .entries = BUFFERED_ENTRIES_DEFAULT,
	    .key_len = sizeof(uint64_t),
	    .hash_func = rte_jhash,
	    .hash_func_init_val = 0,
	    .socket_id = rte_socket_id(),
	};

	upflist_by_ue_hash = rte_hash_create(&rte_hash_params);
	if (!upflist_by_ue_hash) {
		rte_panic("%s hash create failed: %s (%u)\n.",
				rte_hash_params.name,
		    rte_strerror(rte_errno), rte_errno);
	}
}

int
upflist_by_ue_hash_entry_add(uint64_t *imsi_val, uint16_t imsi_len,
		upfs_dnsres_t *entry)
{
	uint64_t imsi = UINT64_MAX;
	memcpy(&imsi, imsi_val, imsi_len);

	/* TODO: Check before adding */
	int ret = rte_hash_add_key_data(upflist_by_ue_hash, &imsi,
			entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add entry in upflist_by_ue_hash"
				"hash table");
		return -1;
	}

	return 0;
}

int
upflist_by_ue_hash_entry_lookup(uint64_t *imsi_val, uint16_t imsi_len,
		upfs_dnsres_t **entry)
{
	uint64_t imsi = UINT64_MAX;
	memcpy(&imsi, imsi_val, imsi_len);

	/* TODO: Check before adding */
	int ret = rte_hash_lookup_data(upflist_by_ue_hash, &imsi,
			(void **)entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to search entry in upflist_by_ue_hash"
				"hash table");
		return ret;
	}

	return 0;
}

int
upflist_by_ue_hash_entry_delete(uint64_t *imsi_val, uint16_t imsi_len)
{
	uint64_t imsi = UINT64_MAX;
	upfs_dnsres_t *entry = NULL;
	memcpy(&imsi, imsi_val, imsi_len);

	int ret = rte_hash_lookup_data(upflist_by_ue_hash, &imsi,
			(void **)&entry);
	if (ret) {
		/* PDN Conn Entry is present. Delete PDN Conn Entry */
		ret = rte_hash_del_key(upflist_by_ue_hash, &imsi);

		if ( ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"IMSI entry is not found:%lu...\n",
						ERR_MSG, imsi);
			return -1;
		}
	}

	/* Free data from hash */
	if (entry != NULL)
		rte_free(entry);

	return 0;
}

