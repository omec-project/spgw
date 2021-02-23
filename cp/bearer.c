// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp.h"
#include "cp_log.h"
#include "pfcp_cp_session.h"

void cleanup_bearer_context(eps_bearer_t *bearer)
{
    pdn_connection_t *pdn_ctxt = bearer->pdn;
    /* Delete rules those are associated with PDN  */
    /* REVIEW: Remove the hardcoded rules counter, use the dynamic counter to maintain the list*/
    for (uint8_t iCnt = 0; iCnt < 16; ++iCnt) {
        if (NULL != bearer->dynamic_rules[iCnt]) {
            rule_name_key_t key = {0};
            memcpy(&key.rule_name, bearer->dynamic_rules[iCnt]->rule_name, 255);
            sprintf(key.rule_name, "%s%d", key.rule_name, (bearer->pdn)->call_id);
            if (del_rule_name_entry(key.rule_name) != 0) {
                LOG_MSG(LOG_ERROR," Error on delete rule name entries %s ", key.rule_name);
            }
        }
    }
    uint64_t ebi_index = bearer->eps_bearer_id - 5;
    if (del_rule_entries(pdn_ctxt->context, ebi_index) != 0) {
        LOG_MSG(LOG_ERROR, "Error on delete rule entries");
    }
    pdn_ctxt->eps_bearers[ebi_index] = NULL;
    pdn_ctxt->context->eps_bearers[ebi_index] = NULL;
    free(bearer);
    return;
}

int
del_rule_entries(ue_context_t *context, uint8_t ebi_index)
{
    pdr_t *pdr_ctx =  NULL;

    /*Delete all pdr, far, qer entry from table */
    for(uint8_t itr = 0; itr < context->eps_bearers[ebi_index]->qer_count ; itr++) {
        if( del_qer_entry(context->eps_bearers[ebi_index]->qer_id[itr].qer_id) != 0 ){
            LOG_MSG(LOG_ERROR, "del_qer_entry deletion");
        }
    }

    for(uint8_t itr = 0; itr < context->eps_bearers[ebi_index]->pdr_count ; itr++) {
        pdr_ctx = context->eps_bearers[ebi_index]->pdrs[itr];
        if(pdr_ctx == NULL) {
            LOG_MSG(LOG_ERROR, "no pdr entry ");
            continue;
        }
        if( del_pdr_entry(context->eps_bearers[ebi_index]->pdrs[itr]->rule_id) != 0 ){
            LOG_MSG(LOG_ERROR, "del_pdr_entry deletion");
        }
    }
    return 0;
}
