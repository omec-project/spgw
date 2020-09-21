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


struct rte_hash *gx_context_by_sess_id_hash;
#define UPF_ENTRIES_DEFAULT (1 << 16)

void
create_gx_context_hash(void)
{
    struct rte_hash_parameters rte_hash_params = {
        .name = "gx_context_by_sess_id_hash",
        .entries = UPF_ENTRIES_DEFAULT,
        .key_len = MAX_LEN,
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    gx_context_by_sess_id_hash = rte_hash_create(&rte_hash_params);
    if (!gx_context_by_sess_id_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }
}

int 
gx_context_entry_add(uint8_t *sess_id, gx_context_t *context)
{
	int ret;
    printf("%s GX context entry add %s \n", __FUNCTION__, sess_id);
	ret = rte_hash_add_key_data(gx_context_by_sess_id_hash,
			(const void *)sess_id, (void *)context);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return -1;
	}
	return 0;

}

int
get_gx_context(uint8_t *sessid, gx_context_t **entry)
{
	int ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
			(const void*) (sessid), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN GX HASH [%lu]\n",
				__func__, __LINE__, sessid);
		return -1;
	}
	return 0;
}


int remove_gx_context(uint8_t *sessid)
{
    rte_hash_del_key(gx_context_by_sess_id_hash, (const void *) sessid);
    return 0;
}

