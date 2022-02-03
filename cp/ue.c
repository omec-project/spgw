// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "ue.h"
#include "cp_interface.h"
#include "cp_log.h"
#include "spgw_config_struct.h"
#include "util.h"
#include "pfcp_cp_session.h"
#include "gen_utils.h"
#include "gtpv2_session.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"

/* base value and offset for seid generation */
const uint32_t s11_sgw_gtpc_base_teid = 0xC0FFEE;
static uint32_t s11_sgw_gtpc_teid_offset;
static uint32_t s5s8_pgw_gtpc_teid_offset;

/* base value and offset for teid generation */
static uint32_t sgw_gtpc_base_teid = 0xC0FFEE;
static uint32_t sgw_gtpc_teid_offset;
static uint32_t pgw_gtpc_base_teid = 0xD0FFEE;
const uint32_t s5s8_pgw_gtpc_base_teid = 0xD0FFEE;
static uint32_t pgw_gtpc_teid_offset;
static uint8_t teid_range = 0xf0;
static uint32_t sgw_gtpu_base_teid = 0xf0000001;
static uint32_t pgw_gtpu_base_teid = 0x00000001;
/*TODO : Decide how to diffrentiate between sgw and pgw teids*/

uint32_t base_s1u_sgw_gtpu_teid = 0xf0000000;

