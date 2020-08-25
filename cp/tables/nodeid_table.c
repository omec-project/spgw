// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
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


struct rte_hash *node_id_hash;

void
create_node_id_hash(void)
{

	struct rte_hash_parameters rte_hash_params = {
		.name = "node_id_hash",
		.entries = LDB_ENTRIES_DEFAULT,
		.key_len = sizeof(uint32_t),
		.hash_func = rte_hash_crc,
		.hash_func_init_val = 0,
		.socket_id = rte_socket_id()
	};

	node_id_hash = rte_hash_create(&rte_hash_params);
	if (!node_id_hash) {
		rte_panic("%s hash create failed: %s (%u)\n.",
				rte_hash_params.name,
				rte_strerror(rte_errno), rte_errno);
	}

}

uint8_t
add_node_id_hash(uint32_t *nodeid, uint64_t *data )
{
	int ret = 0;
	uint32_t key = UINT32_MAX;
	uint64_t *temp = NULL;
	memcpy(&key ,nodeid, sizeof(uint32_t));

	temp =(uint64_t *) rte_zmalloc_socket(NULL, sizeof(uint64_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (temp == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate ue context "
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return 1;
	}
	*temp = *data;
	ret = rte_hash_add_key_data(node_id_hash,
			(const void *)&key , (void *)temp);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		rte_free((temp));
		return 1;
	}
	return 0;
}
