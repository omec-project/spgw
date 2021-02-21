// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "ue.h"
#include "spgw_cpp_wrapper.h"
#include "cp_log.h"
#include "ip_pool.h"

void cleanup_pdn_context(pdn_connection_t *pdn_ctxt)
{
    int i = 0;
    uint64_t sess_id = pdn_ctxt->seid;
    uint32_t teid = UE_SESS_ID(sess_id);

    assert(pdn_ctxt->context->s11_sgw_gtpc_teid == teid);

    for (i = 0; i < MAX_BEARERS; ++i) {
        if (pdn_ctxt->eps_bearers[i]) {
            eps_bearer_t *bearer = pdn_ctxt->eps_bearers[i];
            cleanup_bearer_context(bearer);
            pdn_ctxt->context->pdns[i] = NULL;
            pdn_ctxt->context->bearer_bitmap &= ~(1 << i);
        }
    }

    pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_ctxt->policy.pending_pcc_rules);
    while (pcc_rule != NULL) {
        TAILQ_REMOVE(&pdn_ctxt->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
        free(pcc_rule->dyn_rule);
        free(pcc_rule);
        pcc_rule = TAILQ_FIRST(&pdn_ctxt->policy.pending_pcc_rules);
    }

    /* Delete entry from session entry */
    LOG_MSG(LOG_DEBUG4, "Delete session from the pfcp seid table ");
    if (del_sess_entry_seid(pdn_ctxt->seid) != 0) {
        LOG_MSG(LOG_ERROR, "NO Session Entry Found for Key sess ID:%lu", pdn_ctxt->seid);
    }
    // Improve this to handle Pure P & collapsed call 
    pdn_context_delete_entry_teidKey(pdn_ctxt->context->s11_sgw_gtpc_teid);

    --pdn_ctxt->context->num_pdns;
	pdn_ctxt->context->teid_bitmap = 0;
    release_ip(pdn_ctxt->ipv4);
	free(pdn_ctxt);
    return;
}
