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
#include "gen_utils.h"

#define MAX_HASH_SIZE (1 << 15)

#define PFCP_CNTXT_HASH_SIZE (1 << 15)

/* Number of Entries: 1023 can be stored */
#define MAX_RULES_ENTRIES_HASH_SIZE (1 << 10)

struct rte_hash *pfcp_cntxt_hash;
struct rte_hash *pdr_entry_hash;
struct rte_hash *qer_entry_hash;
struct rte_hash *urr_entry_hash;
struct rte_hash *rule_name_bearer_id_map_hash;

/**
 * @brief Initializes the pfcp context hash table used to account for
 * PDR, QER, BAR and FAR rules information in control plane.
 */
void
init_pfcp_tables(void)
{
	struct rte_hash_parameters
		pfcp_hash_params[6] = {
		{	.name = "PFCP_CNTXT_HASH",
			.entries = PFCP_CNTXT_HASH_SIZE,
			.key_len = sizeof(uint64_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "RULE_NAME_BEARER_ID_HASH",
			.entries = MAX_RULES_ENTRIES_HASH_SIZE,
			.key_len = sizeof(rule_name_key_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "PDR_ENTRY_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint16_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "QER_ENTRY_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint32_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		},
		{	.name = "URR_ENTRY_HASH",
			.entries = MAX_HASH_SIZE,
			.key_len = sizeof(uint32_t),
			.hash_func = rte_hash_crc,
			.hash_func_init_val = 0,
			.socket_id = rte_socket_id()
		}
	};

	pfcp_cntxt_hash = rte_hash_create(&pfcp_hash_params[0]);
	if (!pfcp_cntxt_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params[0].name,
		    rte_strerror(rte_errno), rte_errno);
	}

	rule_name_bearer_id_map_hash = rte_hash_create(&pfcp_hash_params[1]);
	if (!rule_name_bearer_id_map_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params[1].name,
		    rte_strerror(rte_errno), rte_errno);
	}

	pdr_entry_hash = rte_hash_create(&pfcp_hash_params[2]);
	if (!pdr_entry_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params[2].name,
		    rte_strerror(rte_errno), rte_errno);
	}

	qer_entry_hash = rte_hash_create(&pfcp_hash_params[3]);
	if (!qer_entry_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params[3].name,
		    rte_strerror(rte_errno), rte_errno);
	}

	urr_entry_hash = rte_hash_create(&pfcp_hash_params[4]);
	if (!urr_entry_hash) {
		rte_panic("%s: hash create failed: %s (%u)\n",
				pfcp_hash_params[4].name,
		    rte_strerror(rte_errno), rte_errno);
	}

}

/**
 * Add Rule name entry with bearer identifier in Rule and bearer map hash table.
 *
 * @param Rule_Name
 * key.
 * @param uint8_t bearer id
 * return 0 or 1.
 *
 */
uint8_t
add_rule_name_entry(const rule_name_key_t rule_key, bearer_id_t *bearer)
{
	int ret = 0;
	bearer_id_t *tmp = NULL;

	/* Lookup for Rule entry. */
	ret = rte_hash_lookup_data(rule_name_bearer_id_map_hash,
				&rule_key, (void **)&tmp);

	if ( ret < 0) {
		/* Rule Entry if not present */
		ret = rte_hash_add_key_data(rule_name_bearer_id_map_hash,
						&rule_key, bearer);
		if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to add rule entry for Rule_Name = %s"
					"\n\tError= %s\n",
					__func__, __LINE__, rule_key.rule_name,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, bearer, sizeof(bearer_id_t));
	}

	LOG_MSG(LOG_DEBUG, "%s: Rule Name entry add for Rule_Name:%s, Bearer_id:%u\n",
			__func__, rule_key.rule_name, bearer->bearer_id);
	return 0;
}

/**
 * Get Rule Name entry from Rule and Bearer Map table.
 *
 * @param Rule_Name
 * key.
 * return Bearer ID or NULL
 *
 */
