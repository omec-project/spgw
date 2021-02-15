// Copyright (c) 2019 Sprint
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_encoder.h"
#include "gtpv2_session.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_internal.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_session.h" // ajay - check_interface_type - this should be part of pfcp interface 
#include "pfcp_enum.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h" // ajay should be part of interface 
#include "pfcp_cp_session.h"
#include "gtpv2_set_ie.h"
#include "cp_peer.h"
#include "gen_utils.h"
#include "sm_structs_api.h"
#include "pfcp.h"
#include "cp_transactions.h"
#include "tables/tables.h"
#include "cp_io_poll.h"
#include "cp_log.h"
#include "assert.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

/**
 * @brief  : Delete pgwc context
 * @param  : ds_req, hold information from delete session request
 * @param  : context, ue context data
 * @param  : resp, response structure to be filled
 * @return : Returns 0 in case of success , different error codes otherwise
 */
#ifdef FUTURE_NEED
static int
delete_pgwc_context(del_sess_req_t *ds_req, ue_context_t **_context,
		struct gw_info *resp)
{
	int ret = 0, i = 0;
	uint8_t ebi = 0;
	ue_context_t *context = NULL;
	static uint32_t process_pgwc_s5s8_ds_req_cnt;

	ret = get_ue_context(ds_req->header.teid.has_teid.teid, &context);
	if (ret < 0 || !context) {

		LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
				"\n\tprocess_pgwc_s5s8_delete_session_request:"
				"\n\tdelete_pgwc_context-ERROR!!!"
				"\n\tprocess_pgwc_s5s8_ds_req_cnt= %u;"
				"\n\tgtpv2c_s5s8_rx->teid_u.has_teid.teid= %X;"
				"\n\trte_hash_lookup_data("
				"ue_context_by_fteid_hash,..)= %d",
				process_pgwc_s5s8_ds_req_cnt++,
				ds_req->header.teid.has_teid.teid,
				ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/** TODO: we should verify mandatory fields within received message */
	if(ds_req->lbi.header.type == GTP_IE_EPS_BEARER_ID){
		if(ds_req->lbi.header.instance == IE_INSTANCE_ZERO){
			ebi = ds_req->lbi.ebi_ebi;
		}
	}

	if(ds_req->uli.header.type == GTP_IE_USER_LOC_INFO){
		if(ds_req->uli.header.instance == IE_INSTANCE_ZERO){
			/**/
		}
	}

	if(!ebi) {
		/* TODO: should be responding with response indicating error
		 * 		 * in request */
		LOG_MSG(LOG_ERROR, "Received delete session without ebi! - dropping");
		return -EPERM;
	}

	resp->eps_bearer_id = ebi ;
	/* VS: Fill the eps bearer id in response */

	uint8_t ebi_index = ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR,
				"Received delete session on non-existent EBI - "
				"Dropping packet");
		/*LOG_MSG(LOG_ERROR, "ebi %u",
		 * 		    *IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t, ebi_ei_to_be_removed));*/
		LOG_MSG(LOG_ERROR, "ebi %u", ebi);
		LOG_MSG(LOG_ERROR, "ebi_index %u", ebi_index);
		LOG_MSG(LOG_ERROR, "bearer_bitmap %04x", context->bearer_bitmap);
		LOG_MSG(LOG_ERROR, "mask %04x", (1 << ebi_index));
		return -EPERM;
	}

	pdn_connection_t *pdn = context->eps_bearers[ebi_index]->pdn;
	resp->seid = context->pdns[ebi_index]->seid;  //NK:change for seid
	if (!pdn) {
		LOG_MSG(LOG_ERROR, "Received delete session on non-existent EBI");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	if (pdn->default_bearer_id != ebi) {
		LOG_MSG(LOG_ERROR,
				"Received delete session referencing incorrect default bearer ebi");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}
	/* s11_sgw_gtpc_teid= s5s8_sgw_gtpc_teid =
	 * 	 * key->ue_context_by_fteid_hash */
	resp->s5s8_sgw_gtpc_teid = pdn->s5s8_sgw_gtpc_teid;
	resp->s5s8_pgw_gtpc_ipv4 = pdn->s5s8_sgw_gtpc_ipv4.s_addr;

	LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
			" delete_pgwc_context(...);"
			" process_pgwc_s5s8_ds_req_cnt= %u;"
			" ue_ip= pdn->ipv4= %s;"
			" pdn->s5s8_sgw_gtpc_ipv4= %s;"
			" pdn->s5s8_sgw_gtpc_teid= %X;"
			" pdn->s5s8_pgw_gtpc_ipv4= %s;"
			" pdn->s5s8_pgw_gtpc_teid= %X;"
			" rte_hash_lookup_data("
			"ue_context_by_fteid_hash,..)= %d",
			process_pgwc_s5s8_ds_req_cnt++,
			inet_ntoa(pdn->ipv4),
			inet_ntoa(pdn->s5s8_sgw_gtpc_ipv4),
			pdn->s5s8_sgw_gtpc_teid,
			inet_ntoa(pdn->s5s8_pgw_gtpc_ipv4),
			pdn->s5s8_pgw_gtpc_teid,
			ret);

	eps_bearer_t *bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		LOG_MSG(LOG_ERROR, "Received delete session on non-existent "
				"default EBI");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	for (i = 0; i < MAX_BEARERS; ++i) {
		if (pdn->eps_bearers[i] == NULL)
			continue;

		if (context->eps_bearers[i] == pdn->eps_bearers[i]) {
			bearer = context->eps_bearers[i];
			struct session_info si;
			memset(&si, 0, sizeof(si));

			/**
			  * ebi and s1u_sgw_teid is set here for zmq/sdn
			 */
			si.bearer_id = ebi;
			si.ue_addr.u.ipv4_addr =
				htonl(pdn->ipv4.s_addr);
			si.ul_s1_info.sgw_teid =
				bearer->s1u_sgw_gtpu_teid;
			si.sess_id = SESS_ID(
					context->s11_sgw_gtpc_teid,
					si.bearer_id);
			/* Delete rules those are associated with PDN  */
			/* REVIEW: Remove the hardcoded rules counter, use the dynamic counter to maintain the list*/
			for (uint8_t iCnt = 0; iCnt < 16; ++iCnt) {
				if (NULL != bearer->dynamic_rules[iCnt]) {
					rule_name_key_t key = {0};
					memcpy(&key.rule_name, bearer->dynamic_rules[iCnt]->rule_name,
						255);
					sprintf(key.rule_name, "%s%d", key.rule_name, (bearer->pdn)->call_id);
					if (del_rule_name_entry(key) != 0) {
						LOG_MSG(LOG_ERROR,"Error on delete rule name  %s ", key.rule_name);
					}
				}
			}
			rte_free(pdn->eps_bearers[i]);
			pdn->eps_bearers[i] = NULL;
			context->eps_bearers[i] = NULL;
			context->bearer_bitmap &= ~(1 << i);
		} else {
            assert(0);
		}
	}
	--context->num_pdns;
	rte_free(pdn);
	context->pdns[ebi_index] = NULL;
	context->teid_bitmap = 0;

	*_context = context;
	return 0;
}
#endif

