// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <rte_errno.h>
#include "ue.h"
#include "cp_interface.h"
#include "clogger.h"
#include "cp_config.h"

/* LOOKUP TABLE */
struct rte_hash *ue_context_by_imsi_hash;
struct rte_hash *ue_context_by_fteid_hash;
struct rte_hash *pdn_by_fteid_hash;
struct rte_hash *bearer_by_fteid_hash;

/* base value and offset for seid generation */
const uint32_t s11_sgw_gtpc_base_teid = 0xC0FFEE;
static uint32_t s11_sgw_gtpc_teid_offset;
const uint32_t s5s8_sgw_gtpc_base_teid = 0xE0FFEE;
const uint32_t s5s8_pgw_gtpc_base_teid = 0xD0FFEE;
static uint32_t s5s8_pgw_gtpc_teid_offset;

/* base value and offset for teid generation */
static uint32_t sgw_gtpc_base_teid = 0xC0FFEE;
static uint32_t sgw_gtpc_teid_offset;
static uint32_t pgw_gtpc_base_teid = 0xD0FFEE;
static uint32_t pgw_gtpc_teid_offset;
static uint8_t teid_range = 0xf0;
static uint32_t sgw_gtpu_base_teid = 0xf0000001;
static uint32_t pgw_gtpu_base_teid = 0x00000001;
/*TODO : Decide how to diffrentiate between sgw and pgw teids*/

uint32_t base_s1u_sgw_gtpu_teid = 0xf0000000;

// AJAY : Lets understand how this teid range works
void
set_base_teid(uint8_t val)
{

	/* set cp teid_range value */
	teid_range = val;

	/* set base teid value */
	/* teid will start from index 1 instead of index 0*/
	if(cp_config->cp_type == SGWC || cp_config->cp_type == SAEGWC){
		sgw_gtpc_base_teid = (teid_range << 24);
		sgw_gtpc_base_teid++;
	}else if(cp_config->cp_type == PGWC){
		pgw_gtpc_base_teid = (teid_range << 24);
		pgw_gtpc_base_teid++;
	}
	return;
}

void
set_s1u_sgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context)
{
	uint8_t index = __builtin_ffs(~(context->teid_bitmap)) - 1;
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
		sgw_gtpu_base_teid = sgw_gtpc_base_teid + sgw_gtpc_teid_offset;
		++sgw_gtpc_teid_offset;
	}

	bearer->s1u_sgw_gtpu_teid = (sgw_gtpu_base_teid & 0x00ffffff)
		| ((teid_range + index) << 24);
	context->teid_bitmap |= (0x01 << index);
}

void
set_s5s8_sgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context)
{
	uint8_t index = __builtin_ffs(~(context->teid_bitmap)) - 1;
	/* Note: s5s8_sgw_gtpu_teid based s11_sgw_gtpc_teid
	 * Computation same as s1u_sgw_gtpu_teid
	 */
	bearer->s5s8_sgw_gtpu_teid = (sgw_gtpu_base_teid & 0x00ffffff)
	    | ((teid_range + index) << 24);
	context->teid_bitmap |= (0x01 << index);
}

void
set_s5s8_pgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context){
	uint8_t index = __builtin_ffs(~(context->teid_bitmap)) - 1;
	if (cp_config->cp_type == PGWC){
		pgw_gtpu_base_teid = pgw_gtpc_base_teid + pgw_gtpc_teid_offset;
		++pgw_gtpc_teid_offset;
	}
	bearer->s5s8_pgw_gtpu_teid = (pgw_gtpu_base_teid & 0x00ffffff)
		| ((teid_range + index) << 24);
	context->teid_bitmap |= (0x01 << index);
}

void
set_s5s8_pgw_gtpc_teid(pdn_connection_t *pdn)
{
	pdn->s5s8_pgw_gtpc_teid = s5s8_pgw_gtpc_base_teid
		+ s5s8_pgw_gtpc_teid_offset;
	++s5s8_pgw_gtpc_teid_offset;
}

void
set_s5s8_pgw_gtpu_teid_using_pdn(eps_bearer_t *bearer, pdn_connection_t *pdn)
{
	uint8_t index = __builtin_ffs(~(pdn->context->teid_bitmap)) - 1;
	/* Note: s5s8_sgw_gtpu_teid based s11_sgw_gtpc_teid
	 * Computation same as s1u_sgw_gtpu_teid
	 */
	bearer->s5s8_pgw_gtpu_teid = (pdn->s5s8_pgw_gtpc_teid & 0x00ffffff)
	    | ((0xf0 + index) << 24);
	pdn->context->teid_bitmap |= (0x01 << index);
}

