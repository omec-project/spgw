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

struct rte_hash *pdn_conn_hash;
#define MAX_HASH_SIZE (1 << 15)

void create_pdn_callid_hash(void)
{
    struct rte_hash_parameters pfcp_hash_params = 
        {	
            .name = "PDN_CONN_HASH",
            //.entries = MAX_PDN_HASH_SIZE,
            .entries = MAX_HASH_SIZE,
            .key_len = sizeof(uint32_t),
            .hash_func = rte_hash_crc,
            .hash_func_init_val = 0,
            .socket_id = rte_socket_id()
        };

	pdn_conn_hash = rte_hash_create(&pfcp_hash_params);
	if (!pdn_conn_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params.name,
		    rte_strerror(rte_errno), rte_errno);
	}
    return;
}
/**
 * Add PDN Connection entry in PDN hash table.
 *
 * @param CALL ID
 * key.
 * @param pdn_connection_t pdn
 * return 0 or 1.
 *
 */
uint8_t
add_pdn_conn_entry(uint32_t call_id, pdn_connection_t *pdn)
{
	int ret = 0;
	pdn_connection_t *tmp = NULL;

	/* Lookup for PDN Connection entry. */
	ret = rte_hash_lookup_data(pdn_conn_hash,
				&call_id, (void **)&tmp);

	if ( ret < 0) {
		/* PDN Connection Entry if not present */
		ret = rte_hash_add_key_data(pdn_conn_hash,
						&call_id, pdn);
		if (ret) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to add pdn connection for CALL_ID = %u"
					"\n\tError= %s\n",
					__func__, __LINE__, call_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, pdn, sizeof(pdn_connection_t));
	}

	clLog(clSystemLog, eCLSeverityDebug, "%s:%d PDN Connection entry add for CALL_ID:%u",
			__func__, __LINE__, call_id);
	return 0;
}

/**
 * Get PDN Connection entry from PDN hash table.
 *
 * @param CALL ID
 * key.
 * return pdn_connection pdn or NULL
 *
 */
pdn_connection_t *get_pdn_conn_entry(uint32_t call_id)
{
	int ret = 0;
	pdn_connection_t *pdn = NULL;

	/* Check PDN Conn entry is present or Not */
	ret = rte_hash_lookup_data(pdn_conn_hash,
				&call_id, (void **)&pdn);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Entry not found for CALL_ID:%u...\n",
				__func__, __LINE__, call_id);
		return NULL;
	}

	clLog(clSystemLog, eCLSeverityDebug, "%s:%d CALL_ID:%u",
			__func__, __LINE__, call_id);
	return pdn;

}

/**
 * Delete PDN Connection entry from PDN hash table.
 *
 * @param CALL ID
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_pdn_conn_entry(uint32_t call_id)
{
	int ret = 0;
	pdn_connection_t *pdn = NULL;

	/* Check PDN Conn entry is present or Not */
	ret = rte_hash_lookup_data(pdn_conn_hash,
					&call_id, (void **)&pdn);
	if (ret) {
		/* PDN Conn Entry is present. Delete PDN Conn Entry */
		ret = rte_hash_del_key(pdn_conn_hash, &call_id);

		if ( ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Entry not found for CALL_ID:%u...\n",
						__func__, __LINE__, call_id);
			return -1;
		}
	}

	/* Free data from hash */
	rte_free(pdn);

	clLog(clSystemLog, eCLSeverityDebug, "%s: CALL_ID:%u",
			__func__, call_id);

	return 0;
}

