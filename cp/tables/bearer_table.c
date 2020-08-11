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


struct rte_hash *bearer_by_fteid_hash;

void
create_bearer_hash(void)
{
    struct rte_hash_parameters rte_hash_params = {
        .name = "bearer_by_teid_hash",
        .entries = LDB_ENTRIES_DEFAULT,
        .key_len = sizeof(uint32_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    bearer_by_fteid_hash = rte_hash_create(&rte_hash_params);
    if (!bearer_by_fteid_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }
}

int 
bearer_context_entry_add_teidKey(uint32_t teid, eps_bearer_t *context)
{
	int ret;
    printf("%s bearer  entry add %u \n", __FUNCTION__, teid);
	ret = rte_hash_add_key_data(bearer_by_fteid_hash,
			(const void *)&teid, (void *)context);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return -1;
	}
	return 0;
}

int
get_bearer_by_teid(uint32_t teid, eps_bearer_t **entry)
{
	int ret = rte_hash_lookup_data(bearer_by_fteid_hash,
			(const void*) &(teid), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN bearer HASH [%u]\n",
				__func__, __LINE__, teid);
		return -1;
	}
	return 0;
}

int bearer_context_delete_entry_teidKey(uint32_t teid)
{
    rte_hash_del_key(bearer_by_fteid_hash, (const void *) &teid);
    return 0;
}
