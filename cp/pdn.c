// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "ue.h"
#include "spgw_cpp_wrapper.h"
#include "cp_log.h"
#include "ip_pool.h"

eps_bearer_t* 
get_default_bearer(pdn_connection_t *pdn)
{
	return pdn->eps_bearers[pdn->default_bearer_id - 5];

}

eps_bearer_t* 
get_bearer(pdn_connection_t *pdn, bearer_qos_ie *qos)
{
	eps_bearer_t *bearer = NULL;
    LOG_MSG(LOG_DEBUG, "New rule - Qoc qci %d , arp-vulner %d , arp-level %d , arp-capability %d ",qos->qci, qos->arp.preemption_vulnerability, qos->arp.priority_level, qos->arp.preemption_capability);
	for(uint8_t idx = 0; idx < MAX_BEARERS; idx++)
	{
		bearer = pdn->eps_bearers[idx];
		if(bearer != NULL)
		{
            LOG_MSG(LOG_DEBUG,"Bearer checking with bearer id %d ", bearer->eps_bearer_id);
            LOG_MSG(LOG_DEBUG,"Existing bearer qci - %d , arp-vulner %d , arp-level %d , arp-capability %d ", 
                               bearer->qos.qci, bearer->qos.arp.preemption_vulnerability, 
                               bearer->qos.arp.priority_level, bearer->qos.arp.preemption_capability);
			/* Comparing each member in arp */
			if((bearer->qos.qci == qos->qci) &&
				(bearer->qos.arp.preemption_vulnerability == qos->arp.preemption_vulnerability) &&
				(bearer->qos.arp.priority_level == qos->arp.priority_level) &&
				(bearer->qos.arp.preemption_capability == qos->arp.preemption_capability))
			{
                LOG_MSG(LOG_DEBUG,"Matching bearer found %d ",bearer->eps_bearer_id); 
				return bearer;
			}
		}

	}
    LOG_MSG(LOG_DEBUG, "No Matcing bearer for qci/arp combination ");
	return NULL;
}

int8_t
compare_default_bearer_qos(bearer_qos_ie *default_bearer_qos,
					 bearer_qos_ie *rule_qos)
{
	if(default_bearer_qos->qci != rule_qos->qci)
		return -1;

	if(default_bearer_qos->arp.preemption_vulnerability != rule_qos->arp.preemption_vulnerability)
		return -1;

	if(default_bearer_qos->arp.priority_level != rule_qos->arp.priority_level)
		return -1;

	if(default_bearer_qos->arp.preemption_vulnerability != rule_qos->arp.preemption_vulnerability)
		return -1;

	return 0;

}

void 
cleanup_pdn_context(pdn_connection_t *pdn_ctxt)
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
    if(IF_PDN_ADDR_STATIC(pdn_ctxt)) {
        release_ip_node(pdn_ctxt->ipv4);
    } else {
        release_ip(pdn_ctxt->ipv4);
    }
	free(pdn_ctxt);
    return;
}

int8_t
get_new_bearer_id(pdn_connection_t *pdn_cntxt)
{
	return pdn_cntxt->num_bearer;
}