// Requirement: Understand how this teid range works
void
set_base_teid(uint8_t val)
{

	/* set cp teid_range value */
	teid_range = val;

	/* set base teid value */
	/* teid will start from index 1 instead of index 0*/
	if(cp_config->cp_type == SAEGWC){
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
	if (cp_config->cp_type == SAEGWC) {
		sgw_gtpu_base_teid = sgw_gtpc_base_teid + sgw_gtpc_teid_offset;
		++sgw_gtpc_teid_offset;
	}

	bearer->s1u_sgw_gtpu_teid = (sgw_gtpu_base_teid & 0x00ffffff)
		| ((teid_range + index) << 24);
	context->teid_bitmap |= (0x01 << index);
}


void
set_s5s8_pgw_gtpc_teid(pdn_connection_t *pdn)
{
	pdn->s5s8_pgw_gtpc_teid = s5s8_pgw_gtpc_base_teid + s5s8_pgw_gtpc_teid_offset;
	++s5s8_pgw_gtpc_teid_offset;
}

void
set_s5s8_pgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context)
{
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


int
create_ue_context(uint64_t *imsi_val, uint16_t imsi_len,
		uint8_t ebi, ue_context_t **context, uint8_t *apn, uint8_t  apn_len)
{
	int ret;
	int i;
	uint8_t ebi_index;
	uint64_t imsi = UINT64_MAX;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	int if_ue_present = 0;

	memcpy(&imsi, imsi_val, imsi_len);

	*context = (ue_context_t *)ue_context_entry_lookup_imsiKey(imsi);

	if (*context == NULL) {
		(*context) = (ue_context_t *)calloc(1, sizeof(ue_context_t));
		if (*context == NULL) {
			LOG_MSG(LOG_ERROR, "Failure to allocate ue context ");
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		(*context)->imsi = imsi;
		(*context)->imsi_len = imsi_len;
		ret = ue_context_entry_add_imsiKey(imsi, *context); 
		if (ret < 0) {
			LOG_MSG(LOG_ERROR, "Error on imsi to ue_context add");
			free((*context));
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
	} else {
        // Requirement : Rule out its not retransmitted and 
        // Requirment : Rule out that its not 2nd PDN case 
        // for now both cases, return error 
#ifdef REQUIREMENT_MULTIPDN
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
		/*if ((strncmp(apn_requested->apn_name,
					(((*context)->pdns[ebi - 5]))->apn,
					sizeof(apn_requested->apn_name_length))) == 0) {
			LOG_MSG(LOG_ERROR,
				"Discarding re-transmitted csr received for IMSI:%lu ", imsi);
			return -1;
		}*/
#endif
        LOG_MSG(LOG_ERROR,
                "Context Replacement CSReq Received for IMSI:%lu ",
                imsi);
        return GTPV2C_CAUSE_REQUEST_REJECTED ; 
	}
	if (if_ue_present == 0){
		if (cp_config->cp_type == SAEGWC) {
			(*context)->s11_sgw_gtpc_teid = s11_sgw_gtpc_base_teid
			    + s11_sgw_gtpc_teid_offset;
			++s11_sgw_gtpc_teid_offset;

		} else if (cp_config->cp_type == PGWC){
			(*context)->s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid + s5s8_pgw_gtpc_teid_offset;
		}
	}else if (cp_config->cp_type == PGWC){
		(*context)->s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid + s5s8_pgw_gtpc_teid_offset;
	}

    TAILQ_INIT(&((*context)->pending_sub_procs));
	ret = ue_context_entry_add_teidKey((*context)->s11_sgw_gtpc_teid, (*context));

	if (ret < 0) {
		LOG_MSG(LOG_ERROR,
			"%s - Error on ue_context_by_fteid_hash add",
			strerror(ret));
		ue_context_delete_entry_imsiKey((*context)->imsi);
		if (ret < 0) {
			/* If we get here something bad happened. The
			 * context that was added to
			 * ue_context_by_imsi_hash above was not able
			 * to be removed.
			 */
            assert(0);
		}
		free((*context));
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
				free(pdn->eps_bearers[i]);
				pdn->eps_bearers[i] = NULL;
				(*context)->eps_bearers[i] = NULL;
				(*context)->bearer_bitmap &= ~(1 << ebi_index);
			}
		} else {
			/* created session is creating a default bearer in place */
			/* of a different pdn connection's dedicated bearer */
			bearer->pdn->eps_bearers[ebi_index] = NULL;
			bzero(bearer, sizeof(*bearer));
			pdn = (pdn_connection_t *)calloc(1, sizeof(pdn_connection_t));
			if (pdn == NULL) {
				LOG_MSG(LOG_ERROR, "Failure to allocate PDN ");
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
		bearer = (eps_bearer_t *)calloc(1, sizeof(eps_bearer_t));
		if (bearer == NULL) {
			LOG_MSG(LOG_ERROR, "Failure to allocate bearer ");
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		bearer->eps_bearer_id = ebi;
		pdn = (pdn_connection_t *)calloc(1, sizeof(pdn_connection_t));
		if (pdn == NULL) {
			LOG_MSG(LOG_ERROR, "Failure to allocate PDN ");
			return GTPV2C_CAUSE_SYSTEM_FAILURE;
		}
		pdn->num_bearer++;
		(*context)->eps_bearers[ebi_index] = bearer;
		(*context)->pdns[ebi_index] = pdn;
		(*context)->num_pdns++;
		(*context)->bearer_bitmap |= (1 << ebi_index);
		pdn->eps_bearers[ebi_index] = bearer;
		pdn->default_bearer_id = ebi;
        TAILQ_INIT(&(pdn->policy.pending_pcc_rules));
	}

	for (i = 0; i < MAX_FILTERS_PER_UE; ++i)
		bearer->packet_filter_map[i] = -ENOENT;


	bearer->pdn = pdn;
	bearer->eps_bearer_id = ebi;

	pdn = (*context)->pdns[ebi_index];
	bearer = (*context)->eps_bearers[ebi_index];

	ret = pdn_context_entry_add_teidKey((*context)->s11_sgw_gtpc_teid, pdn);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR,
			"%s - Error on pdn by fteid hash add",
			strerror(ret));
		pdn_context_delete_entry_teidKey((*context)->s11_sgw_gtpc_teid);
		if (ret < 0) {
			/* If we get here something bad happened. The
			 * context that was added to
			 * ue_context_by_imsi_hash above was not able
			 * to be removed.
			 */
            assert(0);
		}
		free((*context));
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}
	strcpy((char *)pdn->apn, (char *)apn);
	pdn->apn_len = apn_len;
    LOG_MSG(LOG_DEBUG,"Subscriber PDN created for apn = %s, length = %d ", pdn->apn, pdn->apn_len);
	return 0;
}


int 
cleanup_ue_context(ue_context_t **context_t)
{
    ue_context_t *context = *context_t;

    LOG_MSG(LOG_DEBUG, "Number of PDNS in UE %lu -  %d  ",context->imsi64, context->num_pdns);
    if(context->num_pdns == 0) {
        /* Delete UE context entry from UE Hash */
        if (ue_context_delete_entry_imsiKey(context->imsi) < 0){
            LOG_MSG(LOG_ERROR, "Error on ue_context_by_fteid_hash del");
        }

        /* delete context from user context */
        uint32_t temp_teid = context->s11_sgw_gtpc_teid;
        ue_context_delete_entry_teidKey(temp_teid);

        proc_context_t *proc = TAILQ_FIRST(&context->pending_sub_procs);
        while(proc != NULL) {
          TAILQ_REMOVE(&context->pending_sub_procs, proc, next_sub_proc);
          if(proc != NULL) {
              if(proc->gtpc_trans != NULL) {
                  LOG_MSG(LOG_DEBUG, "Delete gtpc procs ");
                  cleanup_gtpc_trans(proc->gtpc_trans);
              }
              if(proc->pfcp_trans != NULL) {
                  LOG_MSG(LOG_DEBUG, "Delete pfcp procs ");
                  cleanup_pfcp_trans(proc->pfcp_trans);
              }
              if(proc->gx_trans != NULL) {
                  LOG_MSG(LOG_DEBUG, "Delete gx procs ");
                  cleanup_pfcp_trans(proc->gx_trans);
              }
              free(proc);
          }
          proc = TAILQ_FIRST(&context->pending_sub_procs);
        }
 
        //Free UE context
        free(context);
        *context_t = NULL;
    }

    return 0;
}

int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context_t **context)
{
	eps_bearer_t *bearer = NULL;

	bearer = (eps_bearer_t *)get_bearer_by_teid(teid_key);
	if(bearer == NULL) {
		LOG_MSG(LOG_ERROR, "Bearer Entry not found for teid:%x...", teid_key);
        return -1;
	}
	if(bearer->pdn != NULL && bearer->pdn->context != NULL ) {
		*context = bearer->pdn->context;
		return 0;
	}
	return -1;
}

/* This function use only in clean up while error */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context_t **context)
{
	eps_bearer_t *bearer = NULL;
	/* If teid key is sgwc s11 */
	ue_context_t *temp_context = (ue_context_t*)get_ue_context(teid_key);
	if( temp_context == NULL) {
		/* If teid key is sgwc s5s8 */
		bearer = (eps_bearer_t *)get_bearer_by_teid(teid_key);
		if(bearer == NULL) {
			LOG_MSG(LOG_ERROR, "Bearer Entry not found for teid:%x...", teid_key);
			return -1;
		}
     	*context = bearer->pdn->context;
	} else {
        *context = temp_context;
    }
	return 0;
}