void
create_ue_hash(void)
{
    struct rte_hash_parameters rte_hash_params = {
        .name = "ue_context_by_imsi_hash",
        .entries = LDB_ENTRIES_DEFAULT,
        .key_len = sizeof(uint64_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    ue_context_by_imsi_hash = rte_hash_create(&rte_hash_params);
    if (!ue_context_by_imsi_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }

    rte_hash_params.name = "ue_context_by_fteid_hash";
    rte_hash_params.key_len = sizeof(uint32_t);
    ue_context_by_fteid_hash = rte_hash_create(&rte_hash_params);
    if (!ue_context_by_fteid_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }

    rte_hash_params.name = "pdn_by_fteid_hash";
    rte_hash_params.key_len = sizeof(uint32_t);
    pdn_by_fteid_hash = rte_hash_create(&rte_hash_params);
    if (!pdn_by_fteid_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }

    rte_hash_params.name = "bearer_by_teid_hash";
    rte_hash_params.key_len = sizeof(uint32_t);
    bearer_by_fteid_hash = rte_hash_create(&rte_hash_params);
    if (!bearer_by_fteid_hash) {
        rte_panic("%s hash create failed: %s (%u)\n.",
                rte_hash_params.name,
                rte_strerror(rte_errno), rte_errno);
    }
}

void
print_ue_context_by(struct rte_hash *h, ue_context_t *context)
{
	uint64_t *key;
	int32_t ret;
	uint32_t next = 0;
	int i;
	clLog(clSystemLog, eCLSeverityDebug," %16s %1s %16s %16s %8s %8s %11s\n", "imsi", "u", "mei",
			"msisdn", "s11-teid", "s11-ipv4", "56789012345");
	if (context) {
		clLog(clSystemLog, eCLSeverityDebug,"*%16lx %1lx %16lx %16lx %8x %15s ", context->imsi,
		    (uint64_t) context->unathenticated_imsi, context->mei,
		    context->msisdn, context->s11_sgw_gtpc_teid,
		     inet_ntoa(context->s11_sgw_gtpc_ipv4));
		for (i = 0; i < MAX_BEARERS; ++i) {
			clLog(clSystemLog, eCLSeverityDebug,"%c", (context->bearer_bitmap & (1 << i))
					? '1' : '0');
		}
		clLog(clSystemLog, eCLSeverityDebug,"\t0x%04x\n", context->bearer_bitmap);
	}
	if (h == NULL)
		return;
	while (1) {
		ret = rte_hash_iterate(h, (const void **) &key,
				(void **) &context, &next);
		if (ret < 0)
			break;
		clLog(clSystemLog, eCLSeverityDebug," %16lx %1lx %16lx %16lx %8x %15s ",
			context->imsi,
			(uint64_t) context->unathenticated_imsi,
			context->mei,
		    context->msisdn, context->s11_sgw_gtpc_teid,
		    inet_ntoa(context->s11_sgw_gtpc_ipv4));
		for (i = 0; i < MAX_BEARERS; ++i) {
			clLog(clSystemLog, eCLSeverityDebug,"%c", (context->bearer_bitmap & (1 << i))
					? '1' : '0');
		}
		clLog(clSystemLog, eCLSeverityDebug,"\t0x%4x", context->bearer_bitmap);
		puts("");
	}
}

int
add_bearer_entry_by_sgw_s5s8_tied(uint32_t fteid_key, eps_bearer_t **bearer)
{
	int8_t ret = 0;
	ret = rte_hash_add_key_data(bearer_by_fteid_hash,
	    (const void *) &fteid_key, (void *) (*bearer));
	
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
			"%s - Error on rte_hash_add_key_data add\n",
			strerror(ret));
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}
	return 0;
}

