// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <rte_errno.h>
#include "ue.h"
#include "cp_interface.h"
#include "clogger.h"
#include "cp_config.h"
#include "tables/tables.h"
#include "util.h"
#include "pfcp_cp_session.h"
#include "gen_utils.h"
#include "gtpv2_session.h"
#include "spgw_cpp_wrapper.h"

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

// Requirement: Understand how this teid range works
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
	ret = bearer_context_entry_add_teidKey(fteid_key, (*bearer));
	
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
		uint8_t ebi, ue_context_t **context, apn_profile_t *apn_requested,
	  	uint32_t sequence)
{
    RTE_SET_USED(apn_requested);
    RTE_SET_USED(sequence);
	int ret;
	int i;
	uint8_t ebi_index;
	uint64_t imsi = UINT64_MAX;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	int if_ue_present = 0;

	memcpy(&imsi, imsi_val, imsi_len);

	ret = ue_context_entry_lookup_imsiKey(imsi, &(*context));

	if (ret == -1) {
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
		ret = ue_context_entry_add_imsiKey(*context); 
		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
			rte_free((*context));
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
					(((*context)->pdns[ebi - 5])->apn_in_use)->apn_name,
					sizeof(apn_requested->apn_name_length))) == 0) {
			clLog(clSystemLog, eCLSeverityCritical,
				"%s- Discarding re-transmitted csr received for IMSI:%lu \n",
				__func__, imsi);
			return -1;
		}*/
#endif
        clLog(clSystemLog, eCLSeverityCritical,
                "%s- Context Replacement CSReq Received for IMSI:%lu \n",
                __func__, imsi);
        return GTPV2C_CAUSE_REQUEST_REJECTED ; 
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

    TAILQ_INIT(&((*context)->pending_sub_procs));
	ret = ue_context_entry_add_teidKey((*context)->s11_sgw_gtpc_teid, (*context));

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
			"%s - Error on ue_context_by_fteid_hash add\n",
			strerror(ret));
		ue_context_delete_entry_imsiKey((*context)->imsi);
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

	ret = pdn_context_entry_add_teidKey((*context)->s11_sgw_gtpc_teid, pdn);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
			"%s - Error on pdn by fteid hash add\n",
			strerror(ret));
		pdn_context_delete_entry_teidKey((*context)->s11_sgw_gtpc_teid);
		if (ret < 0) {
			/* If we get here something bad happened. The
			 * context that was added to
			 * ue_context_by_imsi_hash above was not able
			 * to be removed.
			 */
			rte_panic("%s - Error on "
				"pdn by fteid hash del\n",
				strerror(ret));
		}
		rte_free((*context));
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}
	pdn->apn_in_use = apn_requested;

	return 0;
}

/* We should queue the event if required */
void
start_procedure(proc_context_t *proc_ctxt, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    context = (ue_context_t *)proc_ctxt->ue_context;

    if(context != NULL) {
        assert(TAILQ_EMPTY(&context->pending_sub_procs)); // empty 
        TAILQ_INSERT_TAIL(&context->pending_sub_procs, proc_ctxt, next_sub_proc); 
        assert(!TAILQ_EMPTY(&context->pending_sub_procs)); // not empty
        /* Decide if we wish to run procedure now... If yes move on ...*/
        proc_ctxt = TAILQ_FIRST(&context->pending_sub_procs);
    }
    assert(proc_ctxt != NULL);
    printf("procedure number = %d \n",proc_ctxt->proc_type);
    switch(proc_ctxt->proc_type) {
        case INITIAL_PDN_ATTACH_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        } 

        case S1_RELEASE_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = proc_ctxt->ue_context;
            context->state = UE_STATE_IDLE_PENDING;
            pdn_connection_t *pdn = proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_IDLE_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case SERVICE_REQUEST_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case DETACH_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = proc_ctxt->ue_context;
            context->state = UE_STATE_DETACH_PENDING;
            pdn_connection_t *pdn = proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_DETACH_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case PAGING_PROC: { 
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = proc_ctxt->ue_context;
            context->state = UE_STATE_PAGING_PENDING;
            pdn_connection_t *pdn = proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_PAGING_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }
    }
    return;
}

