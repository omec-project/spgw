// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#include <rte_hash.h>
#include <rte_jhash.h>
#include "cp_log.h"
#include "rte_lcore.h"
#include "rte_debug.h"
#include "rte_errno.h"


struct rte_hash *pdn_by_fteid_hash;

void
create_pdn_hash(void)
{
    struct rte_hash_parameters rte_hash_params = {
        .name = "pdn_by_fteid_hash",
        .entries = LDB_ENTRIES_DEFAULT,
        .key_len = sizeof(uint32_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    pdn_by_fteid_hash = rte_hash_create(&rte_hash_params);
    assert(pdn_by_fteid_hash != NULL);

}

int 
pdn_context_entry_add_teidKey(uint32_t teid, pdn_connection_t *context)
{
	int ret;
	ret = rte_hash_add_key_data(pdn_by_fteid_hash,
			(const void *)&teid, (void *)context);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR,
				"%s - Error on rte_hash_add_key_data add", strerror(ret));
		return -1;
	}
    LOG_MSG(LOG_DEBUG5, "Add PDN context entry  %u ",teid);
	return 0;

}

int
get_pdn_context(uint32_t teid, pdn_connection_t **entry)
{
	int ret = rte_hash_lookup_data(pdn_by_fteid_hash,
			(const void*) &(teid), (void **) entry);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN PDN HASH [%u]", teid);
		return -1;
	}
    LOG_MSG(LOG_DEBUG5, "Get PDN context entry  %u ",teid);
	return 0;
}

int pdn_context_delete_entry_teidKey(uint32_t teid)
{
    rte_hash_del_key(pdn_by_fteid_hash, (const void *) &teid);
    LOG_MSG(LOG_DEBUG5, "Delete PDN context entry  %u ",teid);
    return 0;
}