int
delete_sgwc_context(uint32_t gtpv2c_teid, ue_context_t **_context, uint64_t *seid)
{
	int i = 0;
	//int ret;
	pdn_connection_t *pdn_ctxt = NULL;
	int ebi_index = UE_BEAR_ID(*seid) - 5;
	pdn_ctxt = (*_context)->pdns[ebi_index];
	for (i = 0; i < MAX_BEARERS; ++i) {
		if (pdn_ctxt->eps_bearers[i]) {
			eps_bearer_t *bearer = pdn_ctxt->eps_bearers[i];
			struct session_info si;
			memset(&si, 0, sizeof(si));

			/**
			 * ebi and s1u_sgw_teid is set here for zmq/sdn
			 */
			si.bearer_id = i + 5;
			si.ue_addr.u.ipv4_addr =
				htonl(pdn_ctxt->ipv4.s_addr);
			si.ul_s1_info.sgw_teid =
				bearer->s1u_sgw_gtpu_teid;
			si.sess_id = SESS_ID(
					pdn_ctxt->context->s11_sgw_gtpc_teid,
					si.bearer_id);
			*seid = si.sess_id;

			/* Delete rules those are associated with PDN  */
			/* REVIEW: Remove the hardcoded rules counter, use the dynamic counter to maintain the list*/
			for (uint8_t iCnt = 0; iCnt < 16; ++iCnt) {
				if (NULL != bearer->dynamic_rules[iCnt]) {
					rule_name_key_t key = {0};
					memcpy(&key.rule_name, bearer->dynamic_rules[iCnt]->rule_name,
						255);
					sprintf(key.rule_name, "%s%d", key.rule_name, (bearer->pdn)->call_id);
					if (del_rule_name_entry(key) != 0) {
						LOG_MSG(LOG_ERROR," Error on delete rule name entries %s ", key.rule_name);
					}
				}
			}

			rte_free(pdn_ctxt->eps_bearers[i]);
			pdn_ctxt->eps_bearers[i] = NULL;
			pdn_ctxt->context->eps_bearers[i] = NULL;
			pdn_ctxt->context->pdns[i] = NULL;
			pdn_ctxt->context->bearer_bitmap &= ~(1 << i);
		}
	}

	--pdn_ctxt->context->num_pdns;
	pdn_ctxt->context->teid_bitmap = 0;

    pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_ctxt->policy.pending_pcc_rules);
    while (pcc_rule != NULL) {
        TAILQ_REMOVE(&pdn_ctxt->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
        free(pcc_rule->dyn_rule);
        free(pcc_rule);
        pcc_rule = TAILQ_FIRST(&pdn_ctxt->policy.pending_pcc_rules);
    }

	//*_context = pdn_ctxt->context;
	rte_free(pdn_ctxt);
	RTE_SET_USED(gtpv2c_teid);
	return 0;
}