int8_t
get_rule_name_entry(const rule_name_key_t rule_key)
{
	int ret = 0;
	bearer_id_t *bearer = NULL;

	/* Check Rule Name entry is present or Not */
	ret = rte_hash_lookup_data(rule_name_bearer_id_map_hash,
				&rule_key, (void **)&bearer);

	if ( ret < 0) {
		/* LOG_MSG(LOG_ERROR, "%s:%d Entry not found for Rule_Name:%s...\n",
				__func__, __LINE__, rule_key.rule_name); */
		return -1;
	}

	LOG_MSG(LOG_DEBUG, "%s: Rule_Name:%s, Bearer_ID:%u\n",
			__func__, rule_key.rule_name, bearer->bearer_id);
	return bearer->bearer_id;

}

/**
 * Delete Rule Name entry from Rule and Bearer Map hash table.
 *
 * @param Rule_Name
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_rule_name_entry(const rule_name_key_t rule_key)
{
	int ret = 0;
	bearer_id_t *bearer = NULL;

	/* Check Rule Name entry is present or Not */
	ret = rte_hash_lookup_data(rule_name_bearer_id_map_hash,
					&rule_key, (void **)bearer);
	if (ret) {
		/* Rule Name Entry is present. Delete Rule Name Entry */
		ret = rte_hash_del_key(rule_name_bearer_id_map_hash, &rule_key);
		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "Entry not found for Rule_Name:%s...",
						rule_key.rule_name);
			return -1;
		}
		LOG_MSG(LOG_DEBUG, "Rule_Name:%s is found ",
				rule_key.rule_name);
	} else {
		LOG_MSG(LOG_DEBUG, "Rule_Name:%s is not found ",
				rule_key.rule_name);
	}

	/* Free data from hash */
	if (bearer != NULL) {
		rte_free(bearer);
		bearer = NULL;
	}

	return 0;
}

/**
 * Add PDR entry in PDR hash table.
 *
 * @param rule_id/PDR_ID
 * key.
 * @param pdr_t cntxt
 * return 0 or 1.
 *
 */
uint8_t
add_pdr_entry(uint16_t rule_id, pdr_t *cntxt)
{
	int ret = 0;
	pdr_t *tmp = NULL;

	/* Lookup for PDR entry. */
	ret = rte_hash_lookup_data(pdr_entry_hash,
				&rule_id, (void **)&tmp);

	if ( ret < 0) {
		/* PDR Entry not present. Add PDR Entry */
		ret = rte_hash_add_key_data(pdr_entry_hash,
						&rule_id, cntxt);
		if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to add entry for PDR_ID = %u"
					"\n\tError= %s\n",
					__func__, __LINE__, rule_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, cntxt, sizeof(struct pfcp_cntxt));
	}

	LOG_MSG(LOG_DEBUG, "PDR entry add for PDR_ID:%u", rule_id);
	return 0;
}

/**
 * Get PDR entry from PDR hash table.
 *
 * @param PDR ID
 * key.
 * return pdr_t cntxt or NULL
 *
 */
pdr_t *get_pdr_entry(uint16_t rule_id)
{
	int ret = 0;
	pdr_t *cntxt = NULL;

	ret = rte_hash_lookup_data(pdr_entry_hash,
				&rule_id, (void **)&cntxt);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:%d Entry not found for PDR_ID:%u...\n",
				__func__, __LINE__, rule_id);
		return NULL;
	}

	LOG_MSG(LOG_DEBUG, "%s: PDR_ID:%u\n",
			__func__, rule_id);
	return cntxt;

}

/**
 * Get PDR entry from PDR hash table.
 * update entry
 */
int
update_pdr_teid(eps_bearer_t *bearer, uint32_t teid, uint32_t ip, uint8_t iface)
{
	int ret = -1;

    LOG_MSG(LOG_DEBUG, "update_pdr_teid bearer->pdr_count %d ", bearer->pdr_count);
	for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
		if(bearer->pdrs[itr]->pdi.src_intfc.interface_value == iface){
			bearer->pdrs[itr]->pdi.local_fteid.teid = teid;
			bearer->pdrs[itr]->pdi.local_fteid.ipv4_address = htonl(ip);
			LOG_MSG(LOG_DEBUG, "Updated pdr entry Successfully for PDR_ID:%u",
					bearer->pdrs[itr]->rule_id);
			ret = 0;
			break;
		}
	}
	return ret;
}