static int cleanup_pdn(pdn_connection_t *pdn, ue_context_t **context_t)
{
    uint64_t sess_id = pdn->seid;
	uint64_t ebi_index = pdn->default_bearer_id - 5;
    ue_context_t *context = pdn->context;
    int ret;

	/* Delete entry from session entry */
    printf("Delete session from the pfcp seid table \n");
	if (del_sess_entry_seid(sess_id) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for Key sess ID:%lu\n",
				__func__, __LINE__, sess_id);
		return -1;
	}

	if (del_rule_entries(context, ebi_index) != 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on delete rule entries\n",__file__);
	}
	uint32_t teid = UE_SESS_ID(sess_id);
	ret = delete_sgwc_context(teid, &context, &sess_id);
	if (ret)
		return ret;
    printf("%s %d : number of PDNS = %d  \n",__FUNCTION__, __LINE__,context->num_pdns);
    if(context->num_pdns == 0) {
        /* Delete UE context entry from UE Hash */
        if (ue_context_delete_entry_imsiKey(context->imsi) < 0){
            clLog(clSystemLog, eCLSeverityCritical,
                    "%s %s - Error on ue_context_by_fteid_hash del\n",__file__,
                    strerror(ret));
        }

        /* delete context from user context */
        uint32_t temp_teid = context->s11_sgw_gtpc_teid;
        ue_context_delete_entry_teidKey(temp_teid);


        /* Delete UPFList entry from UPF Hash */
        if ((context->dns_enable && upflist_by_ue_hash_entry_delete(&context->imsi, sizeof(context->imsi))) < 0){
            clLog(clSystemLog, eCLSeverityCritical,
                    "%s %s - Error on upflist_by_ue_hash deletion of IMSI \n",__file__,
                    strerror(ret));
        }

#ifdef USE_CSID
        
        fqcsid_t *csids

        if (cp_config->cp_type == PGWC) {
            csids  = context->pgw_fqcsid;
        } else {
            csids  = context->sgw_fqcsid;
        }

        /* Get the session ID by csid */
        for (uint16_t itr = 0; itr < csids->num_csid; itr++) {
            sess_csid *tmp = NULL;

            tmp = get_sess_csid_entry(csids->local_csid[itr]);
            if (tmp == NULL)
                continue;

            /* VS: Delete sess id from csid table */
            for(uint16_t cnt = 0; cnt < tmp->seid_cnt; cnt++) {
                if (sess_id == tmp->cp_seid[cnt]) {
                    for(uint16_t pos = cnt; pos < (tmp->seid_cnt - 1); pos++ )
                        tmp->cp_seid[pos] = tmp->cp_seid[pos + 1];

                    tmp->seid_cnt--;
                    clLog(clSystemLog, eCLSeverityDebug, "Session Deleted from csid table sid:%lu\n",
                            sess_id);
                }
            }

            if (tmp->seid_cnt == 0) {
                /* Cleanup Internal data structures */
                ret = del_peer_csid_entry(&csids->local_csid[itr], S5S8_PGWC_PORT_ID);
                if (ret) {
                    clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                            strerror(errno));
                    return -1;
                }

                /* Clean MME FQ-CSID */
                if (context->mme_fqcsid != 0) {
                    ret = del_peer_csid_entry(&(context->mme_fqcsid)->local_csid[itr], S5S8_PGWC_PORT_ID);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                strerror(errno));
                        return -1;
                    }
                    if (!(context->mme_fqcsid)->num_csid)
                        rte_free(context->mme_fqcsid);
                }

                /* Clean UP FQ-CSID */
                if (context->up_fqcsid != 0) {
                    ret = del_peer_csid_entry(&(context->up_fqcsid)->local_csid[itr],
                            SX_PORT_ID);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                strerror(errno));
                        return -1;
                    }
                    if (!(context->up_fqcsid)->num_csid)
                        rte_free(context->up_fqcsid);
                }
            }

        }

