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
#include "util.h"

#define SM_HASH_SIZE (1 << 18)
struct rte_hash *ue_context_by_imsi_hash;
struct rte_hash *ue_context_by_fteid_hash;
struct rte_hash *seid_session_hash=NULL;

void
create_ue_hash(void)
{
    struct rte_hash_parameters rte_hash_imsi_params = {
        .name = "ue_context_by_imsi_hash",
        .entries = LDB_ENTRIES_DEFAULT,
        .key_len = sizeof(uint64_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    ue_context_by_imsi_hash = rte_hash_create(&rte_hash_imsi_params);
    if (!ue_context_by_imsi_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_imsi_params.name,
                rte_strerror(rte_errno), rte_errno);
    }

    struct rte_hash_parameters rte_hash_teid_params = {
        .name = "ue_context_by_fteid_hash",
        .entries = LDB_ENTRIES_DEFAULT,
        .key_len = sizeof(uint32_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    ue_context_by_fteid_hash = rte_hash_create(&rte_hash_teid_params);
    if (!ue_context_by_fteid_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_teid_params.name,
                rte_strerror(rte_errno), rte_errno);
    }

	struct rte_hash_parameters rte_hash_seid_params = {
		.name = "state_machine_hash",
	    .entries = SM_HASH_SIZE,
	    .key_len = sizeof(uint64_t),
	    .hash_func = rte_hash_crc,
	    .hash_func_init_val = 0,
	    .socket_id = rte_socket_id(),
	};

	seid_session_hash = rte_hash_create(&rte_hash_seid_params);
    if (!seid_session_hash) {
        rte_panic("%s hash create failed: %s (%u)\n",
                rte_hash_seid_params.name,
                rte_strerror(rte_errno), rte_errno);
    }
}

int 
ue_context_entry_add_imsiKey(ue_context_t *context)
{
	int ret;
    printf("%s UE context entry add \n", __FUNCTION__);
	ret = rte_hash_add_key_data(ue_context_by_imsi_hash,
			(const void *)&context->imsi, (void *)context);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return -1;
	}
	return 0;

}

int
ue_context_entry_lookup_imsiKey(uint64_t imsi, ue_context_t **entry)
{
	int ret = rte_hash_lookup_data(ue_context_by_imsi_hash,
			(const void*) &(imsi), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN UE HASH [%u]",
				__func__, __LINE__, imsi);
		return -1;
	}
	return 0;
}


int ue_context_delete_entry_imsiKey(uint64_t imsi)
{
    rte_hash_del_key(ue_context_by_imsi_hash, (const void *) &imsi);
    return 0;
}

int 
ue_context_entry_add_teidKey(uint32_t teid, ue_context_t *context)
{
	int ret;
    printf("%s UE context entry add teid %u \n", __FUNCTION__, teid);
	ret = rte_hash_add_key_data(ue_context_by_fteid_hash,
			(const void *)&teid, (void *)context);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return -1;
	}
	return 0;

}

/* LOOKUP - TEID to session */
int
get_ue_context(uint32_t teid, ue_context_t **entry)
{
	int ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
			(const void*) &(teid), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN UE HASH [%u]",
				__func__, __LINE__, teid);
		return -1;
	}
	return 0;
}

int ue_context_delete_entry_teidKey(uint32_t teid)
{
    rte_hash_del_key(ue_context_by_fteid_hash, (const void *) &teid);
    return 0;
}

/**
 * @brief  : Add session entry in state machine hash table.
 * @param  : sess_id, key.
 * @param  : resp_info Resp
 * @return : 0 or 1.
 */
uint8_t
add_sess_entry_seid(uint64_t sess_id, ue_context_t *context)
{
	int ret;
	ue_context_t *temp = NULL;

	/* Lookup for session entry. */
	ret = rte_hash_lookup_data(seid_session_hash,
				&sess_id, (void **)&temp);

	if ( ret < 0) {
		/* No session entry for sess_id
		 * Add session entry for sess_id at seid_session_hash.
		 */
		ret = rte_hash_add_key_data(seid_session_hash,
						&sess_id, context);
		if (ret) {
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to add entry = %lu"
					"\n\tError= %s\n",
					__func__, sess_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} 

	clLog(clSystemLog, eCLSeverityDebug, "%s: Sess Entry add for Sess ID:%lu\n",
			__func__, sess_id);
	return 0;
}

uint8_t
get_sess_entry_seid(uint64_t sess_id, ue_context_t **context)
{
	int ret = 0;
	ret = rte_hash_lookup_data(seid_session_hash,
				&sess_id, (void **)context);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s %s %d Entry not found for sess_id:%lu...\n",__func__,
				__file__, __LINE__,sess_id);
		return -1;
	}

	clLog(clSystemLog, eCLSeverityDebug, "%s %s %d Entry found for sess_id:%lu...\n",__func__,
				__file__, __LINE__,sess_id);
	return 0;

}

/* Requirement : Not sure why we lookup 2 tables. Need to understand this code and clean */
uint8_t
del_sess_entry_seid(uint64_t sess_id)
{
	int ret = 0;
	struct resp_info *resp = NULL;

	/* Check Session Entry is present or Not */
	ret = rte_hash_lookup_data(seid_session_hash,
					&sess_id, (void **)resp);
	if (ret) {
		/* Session Entry is present. Delete Session Entry */
		ret = rte_hash_del_key(seid_session_hash, &sess_id);

		if ( ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s %s %d:Entry not found for sess_id:%lu...\n",
						__func__, __file__, __LINE__, sess_id);
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
		}
	}

	/* Free data from hash */
	rte_free(resp);

	clLog(clSystemLog, eCLSeverityDebug, "%s: Sess ID:%lu\n",
			__func__, sess_id);

	return 0;
}