/**
 * Delete PDR entry from PDR hash table.
 *
 * @param PDR ID
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_pdr_entry(uint16_t rule_id)
{
	int ret = 0;
	pdr_t *cntxt = NULL;

	/* Check PDR entry is present or Not */
	ret = rte_hash_lookup_data(pdr_entry_hash,
					&rule_id, (void **)cntxt);
	if (ret) {
		/* PDR Entry is present. Delete PDR Entry */
		ret = rte_hash_del_key(pdr_entry_hash, &rule_id);

		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Entry not found for PDR_ID:%u...\n",
						__func__, __LINE__, rule_id);
			return -1;
		}
	}

	/* Free data from hash */
	rte_free(cntxt);
	cntxt = NULL;

	LOG_MSG(LOG_DEBUG, "%s: PDR_ID:%u\n",
			__func__, rule_id);

	return 0;
}
/**
 * Add context entry in pfcp context hash table.
 *
 * @param sess_id
 * key.
 * @param pfcp_cntxt cntxt
 * return 0 or 1.
 *
 */
uint8_t
add_pfcp_cntxt_entry(uint64_t sess_id, struct pfcp_cntxt *cntxt)
{
	int ret = 0;
	struct pfcp_cntxt *tmp = NULL;

	/* Lookup for pfcp context entry. */
	ret = rte_hash_lookup_data(pfcp_cntxt_hash,
				&sess_id, (void **)&tmp);

	if ( ret < 0) {
		/* pfcp context Entry not present. Add pfcp context Entry */
		ret = rte_hash_add_key_data(pfcp_cntxt_hash,
						&sess_id, cntxt);
		if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to add entry for Sess_id = %lu"
					"\n\tError= %s\n",
					__func__, __LINE__, sess_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, cntxt, sizeof(struct pfcp_cntxt));
	}

	LOG_MSG(LOG_DEBUG, "%s: PFCP context entry add for Sess_Id:%lu\n",
			__func__, sess_id);
	return 0;
}

/**
 * Get PFCP Context entry from pfcp context table.
 *
 * @param SESS ID
 * key.
 * return pfcp_cntxt cntxt or NULL
 *
 */

struct pfcp_cntxt *
get_pfcp_cntxt_entry(uint64_t sess_id)
{
	int ret = 0;
	struct pfcp_cntxt *cntxt = NULL;

	ret = rte_hash_lookup_data(pfcp_cntxt_hash,
				&sess_id, (void **)&cntxt);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:%d Entry not found for Sess_Id:%lu...\n",
				__func__, __LINE__, sess_id);
		return NULL;
	}

	LOG_MSG(LOG_DEBUG, "%s: Sess_Id:%lu\n",
			__func__, sess_id);
	return cntxt;

}

/**
 * Delete PFCP context entry from PFCP Context hash table.
 *
 * @param SESS ID
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_pfcp_cntxt_entry(uint64_t sess_id)
{
	int ret = 0;
	struct pfcp_cntxt *cntxt = NULL;

	/* Check pfcp context entry is present or Not */
	ret = rte_hash_lookup_data(pfcp_cntxt_hash,
					&sess_id, (void **)&cntxt);
	if (ret) {
		/* pfcp context Entry is present. Delete Session Entry */
		ret = rte_hash_del_key(pfcp_cntxt_hash, &sess_id);

		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Entry not found for Sess_Id:%lu...\n",
						__func__, __LINE__, sess_id);
			return -1;
		}
	}

	/* Free data from hash */
	rte_free(cntxt);

	LOG_MSG(LOG_DEBUG, "%s: Sess_Id:%lu\n",
			__func__, sess_id);

	return 0;
}

/**
 * Add QER entry in QER hash table.
 *
 * @param qer_id
 * key.
 * @param qer_t context
 * return 0 or 1.
 *
 */
uint8_t
add_qer_entry(uint32_t qer_id, qer_t *cntxt)
{
	int ret = 0;
	qer_t *tmp = NULL;

	/* Lookup for QER entry. */
	ret = rte_hash_lookup_data(qer_entry_hash,
				&qer_id, (void **)&tmp);

	if ( ret < 0) {
		/* QER Entry not present. Add QER Entry in table */
		ret = rte_hash_add_key_data(qer_entry_hash,
						&qer_id, cntxt);
		if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to add QER entry for QER_ID = %u"
					"\n\tError= %s\n",
					__func__, __LINE__, qer_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, cntxt, sizeof(qer_t));
	}

	LOG_MSG(LOG_DEBUG, "QER entry add for QER_ID:%u",qer_id);
	return 0;
}

