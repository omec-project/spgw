// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef PFCP_CP_ASSOC_H
#define PFCP_CP_ASSOC_H

#include "pfcp_messages.h"
#include "sm_struct.h"
#include "upf_struct.h"
#include "proc_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  : This function processes pfcp associatiuon response
 * @param  : msg hold the data from pfcp associatiuon response
 * @param  : peer_addr denotes address of peer node
 * @return : Returns 0 in case of success else negative value
 */
uint8_t
process_pfcp_ass_resp(msg_info_t *msg, struct sockaddr_in *peer_addr);

/**
 * @brief  : fills default rule and qos values
 * @param  : pdn
 * @return : Returns nothing
 */
void
fill_rule_and_qos_inform_in_pdn(pdn_connection_t *pdn);


/**
 * @brief  : This is a function to fill pfcp association update response
 * @param  : pfcp_asso_update_resp is pointer to structure of pfcp association update response
 * @return : This function dose not return anything
 */
void
fill_pfcp_association_update_resp(pfcp_assn_upd_rsp_t *pfcp_asso_update_resp);

/**
 * @brief  : This is a function to fill pfcp association update request
 * @param  : pfcp_asso_update_req is pointer to structure of pfcp association update request
 * @return : This function dose not return anything
 */
void
fill_pfcp_association_update_req(pfcp_assn_upd_req_t *pfcp_ass_update_req);

/**
 * @brief  : This is a function to fill pfcp association setup response
 * @param  : pfcp_asso_setup_resp is pointer to structure of pfcp association setup response
 * @param  : caues describes the whether request is accepted or not
 * @return : This function dose not return anything
 */
void
fill_pfcp_association_setup_resp(pfcp_assn_setup_rsp_t *pfcp_ass_setup_resp, uint8_t cause);

/**
 * @brief  : This is a function to fill pfcp association release request
 * @param  : pfcp_asso_rel_req is pointer to structure of pfcp association release request
 * @return : This function dose not return anything
 */
void
fill_pfcp_association_release_req(pfcp_assn_rel_req_t *pfcp_ass_rel_req);

/**
 * @brief  : This is a function to fill pfcp association release response
 * @param  : pfcp_asso_rel_resp is pointer to structure of pfcp association release response
 * @return : This function dose not return anything
 */
void
fill_pfcp_association_release_resp(pfcp_assn_rel_rsp_t *pfcp_ass_rel_resp);

/**
 * @brief  : This is a function to fill pfcp node report request
 * @param  : pfcp_node_rep_req is pointer to structure of pfcp node report request
 * @return : This function dose not return anything
 */
void
fill_pfcp_node_report_req(pfcp_node_rpt_req_t *pfcp_node_rep_req);

/**
 * @brief  : This is a function to fill pfcp node report response
 * @param  : pfcp_node_rep_resp is pointer to structure of pfcp node report response
 * @return : This function dose not return anything
 */
void
fill_pfcp_node_report_resp(pfcp_node_rpt_rsp_t *pfcp_node_rep_resp);

/**
 * @brief  : This is a function to fill pfcp pfd management response
 * @param  : pfd_resp is pointer to structure of pfcp pfd management response
 * @param  : cause_id describes cause if requested or not
 * @param  : offending_ie describes IE due which request got rejected if any
 * @return : This function dose not return anything
 */
void
fill_pfcp_pfd_mgmt_resp(pfcp_pfd_mgmt_rsp_t *pfd_resp, uint8_t cause_id, int offending_ie);

/**
 * @brief  : This is a function to fill pfcp heartbeat request
 * @param  : pfcp_heartbeat_req is pointer to structure of pfcp heartbeat request
 * @param  : seq indicates the sequence number
 * @return : This function dose not return anything
 */
void
fill_pfcp_heartbeat_req(pfcp_hrtbeat_req_t *pfcp_heartbeat_req, uint32_t seq);

/**
 * @brief  : Process pfcp heartbeat request
 * @param  : peer_addr, peer node address
 * @param  : seq, sequence number
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_heartbeat_req(struct sockaddr_in *peer_addr, uint32_t seq);

#ifdef __cplusplus
}
#endif
#endif /* PFCP_ASSOC_H */