int
create_ue_context(uint64_t *imsi_val, uint16_t imsi_len,
		uint8_t ebi, ue_context_t **context, apn_t *apn_requested,
	  	uint32_t sequence)
{
	int ret;
	int i;
	uint8_t ebi_index;
	uint64_t imsi = UINT64_MAX;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	int if_ue_present = 0;

	memcpy(&imsi, imsi_val, imsi_len);

	ret = rte_hash_lookup_data(ue_context_by_imsi_hash, &imsi,
	    (void **) &(*context));

	if (ret == -ENOENT) {
		(*context) = rte_zmalloc_socket(NULL, sizeof(ue_context_t),
		    RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (*context == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate ue context "
					"structure: %s (%s:%d)\n",
					rte_strerror(rte_errno),
					__FILE__,
					__LINE__);
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		(*context)->imsi = imsi;
		(*context)->imsi_len = imsi_len;
		ret = rte_hash_add_key_data(ue_context_by_imsi_hash,
		    (const void *) &(*context)->imsi, (void *) (*context));
		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
			rte_free((*context));
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
	} else {
		/* VS: TODO: Need to think on this, flush entry when received DSR*/
		RTE_SET_USED(apn_requested);
		if_ue_present = 1;
		if((*context)->eps_bearers[ebi - 5] != NULL ) {
			pdn = (*context)->eps_bearers[ebi - 5]->pdn;
			if(pdn != NULL ) {
				if(pdn->csr_sequence == sequence ) 
				{
					/* -2 : Discarding re-transmitted csr */
					return -2;
				}
			}
		}
		/*if ((strncmp(apn_requested->apn_name_label,
					(((*context)->pdns[ebi - 5])->apn_in_use)->apn_name_label,
					sizeof(apn_requested->apn_name_length))) == 0) {
			clLog(clSystemLog, eCLSeverityCritical,
				"%s- Discarding re-transmitted csr received for IMSI:%lu \n",
				__func__, imsi);
			return -1;
		}*/
	}
	if (if_ue_present == 0){
		if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
			(*context)->s11_sgw_gtpc_teid = s11_sgw_gtpc_base_teid
			    + s11_sgw_gtpc_teid_offset;
			++s11_sgw_gtpc_teid_offset;

		} else if (cp_config->cp_type == PGWC){
			(*context)->s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid
				+ s5s8_pgw_gtpc_teid_offset;
		}
	}else if (cp_config->cp_type == PGWC){
		(*context)->s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid
			+ s5s8_pgw_gtpc_teid_offset;
	}

	ret = rte_hash_add_key_data(ue_context_by_fteid_hash,
	    (const void *) &(*context)->s11_sgw_gtpc_teid,
	    (void *) (*context));

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
			"%s - Error on ue_context_by_fteid_hash add\n",
			strerror(ret));
		rte_hash_del_key(ue_context_by_imsi_hash,
		    (const void *) &(*context)->imsi);
		if (ret < 0) {
			/* If we get here something bad happened. The
			 * context that was added to
			 * ue_context_by_imsi_hash above was not able
			 * to be removed.
			 */
			rte_panic("%s - Error on "
				"ue_context_by_imsi_hash del\n",
				strerror(ret));
		}
		rte_free((*context));
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}

	ebi_index = ebi - 5;
	bearer = (*context)->eps_bearers[ebi_index];

	if (bearer) {
		if (pdn) {
			/* created session is overwriting old session... */
			/*  ...clean up old session's dedicated bearers */
			for (i = 0; i < MAX_BEARERS; ++i) {
				if (!pdn->eps_bearers[i])
					continue;
				if (i == ebi_index) {
					bzero(bearer, sizeof(*bearer));
					continue;
				}
				rte_free(pdn->eps_bearers[i]);
				pdn->eps_bearers[i] = NULL;
				(*context)->eps_bearers[i] = NULL;
				(*context)->bearer_bitmap &= ~(1 << ebi_index);
			}
		} else {
			/* created session is creating a default bearer in place */
			/* of a different pdn connection's dedicated bearer */
			bearer->pdn->eps_bearers[ebi_index] = NULL;
			bzero(bearer, sizeof(*bearer));
			pdn = rte_zmalloc_socket(NULL,
				sizeof(pdn_connection_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
			if (pdn == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate PDN "
						"structure: %s (%s:%d)\n",
						rte_strerror(rte_errno),
						__FILE__,
						__LINE__);
				return GTPV2C_CAUSE_SYSTEM_FAILURE;
			}
			pdn->num_bearer++;
			(*context)->pdns[ebi_index] = pdn;
			(*context)->num_pdns++;
			pdn->eps_bearers[ebi_index] = bearer;
			pdn->default_bearer_id = ebi;
		}
	} else {
		/*
		 * Allocate default bearer
		 */
		bearer = rte_zmalloc_socket(NULL, sizeof(eps_bearer_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (bearer == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate bearer "
					"structure: %s (%s:%d)\n",
					rte_strerror(rte_errno),
					__FILE__,
					__LINE__);
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		bearer->eps_bearer_id = ebi;
		pdn = rte_zmalloc_socket(NULL, sizeof(pdn_connection_t),
		    RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (pdn == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate PDN "
					"structure: %s (%s:%d)\n",
					rte_strerror(rte_errno),
					__FILE__,
					__LINE__);
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		pdn->num_bearer++;
		(*context)->eps_bearers[ebi_index] = bearer;
		(*context)->pdns[ebi_index] = pdn;
		(*context)->num_pdns++;
		(*context)->bearer_bitmap |= (1 << ebi_index);
		pdn->eps_bearers[ebi_index] = bearer;
		pdn->default_bearer_id = ebi;
	}

	for (i = 0; i < MAX_FILTERS_PER_UE; ++i)
		bearer->packet_filter_map[i] = -ENOENT;


	bearer->pdn = pdn;
	bearer->eps_bearer_id = ebi;

	pdn = (*context)->pdns[ebi_index];
	bearer = (*context)->eps_bearers[ebi_index];

	ret = rte_hash_add_key_data(pdn_by_fteid_hash,
	    (const void *) &(*context)->s11_sgw_gtpc_teid,
	    (void *) pdn);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
			"%s - Error on pdn_by_fteid_hash add\n",
			strerror(ret));
		rte_hash_del_key(pdn_by_fteid_hash,
		    (const void *) &(*context)->s11_sgw_gtpc_teid);
		if (ret < 0) {
			/* If we get here something bad happened. The
			 * context that was added to
			 * ue_context_by_imsi_hash above was not able
			 * to be removed.
			 */
			rte_panic("%s - Error on "
				"pdn_by_fteid_hash del\n",
				strerror(ret));
		}
		rte_free((*context));
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}

	return 0;
}