/**
 * Get QER entry from QER hash table.
 *
 * @param QER ID
 * key.
 * return qer_t cntxt or NULL
 *
 */
qer_t *get_qer_entry(uint32_t qer_id)
{
	int ret = 0;
	qer_t *cntxt = NULL;

	/* Retireve QER entry */
	ret = rte_hash_lookup_data(qer_entry_hash,
				&qer_id, (void **)&cntxt);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "Entry not found for QER_ID:%u...", qer_id);
		return NULL;
	}

	LOG_MSG(LOG_DEBUG, "QER_ID:%u", qer_id);
	return cntxt;

}

/**
 * Delete QER entry from QER hash table.
 *
 * @param QER ID
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_qer_entry(uint32_t qer_id)
{
	int ret = 0;
	qer_t *cntxt = NULL;

	/* Check QER entry is present or Not */
	ret = rte_hash_lookup_data(qer_entry_hash,
					&qer_id, (void **)cntxt);
	if (ret) {
		/* QER Entry is present. Delete Session Entry */
		ret = rte_hash_del_key(qer_entry_hash, &qer_id);

		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Entry not found for QER_ID:%u...\n",
						__func__, __LINE__, qer_id);
			return -1;
		}
	}

	/* Free data from hash */
	if (cntxt != NULL)
		rte_free(cntxt);

	LOG_MSG(LOG_DEBUG, "%s: QER_ID:%u\n",
			__func__, qer_id);

	return 0;
}

/**
 * Add URR entry in URR hash table.
 *
 * @param urr_id
 * key.
 * @param urr_t context
 * return 0 or 1.
 *
 */
uint8_t
add_urr_entry(uint32_t urr_id, urr_t *cntxt)
{
	int ret = 0;
	urr_t *tmp = NULL;

	/* Lookup for URR entry. */
	ret = rte_hash_lookup_data(urr_entry_hash,
				&urr_id, (void **)&tmp);

	if ( ret < 0) {
		/* URR Entry not present. Add URR Entry in table */
		ret = rte_hash_add_key_data(urr_entry_hash,
						&urr_id, cntxt);
		if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to add URR entry for URR_ID = %u"
					"\n\tError= %s\n",
					__func__, __LINE__, urr_id,
					rte_strerror(abs(ret)));
			return -1;
		}
	} else {
		memcpy(tmp, cntxt, sizeof(urr_t));
	}

	LOG_MSG(LOG_DEBUG, "%s: URR entry add for URR_ID:%u\n",
			__func__, urr_id);
	return 0;
}

/**
 * Get URR entry from urr hash table.
 *
 * @param URR ID
 * key.
 * return urr_t cntxt or NULL
 *
 */
urr_t *get_urr_entry(uint32_t urr_id)
{
	int ret = 0;
	urr_t *cntxt = NULL;

	/* Retireve URR entry */
	ret = rte_hash_lookup_data(urr_entry_hash,
				&urr_id, (void **)&cntxt);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:%d Entry not found for URR_ID:%u...\n",
				__func__, __LINE__, urr_id);
		return NULL;
	}

	LOG_MSG(LOG_DEBUG, "%s: URR_ID:%u\n",
			__func__, urr_id);
	return cntxt;

}

/**
 * Delete URR entry from URR hash table.
 *
 * @param URR ID
 * key.
 * return 0 or 1.
 *
 */
uint8_t
del_urr_entry(uint32_t urr_id)
{
	int ret = 0;
	urr_t *cntxt = NULL;

	/* Check URR entry is present or Not */
	ret = rte_hash_lookup_data(urr_entry_hash,
					&urr_id, (void **)&cntxt);
	if (ret) {
		/* URR Entry is present. Delete Session Entry */
		ret = rte_hash_del_key(urr_entry_hash, &urr_id);

		if ( ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Entry not found for URR_ID:%u...\n",
						__func__, __LINE__, urr_id);
			return -1;
		}
	}

	/* Free data from hash */
	rte_free(cntxt);

	LOG_MSG(LOG_DEBUG, "%s: URR_ID:%u\n",
			__func__, urr_id);

	return 0;
}


