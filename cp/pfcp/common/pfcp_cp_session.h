// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef PFCP_CP_SESSION_H
#define PFCP_CP_SESSION_H

#include "pfcp_messages.h"
#include "sm_struct.h"
#include "pfcp_cp_set_ie.h"
#include "gtp_messages.h"
#include "ue.h"
#include "pdn.h"
#include "bearer.h"
#include "policy.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Fill pfcp session establishment response
 * @param  : pfcp_sess_est_resp , structure to be filled
 * @param  : cause , cause whether request is accepted or not
 * @param  : offend , Offending ie type
 * @param  : dp_comm_ip , ip address of dp
 * @param  : pfcp_session_request, pfcp session establishment request data
 * @return : Returns nothing
 */
void
fill_pfcp_session_est_resp(pfcp_sess_estab_rsp_t
				*pfcp_sess_est_resp, uint8_t cause, int offend,
				struct in_addr dp_comm_ip,
				struct pfcp_sess_estab_req_t *pfcp_session_request);

/**
 * @brief  : Fill pfcp session delete response
 * @param  : pfcp_sess_del_resp , structure to be filled
 * @param  : cause , cause whether request is accepted or not
 * @param  : offend , Offending ie type
 * @return : Returns nothing
 */
void
fill_pfcp_sess_del_resp(pfcp_sess_del_rsp_t
			*pfcp_sess_del_resp, uint8_t cause, int offend);

/**
 * @brief  : Fill pfcp session modify response
 * @param  : pfcp_sess_modify_resp , structure to be filled
 * @param  : pfcp_session_mod_req , pfcp session modify request data
 * @param  : cause , cause whether request is accepted or not
 * @param  : offend , Offending ie type
 * @return : Returns nothing
 */
void
fill_pfcp_session_modify_resp(pfcp_sess_mod_rsp_t *pfcp_sess_modify_resp,
		pfcp_sess_mod_req_t *pfcp_session_mod_req, uint8_t cause, int offend);

/**
 * @brief  : Fill pfcp session establishment request
 * @param  : pfcp_sess_est_req , structure to be filled
 * @param  : context , pointer to ue context structure
 * @param  : ebi_index, index of bearer in array
 * @param  : seq, sequence number of request
 * @return : Returns nothing
 */
void
fill_pfcp_sess_est_req( pfcp_sess_estab_req_t *pfcp_sess_est_req,
		pdn_connection_t *pdn, uint32_t seq);

/**
 * @brief  : Checks and returns interface type if it access or core
 * @param  : iface , interface type
 * @retrun : Returns interface type in case of success , -1 otherwise
 */
int
check_interface_type(uint8_t iface);

/**
 * @brief  : Fill pfcp session modification request
 * @param  : pfcp_sess_mod_req , structure to be filled
 * @param  : header, holds info in gtpv2c header
 * @param  : bearer, pointer to bearer structure
 * @param  : pdn , pdn information
 * @param  : update_far ,  array of update far rules
 * @param  : handover_flag ,  flag to check if it is handover scenario or not
 * @return : Returns nothing
 */
uint32_t
fill_pfcp_sess_mod_req( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, eps_bearer_t *bearer,
		pdn_connection_t *pdn, pfcp_update_far_ie_t update_far[],  uint8_t handover_flag);

/**
 * @brief  : Fill pfcp session modification request for delete session request
 * @param  : pfcp_sess_mod_req , structure to be filled
 * @param  : header, holds info in gtpv2c header
 * @param  : context , pointer to ue context structure
 * @param  : pdn , pdn information
 * @return : Returns nothing
 */
void
fill_pfcp_sess_mod_req_pgw_init_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr);

void
fill_pfcp_sess_mod_req_pgw_init_remove_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr);


void
fill_pfcp_sess_mod_req_pgw_del_cmd_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr);

/**
 * @brief  : Process pfcp session modification response
 * @param  : sess_id, session id
 * @param  : gtpv2c_tx, holds info in gtpv2c header
 * @retrun : Returns 0 in case of success
 */
uint8_t
process_pfcp_sess_mod_resp(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx);

uint8_t
process_delete_bearer_pfcp_sess_response(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx);

int
process_pfcp_sess_mod_resp_del_cmd(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx, uint8_t *flag);

int
process_sess_mod_req_del_cmd(pdn_connection_t *pdn);


/**
 * @brief  : Fill pdr entry
 * @param  : context , pointer to ue context structure
 * @param  : pdn , pdn information
 * @param  : bearer, pointer to bearer structure
 * @param  : iface , interface type access or core
 * @param  : itr, index in pdr array stored in bearer context
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
process_delete_bearer_cmd_request(del_bearer_cmd_t *del_bearer_cmd, gtpv2c_header_t *gtpv2c_tx);

pdr_t*
fill_pdr_entry(ue_context_t *context, pdn_connection_t *pdn,
		eps_bearer_t *bearer, uint8_t iface, uint8_t itr, uint32_t qer_id, uint32_t urr_id);

/**
 * @brief  : Fill qer entry
 * @param  : pdn , pdn information
 * @param  : bearer, pointer to bearer structure
 * @param  : itr, index in qer array stored in bearer context
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
fill_qer_entry(pdn_connection_t *pdn, eps_bearer_t *bearer, uint32_t qer_id, bool apnAmbr);

/**
 * @brief  : Process pfcp delete session response
 * @param  : sess_id, session id
 * @param  : gtpv2c_tx, holds info in gtpv2c header
 * @param  : ccr_request, structure to be filled for ccr request
 * @param  : msglen, total length
 * @retrun : Returns 0 in case of success
 */