#endif /* USE_CSID */
        //Free UE context
        rte_free(context);
        *context_t = NULL;
    }

    return 0;
}

void
end_procedure(proc_context_t *proc_ctxt) 
{
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

    assert(proc_ctxt != NULL);

    if(proc_ctxt->gtpc_trans != NULL) {
        transData_t *trans = proc_ctxt->gtpc_trans;

        uint16_t port_num = trans->peer_sockaddr.sin_port; 
        uint32_t sender_addr = trans->peer_sockaddr.sin_addr.s_addr; 
        uint32_t seq_num = trans->sequence; 

        transData_t *gtpc_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
        assert(gtpc_trans != NULL);

        /* Let's cross check if transaction from the table is matchig with the one we have 
         * in subscriber 
         */
        assert(gtpc_trans == trans);
        proc_ctxt->gtpc_trans =  NULL;
        free(gtpc_trans);
    }
    if(proc_ctxt->pfcp_trans != NULL) {
        transData_t *pfcp_trans = proc_ctxt->pfcp_trans;
        uint16_t port_num = pfcp_trans->peer_sockaddr.sin_port; 
        uint32_t sender_addr = pfcp_trans->peer_sockaddr.sin_addr.s_addr; 
        uint32_t seq_num = pfcp_trans->sequence; 

        transData_t *temp = delete_gtp_transaction(sender_addr, port_num, seq_num);
        if(temp != NULL) {
            /* Let's cross check if transaction from the table is matchig with the one we have 
            * in subscriber 
            */
            assert(pfcp_trans == temp);
        }
        free(pfcp_trans);
        proc_ctxt->pfcp_trans = NULL;
    }

    context = proc_ctxt->ue_context;
    if(context != NULL) {
        TAILQ_REMOVE(&context->pending_sub_procs, proc_ctxt, next_sub_proc);
        assert(TAILQ_EMPTY(&context->pending_sub_procs)); // empty
    }
    switch(proc_ctxt->proc_type) {
        case INITIAL_PDN_ATTACH_PROC: {
            context = proc_ctxt->ue_context;
            pdn = proc_ctxt->pdn_context;
            if(context != NULL && proc_ctxt->result == PROC_RESULT_SUCCESS) {
                context->state = UE_STATE_ACTIVE;
                assert(pdn != NULL);
                pdn->state = PDN_STATE_ACTIVE;
            } else {
                cleanup_pdn(pdn, &context);
            }
            free(proc_ctxt);
            proc_ctxt = NULL;
            break;
        }
        case S1_RELEASE_PROC: {
            context = proc_ctxt->ue_context;
            context->state = UE_STATE_IDLE;
            pdn = proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_IDLE;
            free(proc_ctxt);
            break;
        }
        case SERVICE_REQUEST_PROC: {
            // No state change 
            free(proc_ctxt);
            context = proc_ctxt->ue_context;
            break;
        }
        case DETACH_PROC: {
            context = proc_ctxt->ue_context;
            assert(context  != NULL);
            context->state = UE_STATE_DETACH;
            pdn = proc_ctxt->pdn_context;
            assert(pdn != NULL);
            pdn->state = PDN_STATE_DETACH;
            free(proc_ctxt);
            cleanup_pdn(pdn, &context);
            break;
        }
        case PAGING_PROC : {
            context = proc_ctxt->ue_context;
            context->state = UE_STATE_ACTIVE;
            pdn = proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_ACTIVE;
            free(proc_ctxt);
            break;
        }
    }

    if(context != NULL) {
        // start new procedure if something is pending 
        printf("Continue with subscriber \n");
    } else {
        printf("Released subscriber \n");
    }
    // result_success - schedule next proc
    // result_fail_call_del - abort all outstanding procs and delete the call
    // result_fail_keep_call - continue with call as is and schedule new proc if any  
    return;
}