int8_t
process_pfcp_sess_del_resp(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx,
		gx_msg *ccr_request, uint16_t *msglen, proc_context_t *proc);

/**
 * @brief  : fill create session response on PGWC
 * @param  : cs_resp, structure to be filled
 * @param  : sequence, seq number of request
 * @param  : context , pointer to ue context structure
 * @param  : ebi_index, index of bearer in array
 * @return : Returns nothing
 */
void
fill_pgwc_create_session_response(create_sess_rsp_t *cs_resp,
				uint32_t sequence, ue_context_t *context, uint8_t ebi_index);

/**
 * @brief  : function to proces delete session request on PGWC
 * @param  : ds_req, holds information in delete session request
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
process_pgwc_s5s8_delete_session_request(del_sess_req_t *ds_req);

/**
 * @brief  : Delete all pdr, far, qer entry from table
 * @param  : ebi_index, index of bearer in array
 * @param  : context , pointer to ue context structure
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
delete_dedicated_bearers(pdn_connection_t *pdn, uint8_t bearer_ids[], uint8_t bearer_cntr);

/**
 * @brief  : Generate string using sdf packet filters
 * @param  : sdf_flow , sdf packect filter info
 * @param  : sdf_str , string to store output
 * @param  : direction, data flow direction
 * @return : Returns nothing
 */
void
sdf_pkt_filter_to_string(flow_desc_t *sdf_flow, char *sdf_str,uint8_t direction);

/**
 * @brief  : Fill sdf packet filters  in pfcp session establishment  request
 * @param  : pfcp_sess_est_req, structure to be filled
 * @param  : bearer, pointer to bearer structure
 * @param  : pdr_counter , index to pdr
 * @param  : sdf_filter_count
 * @param  : dynamic_filter_cnt
 * @param  : flow_cnt
 * @param  : direction, data flow direction
 * @return : Returns nothing
 */
void sdf_pkt_filter_add(pfcp_sess_estab_req_t* pfcp_sess_est_req, dynamic_rule_t *dynamic_rules,
		int pdr_counter, int sdf_filter_count, int flow_cnt, uint8_t direction);


/**
 * @brief  : Fill sdf packet filters in pfcp session modification request
 * @param  : pfcp_sess_mod_req, structure to be filled
 * @param  : bearer, pointer to bearer structure
 * @param  : pdr_counter , index to pdr
 * @param  : sdf_filter_count
 * @param  : dynamic_filter_cnt
 * @param  : flow_cnt
 * @param  : direction, data flow direction
 * @return : Returns nothing
 */
void
sdf_pkt_filter_mod(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
		eps_bearer_t* bearer,int pdr_counter,
		int sdf_filter_count, int dynamic_filter_cnt, int flow_cnt,
		uint8_t direction);

void sdf_pkt_filter_gx_mod(pfcp_create_pdr_ie_t *pdr, dynamic_rule_t *dyn_rule, int sdf_filter_count, int flow_cnt, uint8_t direction);

/**
 * @brief  : Fill sdf rules in pfcp session establishment  request
 * @param  : pfcp_sess_est_req, structure to be filled
 * @param  : bearer, pointer to bearer structure
 * @param  : pdr_counter , index to pdr
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
fill_sdf_rules(pfcp_sess_estab_req_t *pfcp_sess_est_req,
		dynamic_rule_t *dynamic_rules, int pdr_counter);

/**
 * @brief  : Fill sdf rules in pfcp session mod request
 * @param  : pfcp_sess_mod_req, structure to be filled
 * @param  : bearer, pointer to bearer structure
 * @param  : pdr_counter , index to pdr
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
fill_sdf_rules_modification(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		eps_bearer_t *bearer, int pdr_counter);

/**
 * @brief  : Fill pdr , far and qer in pfcp session mod request from bearer
 * @param  : pfcp_sess_mod_req, structure to be filled
 * @param  : bearer, pointer to bearer structure
 * @return : Returns nothing
 */
void
fill_pdr_far_qer_using_bearer(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		eps_bearer_t *bearer);

/**
 * @brief  : Fill dedicated bearer information
 * @param  : bearer, pointer to bearer structure
 * @param  : context , pointer to ue context structure
 * @param  : pdn , pointer to pdn connection structure
 * @retrun : Returns 0 in case of success , -1 otherwise
 */
int
fill_dedicated_bearer_info(eps_bearer_t *bearer, ue_context_t *context, pdn_connection_t *pdn);

/**
 * @brief  : Fill gate status in pfcp session establishment request
 * @param  : pfcp_sess_est_req , structure to be filled
 * @param  : qer_counter , qer rule index
 * @param  : f_status , flow status
 * @return : Returns nothing
 */
void fill_gate_status(pfcp_sess_estab_req_t *pfcp_sess_est_req,int qer_counter,enum flow_status f_status);


int
fill_create_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule, eps_bearer_t *bearer);

int
fill_update_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule);

int
fill_remove_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer);



/**
 * @brief  : Fill pfcp session delete request
 * @param  : pfcp_sess_del_req , structure to be filled
 * @return : Returns nothing
 */
uint32_t
fill_pfcp_sess_del_req( pfcp_sess_del_req_t *pfcp_sess_del_req);

void
fill_update_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer);

int fill_upd_bearer_sdf_rule(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
								eps_bearer_t* bearer, int pdr_counter);

void sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
    eps_bearer_t* bearer,
    int pdr_counter,
    int sdf_filter_count,
    int dynamic_filter_cnt,
    int flow_cnt,
    uint8_t direction);

#ifdef __cplusplus
}
#endif
#endif /* PFCP_CP_SESSION_H */
