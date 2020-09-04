// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_errno.h"
#include "pfcp_cp_util.h"
#include "pfcp_enum.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "sm_structs_api.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "ue.h"
#include "cp_main.h"
#include "pfcp.h"
#include "ipc_api.h"
#include "cp_config.h"
#include "cp_config_apis.h"
#include "gtpv2_session.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "csid_api.h"
#include "cp_config_defs.h"
#include "ip_pool.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gtpv2_internal.h"
#include "gx_interface.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"


#define size sizeof(pfcp_sess_mod_req_t) /* ajay - clean this */
/* Header Size of set_upd_forwarding_param ie */
#define UPD_PARAM_HEADER_SIZE 4

/* len of flags*/
#define FLAG_LEN 2

uint32_t 
fill_pfcp_sess_del_req( pfcp_sess_del_req_t *pfcp_sess_del_req)
{
	uint32_t seq = 1;

	memset(pfcp_sess_del_req, 0, sizeof(pfcp_sess_del_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_DELETION_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_del_req->header),
		PFCP_SESSION_DELETION_REQUEST, HAS_SEID, seq);

    return seq;
}

void
fill_pfcp_sess_set_del_req( pfcp_sess_set_del_req_t *pfcp_sess_set_del_req)
{

	uint32_t seq = 1;
	char sgwc_addr[INET_ADDRSTRLEN] = {0};
	char pgwc_addr[INET_ADDRSTRLEN] = {0};
	char mme_addr[INET_ADDRSTRLEN]  = {0};
	char sgwu_addr[INET_ADDRSTRLEN] = {0};
	char pgwu_addr[INET_ADDRSTRLEN] = {0};
	uint32_t node_value = 0;

	/*Added hardcoded value to remove compile error.Right now,we are using
	function. Will remove hard value  */
	const char* pAddr = "192.168.0.10";
	const char* twan_addr = "192.16.0.1";
	const char* epdg_addr = "192.16.0.2";
	unsigned long sgwc_value = 0;

	memset(pfcp_sess_set_del_req, 0, sizeof(pfcp_sess_set_del_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_SET_DELETION_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_set_del_req->header),
			PFCP_SESSION_SET_DELETION_REQUEST, HAS_SEID, seq);

	node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_sess_set_del_req->node_id), node_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->sgw_c_fqcsid), sgwc_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->pgw_c_fqcsid), pgwc_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwu_addr, INET_ADDRSTRLEN);
	unsigned long sgwu_value = inet_addr(sgwu_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->sgw_u_fqcsid), sgwu_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwu_addr, INET_ADDRSTRLEN);
	unsigned long pgwu_value = inet_addr(pgwu_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->pgw_u_fqcsid), pgwu_value);

	// set of twan fqcsid
	//TODO : IP addres for twan is hardcoded
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->twan_fqcsid), twan_value);

	// set of epdg fqcsid
	//TODO : IP addres for epdgg is hardcoded
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->epdg_fqcsid), epdg_value);

	inet_ntop(AF_INET, &(cp_config->s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->mme_fqcsid), mme_value);

}

void
fill_pfcp_gx_sess_mod_req( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn)
{

	int ret = 0;
	uint8_t bearer_id = 0;
	uint32_t seq = 0;
	eps_bearer_t *bearer = NULL;
	upf_context_t *upf_ctx = NULL;


	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
			&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return;
	}

	memset(pfcp_sess_mod_req,0,sizeof(pfcp_sess_mod_req_t));
	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
					           HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	if ((cp_config->cp_type == PGWC) ||
			(SAEGWC == cp_config->cp_type))
	{
		for (int idx=0; idx <  (pdn->policy.count - pdn->policy.num_charg_rule_delete); idx++)
		{
			if(pdn->policy.pcc_rule[idx].action == RULE_ACTION_ADD)
			{
				/*
				 * Installing new rule
				 */
				 bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx].dyn_rule.qos);
				 if(bearer == NULL)
				 {
					/*
					 * create dedicated bearer
					 */
					bearer = rte_zmalloc_socket(NULL, sizeof(eps_bearer_t),
							RTE_CACHE_LINE_SIZE, rte_socket_id());
					if(bearer == NULL)
					{
						clLog(sxlogger, eCLSeverityCritical,
							"Failure to allocate bearer "
							"structure: %s (%s:%d)\n",
							rte_strerror(rte_errno),
							 __FILE__,  __LINE__);
						return;
						/* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
					}
					bzero(bearer,  sizeof(eps_bearer_t));
					bearer->pdn = pdn;
					bearer_id = get_new_bearer_id(pdn);
					pdn->eps_bearers[bearer_id] = bearer;
					pdn->context->eps_bearers[bearer_id] = bearer;
					pdn->num_bearer++;
					set_s5s8_pgw_gtpu_teid_using_pdn(bearer, pdn);
					fill_dedicated_bearer_info(bearer, pdn->context, pdn);
					memcpy(&(bearer->qos), &(pdn->policy.pcc_rule[idx].dyn_rule.qos), sizeof(bearer_qos_ie));

				 }
				 bearer->dynamic_rules[bearer->num_dynamic_filters] = rte_zmalloc_socket(NULL, sizeof(dynamic_rule_t),
						 RTE_CACHE_LINE_SIZE, rte_socket_id());
				 if (bearer->dynamic_rules[bearer->num_dynamic_filters] == NULL)
				 {
					 clLog(sxlogger, eCLSeverityCritical,
						"Failure to allocate dynamic rule memory "
						"structure: %s (%s:%d)\n",
						rte_strerror(rte_errno),
						__FILE__, __LINE__);
					 return;
					 /* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
				 }

				 fill_pfcp_entry(bearer, &pdn->policy.pcc_rule[idx].dyn_rule, RULE_ACTION_ADD);

				 memcpy( (bearer->dynamic_rules[bearer->num_dynamic_filters]),
						 &(pdn->policy.pcc_rule[idx].dyn_rule),
						 sizeof(dynamic_rule_t));

				 fill_create_pfcp_info(pfcp_sess_mod_req, &pdn->policy.pcc_rule[idx].dyn_rule);
				 bearer->num_dynamic_filters++;

				//Adding rule and bearer id to a hash
				bearer_id_t *id;
				id = malloc(sizeof(bearer_id_t));
				memset(id, 0 , sizeof(bearer_id_t));
				rule_name_key_t key = {0};
				id->bearer_id = bearer_id;
				strncpy(key.rule_name, pdn->policy.pcc_rule[idx].dyn_rule.rule_name,
						strlen(pdn->policy.pcc_rule[idx].dyn_rule.rule_name));
				sprintf(key.rule_name, "%s%d", key.rule_name, pdn->call_id);
				if (add_rule_name_entry(key, id) != 0) {
					clLog(sxlogger, eCLSeverityCritical,
						"%s:%d Failed to add_rule_name_entry with rule_name\n",
						__func__, __LINE__);
					return;
				}
			}
			else if(pdn->policy.pcc_rule[idx].action == RULE_ACTION_MODIFY)
			{
				/*
				 * Currently not handling dynamic rule qos modificaiton
				 */
				bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx].dyn_rule.qos);
				if(bearer == NULL)
				{
					 clLog(sxlogger, eCLSeverityCritical, "Failure to find bearer "
							 "structure: %s (%s:%d)\n",
							 rte_strerror(rte_errno),
							 __FILE__,
							 __LINE__);
					 return;
					 /* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
				}
				fill_pfcp_entry(bearer, &pdn->policy.pcc_rule[idx].dyn_rule, RULE_ACTION_MODIFY);
				fill_update_pfcp_info(pfcp_sess_mod_req, &pdn->policy.pcc_rule[idx].dyn_rule);

			}
		}

		/* TODO: Remove Below section START after install, modify and remove support */
		if (pdn->policy.num_charg_rule_delete != 0) {
			memset(pfcp_sess_mod_req,0,sizeof(pfcp_sess_mod_req_t));
			seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

			set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
							           HAS_SEID, seq);

			pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

			//TODO modify this hard code to generic
			inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
			node_value = inet_addr(pAddr);

			set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);
		}
		/* TODO: Remove Below section END */

		uint8_t idx_offset =  pdn->policy.num_charg_rule_install + pdn->policy.num_charg_rule_modify;
		for (int idx = 0; idx < pdn->policy.num_charg_rule_delete; idx++) {
			if(RULE_ACTION_DELETE == pdn->policy.pcc_rule[idx + idx_offset].action)
			{
				/* bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx + idx_offset].dyn_rule.qos);
				if(NULL == bearer)
				{
					 clLog(clSystemLog, eCLSeverityCritical, "Failure to find bearer "
							 "structure: %s (%s:%d)\n",
							 rte_strerror(rte_errno),
							 __FILE__,
							 __LINE__);
					 return;
				} */

				rule_name_key_t rule_name = {0};
				memset(rule_name.rule_name, '\0', sizeof(rule_name.rule_name));
				strncpy(rule_name.rule_name, pdn->policy.pcc_rule[idx + idx_offset].dyn_rule.rule_name,
					   strlen(pdn->policy.pcc_rule[idx + idx_offset].dyn_rule.rule_name));
				sprintf(rule_name.rule_name, "%s%d",
						rule_name.rule_name, pdn->call_id);
				int8_t bearer_id = get_rule_name_entry(rule_name);
				if (-1 == bearer_id) {
					/* TODO: Error handling bearer not found */
				}

				if ((bearer_id + 5) == pdn->default_bearer_id) {
					for (uint8_t iCnt = 0; iCnt < MAX_BEARERS; ++iCnt) {
						if (NULL != pdn->eps_bearers[iCnt]) {
							fill_remove_pfcp_info(pfcp_sess_mod_req, pdn->eps_bearers[iCnt]);
						}
					}
				} else {
					fill_remove_pfcp_info(pfcp_sess_mod_req, pdn->eps_bearers[bearer_id]);
				}
			}
		}
	}
}
#define MAX_PDR_PER_RULE 2
int
fill_create_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule)
{

	uint8_t sdf_filter_count = 0;
	pfcp_create_pdr_ie_t *pdr = NULL;
	pfcp_create_far_ie_t *far = NULL;
	pfcp_create_qer_ie_t *qer = NULL;

	for(int i=0; i<MAX_PDR_PER_RULE; i++)
	{
		pdr = &(pfcp_sess_mod_req->create_pdr[i]);
		far = &(pfcp_sess_mod_req->create_far[i]);
		qer = &(pfcp_sess_mod_req->create_qer[i]);

		pdr->qer_id_count = 1;
		pdr->qer_id_count = 1;

		creating_pdr(pdr, i);

		pdr->pdr_id.rule_id = dyn_rule->pdr[i]->rule_id;
		pdr->precedence.prcdnc_val = dyn_rule->pdr[i]->prcdnc_val;
		pdr->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;
		pdr->qer_id[0].qer_id_value = dyn_rule->pdr[i]->qer.qer_id;

		pdr->pdi.ue_ip_address.ipv4_address =
			htonl(dyn_rule->pdr[i]->pdi.ue_addr.ipv4_address);
		pdr->pdi.local_fteid.teid =
			dyn_rule->pdr[i]->pdi.local_fteid.teid;
		pdr->pdi.local_fteid.ipv4_address =
				htonl(dyn_rule->pdr[i]->pdi.local_fteid.ipv4_address);
		pdr->pdi.src_intfc.interface_value =
				dyn_rule->pdr[i]->pdi.src_intfc.interface_value;
		strncpy((char *)pdr->pdi.ntwk_inst.ntwk_inst,
				(char *)&dyn_rule->pdr[i]->pdi.ntwk_inst.ntwk_inst, 32);

		pdr->pdi.src_intfc.interface_value =
			dyn_rule->pdr[i]->pdi.src_intfc.interface_value;

		if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) {
			uint32_t size_teid = 0;
			size_teid = pdr->pdi.local_fteid.header.len + sizeof(pfcp_ie_header_t);
			pdr->pdi.header.len = pdr->pdi.header.len - size_teid;
			pdr->header.len = pdr->header.len - size_teid;
			pdr->pdi.local_fteid.header.len = 0;
		} else if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) {
			uint32_t size_ie = pdr->pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t) + pdr->pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);

			pdr->pdi.header.len -= size_ie;
			pdr->header.len -= size_ie;

			pdr->pdi.ue_ip_address.header.len = 0;
			pdr->pdi.ntwk_inst.header.len = 0;
		}
#if 0
		memcpy(&(pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
			&(dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc);

		pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc =
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc;

		pdr->pdi.sdf_filter_count++;
#endif
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++) {

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {

				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					sdf_pkt_filter_gx_mod(
							pdr, dyn_rule, sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {
				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					sdf_pkt_filter_gx_mod(
							pdr, dyn_rule, sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}
		}
		pdr->pdi.sdf_filter_count = sdf_filter_count;

		creating_far(far);
		far->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;
		set_destination_interface(&(far->frwdng_parms.dst_intfc));
		pfcp_set_ie_header(&(far->frwdng_parms.header),
				PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

		far->frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

		uint16_t len = 0;
		len += sizeof(pfcp_dst_intfc_ie_t);
		len += UPD_PARAM_HEADER_SIZE;

		far->header.len += len;
		far->frwdng_parms.dst_intfc.interface_value = dyn_rule->pdr[i]->far.dst_intfc.interface_value;

		far->apply_action.forw = PRESENT;

		creating_qer(qer);
		qer->qer_id.qer_id_value  = dyn_rule->pdr[i]->qer.qer_id;

		qer->maximum_bitrate.ul_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.ul_mbr;
		qer->maximum_bitrate.dl_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.dl_mbr;
		qer->guaranteed_bitrate.ul_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.ul_gbr;
		qer->guaranteed_bitrate.dl_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.dl_gbr;
		qer->gate_status.ul_gate  = dyn_rule->pdr[i]->qer.gate_status.ul_gate;
		qer->gate_status.dl_gate  = dyn_rule->pdr[i]->qer.gate_status.dl_gate;

		pfcp_sess_mod_req->create_pdr_count++;
		pfcp_sess_mod_req->create_far_count++;
		pfcp_sess_mod_req->create_qer_count++;
	}
	return 0;
}

int
fill_update_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule)
{

	uint8_t sdf_filter_count = 0;
	pfcp_update_pdr_ie_t *pdr = NULL;
	pfcp_update_far_ie_t *far = NULL;
	pfcp_update_qer_ie_t *qer = NULL;

	for(int i=0; i<MAX_PDR_PER_RULE; i++)
	{
		pdr = &(pfcp_sess_mod_req->update_pdr[pfcp_sess_mod_req->update_pdr_count+i]);
		far = &(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count+i]);
		qer = &(pfcp_sess_mod_req->update_qer[pfcp_sess_mod_req->update_qer_count+i]);

		updating_pdr(pdr, i);

		pdr->pdr_id.rule_id = dyn_rule->pdr[i]->rule_id;
		pdr->precedence.prcdnc_val = dyn_rule->pdr[i]->prcdnc_val;
		pdr->qer_id.qer_id_value = dyn_rule->pdr[i]->qer_id[0].qer_id;
		pdr->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;


		pdr->pdi.ue_ip_address.ipv4_address =
			htonl(dyn_rule->pdr[i]->pdi.ue_addr.ipv4_address);
		pdr->pdi.local_fteid.teid =
			dyn_rule->pdr[i]->pdi.local_fteid.teid;
		pdr->pdi.local_fteid.ipv4_address =
				htonl(dyn_rule->pdr[i]->pdi.local_fteid.ipv4_address);
		pdr->pdi.src_intfc.interface_value =
				dyn_rule->pdr[i]->pdi.src_intfc.interface_value;
		strncpy((char *)pdr->pdi.ntwk_inst.ntwk_inst,
				(char *)&dyn_rule->pdr[i]->pdi.ntwk_inst.ntwk_inst, 32);

		pdr->pdi.src_intfc.interface_value =
			dyn_rule->pdr[i]->pdi.src_intfc.interface_value;

		if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) {
			uint32_t size_teid = 0;
			size_teid = pdr->pdi.local_fteid.header.len + sizeof(pfcp_ie_header_t);
			pdr->pdi.header.len = pdr->pdi.header.len - size_teid;
			pdr->header.len = pdr->header.len - size_teid;
			pdr->pdi.local_fteid.header.len = 0;
		} else if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) {
			uint32_t size_ie = pdr->pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t) + pdr->pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);

			pdr->pdi.header.len -= size_ie;
			pdr->header.len -= size_ie;

			pdr->pdi.ue_ip_address.header.len = 0;
			pdr->pdi.ntwk_inst.header.len = 0;
		}
#if 0
		memcpy(&(pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
			&(dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc);

		pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc =
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc;

		pdr->pdi.sdf_filter_count++;
#endif
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++) {

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {

				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					/* TODO: Revisit following line, change funtion signature and remove type casting */
					sdf_pkt_filter_gx_mod((pfcp_create_pdr_ie_t *)pdr, dyn_rule,
							 sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {
				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					/* TODO: Revisit following line, change funtion signature and remove type casting */
					sdf_pkt_filter_gx_mod((pfcp_create_pdr_ie_t *) pdr, dyn_rule,
							sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}
		}
		pdr->pdi.sdf_filter_count = sdf_filter_count;

		updating_far(far);
		far->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;
		set_destination_interface(&(far->upd_frwdng_parms.dst_intfc));
		pfcp_set_ie_header(&(far->upd_frwdng_parms.header),
				PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

		far->upd_frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

		uint16_t len = 0;
		len += sizeof(pfcp_dst_intfc_ie_t);
		len += UPD_PARAM_HEADER_SIZE;

		far->header.len += len;
		far->upd_frwdng_parms.dst_intfc.interface_value = dyn_rule->pdr[i]->far.dst_intfc.interface_value;

		far->apply_action.forw = PRESENT;

		updating_qer(qer);
		qer->qer_id.qer_id_value  = dyn_rule->pdr[i]->qer.qer_id;

		qer->maximum_bitrate.ul_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.ul_mbr;
		qer->maximum_bitrate.dl_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.dl_mbr;
		qer->guaranteed_bitrate.ul_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.ul_gbr;
		qer->guaranteed_bitrate.dl_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.dl_gbr;
		qer->gate_status.ul_gate  = dyn_rule->pdr[i]->qer.gate_status.ul_gate;
		qer->gate_status.dl_gate  = dyn_rule->pdr[i]->qer.gate_status.dl_gate;

		pfcp_sess_mod_req->update_pdr_count++;
		pfcp_sess_mod_req->update_far_count++;
		pfcp_sess_mod_req->update_qer_count++;
	}
	return 0;
}

int
fill_remove_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer)
{
	pfcp_update_far_ie_t *far = NULL;

	for(int i=0; i<MAX_PDR_PER_RULE; i++)
	{
		far = &(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]);

		updating_far(far);
		far->far_id.far_id_value = bearer->pdrs[i]->far.far_id_value;
		far->apply_action.drop = PRESENT;

		pfcp_sess_mod_req->update_far_count++;
	}
	return 0;
}

void sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
    eps_bearer_t* bearer,
    int pdr_counter,
    int sdf_filter_count,
    int dynamic_filter_cnt,
    int flow_cnt,
    uint8_t direction)
{
    int len = 0;
    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
    sdf_pkt_filter_to_string(&(bearer->dynamic_rules[dynamic_filter_cnt]->flow_desc[flow_cnt].sdf_flw_desc),
        (char*)(pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
        strlen((char*)(&pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

    len += FLAG_LEN;
    len += sizeof(uint16_t);
    len += pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

    pfcp_set_ie_header(
        &(pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

    /*VG updated the header len of pdi as sdf rules has been added*/
    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
    pfcp_sess_mod_req->update_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}

int fill_upd_bearer_sdf_rule(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
								eps_bearer_t* bearer,	int pdr_counter){
    int ret = 0;
    int sdf_filter_count = 0;
    /*VG convert pkt_filter_strucutre to char string*/
    for(int index = 0; index < bearer->num_dynamic_filters; index++) {

        pfcp_sess_mod_req->update_pdr[pdr_counter].precedence.prcdnc_val = bearer->dynamic_rules[index]->precedence;
        // itr is for flow information counter
        // sdf_filter_count is for SDF information counter
        for(int itr = 0; itr < bearer->dynamic_rules[index]->num_flw_desc; itr++) {

            if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {

                if((pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
                    ((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
                    (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

                    sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req, bearer, pdr_counter,
                    			sdf_filter_count, index, itr, TFT_DIRECTION_UPLINK_ONLY);
                    sdf_filter_count++;
                }

            } else {
                clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
            }

            if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {
                if((pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
                    ((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
                    (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
                    sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req, bearer, pdr_counter,
                    		sdf_filter_count, index, itr, TFT_DIRECTION_DOWNLINK_ONLY);
                    sdf_filter_count++;
                }
            } else {
                clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
            }
        }

		pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;

    }
    return ret;
}

void
fill_update_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer){

	int size1 = 0;

	for(int i = pfcp_sess_mod_req->update_pdr_count;
			i < pfcp_sess_mod_req->update_pdr_count + NUMBER_OF_PDR_PER_BEARER;
			i++){

		size1 = 0;
		size1 += set_pdr_id(&(pfcp_sess_mod_req->update_pdr[i].pdr_id));
		size1 += set_precedence(&(pfcp_sess_mod_req->update_pdr[i].precedence));
		size1 += set_pdi(&(pfcp_sess_mod_req->update_pdr[i].pdi));

		int itr = i - pfcp_sess_mod_req->update_pdr_count;

		pfcp_set_ie_header(&(pfcp_sess_mod_req->update_pdr[i].header), PFCP_IE_UPDATE_PDR, size1);

		pfcp_sess_mod_req->update_pdr[i].pdr_id.rule_id = bearer->pdrs[itr]->rule_id;

		pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.teid =
			bearer->pdrs[itr]->pdi.local_fteid.teid;

		if((cp_config->cp_type == SGWC) ||
				(bearer->pdrs[itr]->pdi.src_intfc.interface_value ==
				SOURCE_INTERFACE_VALUE_ACCESS)) {
			/*No need to send ue ip and network instance for pgwc access interface or
			 * for any sgwc interface */
			uint32_t size_ie = 0;
			size_ie = pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t);
			size_ie = size_ie + pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->update_pdr[i].pdi.header.len =
				pfcp_sess_mod_req->update_pdr[i].pdi.header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].header.len =
				pfcp_sess_mod_req->update_pdr[i].header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.header.len = 0;
			pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.header.len = 0;
		}else{
			pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.ipv4_address =
				bearer->pdrs[itr]->pdi.ue_addr.ipv4_address;
			strncpy((char *)pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.ntwk_inst,
				(char *)&bearer->pdrs[itr]->pdi.ntwk_inst.ntwk_inst, 32);
		}

		if (
				((PGWC == cp_config->cp_type) || (SAEGWC == cp_config->cp_type)) &&
				(SOURCE_INTERFACE_VALUE_CORE ==
				bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {

			uint32_t size_ie = 0;

			size_ie = pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->update_pdr[i].pdi.header.len =
				pfcp_sess_mod_req->update_pdr[i].pdi.header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].header.len =
				pfcp_sess_mod_req->update_pdr[i].header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.header.len = 0;

		} else {
			pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.ipv4_address =
				bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
		}

		pfcp_sess_mod_req->update_pdr[i].pdi.src_intfc.interface_value =
			bearer->pdrs[itr]->pdi.src_intfc.interface_value;

		fill_upd_bearer_sdf_rule(pfcp_sess_mod_req, bearer, i);
	}

	pfcp_sess_mod_req->update_pdr_count += NUMBER_OF_PDR_PER_BEARER;
	return;
}

/* REVIEW: Context will remove after merging */
uint32_t 
fill_pfcp_sess_mod_req( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, eps_bearer_t *bearer,
		pdn_connection_t *pdn, pfcp_update_far_ie_t update_far[], uint8_t x2_handover_flag)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
			&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return 0;
	}

	if( header != NULL)
		clLog(sxlogger, eCLSeverityDebug, "TEID[%d]\n", header->teid.has_teid.teid);

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
					           HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	/*SP: This depends on condition in pcrf data(pcrf will send bar_rule_id if it needs to be delated). Need to handle after pcrf integration*/
	/* removing_bar(&(pfcp_sess_mod_req->remove_bar)); */

	//set create PDR

	/************************************************
	 *  cp_type  count     FTEID_1          FTEID_2 *
	 *************************************************
	 In case MBR received from MME:-
	 SGWC         1      enodeB               -
	 PGWC         -        -                  -
	 SAEGWC       1      enodeB               -
	 *************************************************
	 In case of CSResp received from PGWC to SGWC :-
	 SGWC <----CSResp--- PGWC
	 |
	 pfcp_sess_mod_req
	 |
	 v
	 SGWU
	 In above scenario:
	 count = 1 ,     FTEID_1 = s5s8 PGWU
	 ************************************************/
	/*SP: create pdr IE is not needed in session modification request , hence removing*/
	/*
	pfcp_sess_mod_req->create_pdr_count = 1;

	for( int i = 0; i < pfcp_sess_mod_req->create_pdr_count ; i++)
		creating_pdr(&(pfcp_sess_mod_req->create_pdr[i]));
	*/

	if (pfcp_sess_mod_req->create_pdr_count) {
		fill_pdr_far_qer_using_bearer(pfcp_sess_mod_req, bearer);
	}

	/*SP: This depends on condition  if the CP function requests the UP function to create a new BAR
	  Need to add condition to check if CP needs creation of BAR*/
	for( int i = 0; i < pfcp_sess_mod_req->create_pdr_count ; i++){
		if((pfcp_sess_mod_req->create_pdr[i].header.len) &&
				(pfcp_sess_mod_req->create_pdr[i].far_id.header.len)){
			for( int j = 0; j < pfcp_sess_mod_req->create_far_count ; j++){
				if(pfcp_sess_mod_req->create_far[i].bar_id.header.len){
					/* TODO: Pass bar_id from pfcp_session_mod_req->create_far[i].bar_id.bar_id_value
					   to set bar_id*/
					creating_bar(&(pfcp_sess_mod_req->create_bar));
				}
			}
		}
	}

	/*SP: Adding FAR IE*/
	for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count ; itr1++) {
		for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
			if(bearer->pdrs[itr]->pdi.src_intfc.interface_value !=
					update_far[itr1].upd_frwdng_parms.dst_intfc.interface_value){
				pdr_ctxt = bearer->pdrs[itr];
				updating_far(&(pfcp_sess_mod_req->update_far[itr1]));
				pfcp_sess_mod_req->update_far[itr1].far_id.far_id_value =
					pdr_ctxt->far.far_id_value;
				pfcp_sess_mod_req->update_far[itr1].apply_action.forw = PRESENT;
				if (pfcp_sess_mod_req->update_far[itr1].apply_action.forw == PRESENT) {
					uint16_t len = 0;
					len += set_upd_forwarding_param(&(pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms));
					/* Currently take as hardcoded value */
					len += UPD_PARAM_HEADER_SIZE;
					pfcp_sess_mod_req->update_far[itr1].header.len += len;
				}
				pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.outer_hdr_creation.teid =
					update_far[itr1].upd_frwdng_parms.outer_hdr_creation.teid;
				pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
					(update_far[itr1].upd_frwdng_parms.outer_hdr_creation.ipv4_address);
				pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.dst_intfc.interface_value =
					update_far[itr1].upd_frwdng_parms.dst_intfc.interface_value;

				if(x2_handover_flag) {

					set_pfcpsmreqflags(&(pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.pfcpsmreq_flags));
					pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.pfcpsmreq_flags.sndem = 1;
					pfcp_sess_mod_req->update_far[itr1].header.len += sizeof(struct  pfcp_pfcpsmreq_flags_ie_t);
					pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.header.len += sizeof(struct  pfcp_pfcpsmreq_flags_ie_t);

				}
			}

		}
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->create_pdr_count){
				for(int itr = 0; itr < pfcp_sess_mod_req->create_pdr_count; itr++) {
					pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.teid =
						bearer->pdrs[itr]->pdi.local_fteid.teid ;
					/* TODO: Revisit this for change in yang */
					pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.ipv4_address =
						htonl(bearer->pdrs[itr]->pdi.ue_addr.ipv4_address);
					pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.ipv4_address =
						bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
					pfcp_sess_mod_req->create_pdr[itr].pdi.src_intfc.interface_value =
						bearer->pdrs[itr]->pdi.src_intfc.interface_value;
				}
			}
			break;

		case PGWC :
			break;

		default :
			clLog(clSystemLog, eCLSeverityDebug,"%s:%d default pfcp sess mod req\n", __func__, __LINE__);
			break;
	}

	// set of update QER
	/*SP: No QER is not generated previously, No update needed*/
	/*
	pfcp_sess_mod_req->update_qer_count = bearer->qer_count;

	for(int i=0; i < pfcp_sess_mod_req->update_qer_count; i++ ){
		updating_qer(&(pfcp_sess_mod_req->update_qer[i]));
		pfcp_sess_mod_req->update_qer[i] == bearer->qer_id.qer.id;
	}
	*/

	// set of update BAR
	/*SP: If previously created BAR needs to be modified, this IE should be included*/
	/*
	 updating_bar(&(pfcp_sess_mod_req->update_bar));
	*/

	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	      excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}

	/*SP: This IE is included if node supports Partial failure handling support
	      excluding this IE since we dont have this support  */
	/*
	char sgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	unsigned long sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_mod_req->sgw_c_fqcsid), sgwc_value);

	char mme_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config.s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_mod_req->mme_fqcsid), mme_value);

	char pgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_mod_req->pgw_c_fqcsid), pgwc_value);

	//TODO : IP addres for epdgg is hardcoded
	const char* epdg_addr = "0.0.0.0";
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_mod_req->epdg_fqcsid), epdg_value);

	//TODO : IP addres for twan is hardcoded
	const char* twan_addr = "0.0.0.0";
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_mod_req->twan_fqcsid), twan_value);
	*/

	 /*SP: Not in use*/
	 /*
		set_up_inactivity_timer(&(pfcp_sess_mod_req->user_plane_inact_timer));
	 */

	/*SP: This IE is included if QAURR flag is set (this flag is in PFCPSMReq-Flags IE) or Query URR IE is present,
	  Adding check to exclud  this IE if any of these condition is not satisfied*/
	if(pfcp_sess_mod_req->pfcpsmreq_flags.qaurr ||
			pfcp_sess_mod_req->query_urr_count){
		set_query_urr_refernce(&(pfcp_sess_mod_req->query_urr_ref));
	}

	if (upf_ctx->up_supp_features & UP_TRACE)
		set_trace_info(&(pfcp_sess_mod_req->trc_info));

    return seq;
}

void
sdf_pkt_filter_to_string(sdf_pkt_fltr *sdf_flow,
		char *sdf_str , uint8_t direction)
{
	char local_ip[INET_ADDRSTRLEN];
	char remote_ip[INET_ADDRSTRLEN];

	snprintf(local_ip, sizeof(local_ip), "%s",
			inet_ntoa(sdf_flow->local_ip_addr));
	snprintf(remote_ip, sizeof(remote_ip), "%s",
			inet_ntoa(sdf_flow->remote_ip_addr));

	if (direction == TFT_DIRECTION_DOWNLINK_ONLY) {
		snprintf(sdf_str, MAX_LEN, "%s/%"PRIu8" %s/%"PRIu8" %"
				PRIu16" : %"PRIu16" %"PRIu16" : %"PRIu16
				" 0x%"PRIx8"/0x%"PRIx8"",
				local_ip, sdf_flow->local_ip_mask, remote_ip,
				sdf_flow->remote_ip_mask,
				(sdf_flow->local_port_low),
				(sdf_flow->local_port_high),
				(sdf_flow->remote_port_low),
				(sdf_flow->remote_port_high),
				sdf_flow->proto_id, sdf_flow->proto_mask);
	} else if (direction == TFT_DIRECTION_UPLINK_ONLY) {
		snprintf(sdf_str, MAX_LEN, "%s/%"PRIu8" %s/%"PRIu8" %"
				PRIu16" : %"PRIu16" %"PRIu16" : %"PRIu16
				" 0x%"PRIx8"/0x%"PRIx8"",
				local_ip, sdf_flow->local_ip_mask, remote_ip,
				sdf_flow->remote_ip_mask,
				(sdf_flow->local_port_low),
				(sdf_flow->local_port_high),
				(sdf_flow->remote_port_low),
				(sdf_flow->remote_port_high),
				sdf_flow->proto_id, sdf_flow->proto_mask);
	}
}

void
fill_pdr_far_qer_using_bearer(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		eps_bearer_t *bearer)
{
	pfcp_sess_mod_req->create_pdr_count = bearer->pdr_count;

	for(int i = 0; i < pfcp_sess_mod_req->create_pdr_count; i++) {
		pfcp_sess_mod_req->create_pdr[i].qer_id_count = 1;
		//pfcp_sess_mod_req->create_pdr[i].qer_id_count = bearer->qer_count;
		creating_pdr(&(pfcp_sess_mod_req->create_pdr[i]), bearer->pdrs[i]->pdi.src_intfc.interface_value);
		pfcp_sess_mod_req->create_far_count++;
		creating_far(&(pfcp_sess_mod_req->create_far[i]));
	}

	for(int itr = 0; itr < pfcp_sess_mod_req->create_pdr_count ; itr++) {
		pfcp_sess_mod_req->create_pdr[itr].pdr_id.rule_id  =
			bearer->pdrs[itr]->rule_id;
		pfcp_sess_mod_req->create_pdr[itr].far_id.far_id_value =
			bearer->pdrs[itr]->far.far_id_value;
		pfcp_sess_mod_req->create_pdr[itr].precedence.prcdnc_val =
			bearer->pdrs[itr]->prcdnc_val;

		pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.teid =
			bearer->pdrs[itr]->pdi.local_fteid.teid;

		if((cp_config->cp_type == SGWC) ||
				(bearer->pdrs[itr]->pdi.src_intfc.interface_value ==
				SOURCE_INTERFACE_VALUE_ACCESS)) {
			/*No need to send ue ip and network instance for pgwc access interface or
			 * for any sgwc interface */
			uint32_t size_ie = 0;
			size_ie = pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t);
			size_ie = size_ie + pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->create_pdr[itr].pdi.header.len =
				pfcp_sess_mod_req->create_pdr[itr].pdi.header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].header.len =
				pfcp_sess_mod_req->create_pdr[itr].header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.header.len = 0;
			pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.header.len = 0;
		}else{
			pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.ipv4_address =
				bearer->pdrs[itr]->pdi.ue_addr.ipv4_address;
			strncpy((char *)pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.ntwk_inst,
				(char *)&bearer->pdrs[itr]->pdi.ntwk_inst.ntwk_inst, 32);
		}

		if (
				((PGWC == cp_config->cp_type) || (SAEGWC == cp_config->cp_type)) &&
				(SOURCE_INTERFACE_VALUE_CORE ==
				bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {

			uint32_t size_ie = 0;

			size_ie = pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->create_pdr[itr].pdi.header.len =
				pfcp_sess_mod_req->create_pdr[itr].pdi.header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].header.len =
				pfcp_sess_mod_req->create_pdr[itr].header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.header.len = 0;

		} else {
			pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.ipv4_address =
				bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
		}

		pfcp_sess_mod_req->create_pdr[itr].pdi.src_intfc.interface_value =
			bearer->pdrs[itr]->pdi.src_intfc.interface_value;

		pfcp_sess_mod_req->create_far[itr].far_id.far_id_value =
			bearer->pdrs[itr]->far.far_id_value;

        if(cp_config->gx_enabled) {
            if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
                pfcp_sess_mod_req->create_pdr[itr].qer_id_count =
                    bearer->pdrs[itr]->qer_id_count;
                for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_pdr[itr].qer_id_count; itr1++) {
                    pfcp_sess_mod_req->create_pdr[itr].qer_id[itr1].qer_id_value =
                        bearer->pdrs[itr]->qer_id[itr1].qer_id;
                }
            }
        }

		if ((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) {
			pfcp_sess_mod_req->create_far[itr].apply_action.forw = PRESENT;
			if (pfcp_sess_mod_req->create_far[itr].apply_action.forw == PRESENT) {
				uint16_t len = 0;

				if (
						(SAEGWC == cp_config->cp_type) ||
						(SOURCE_INTERFACE_VALUE_ACCESS ==
						 bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {
					set_destination_interface(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc));
					pfcp_set_ie_header(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms.header),
							PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

					pfcp_sess_mod_req->create_far[itr].frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

					len += sizeof(pfcp_dst_intfc_ie_t);
					len += UPD_PARAM_HEADER_SIZE;

					pfcp_sess_mod_req->create_far[itr].header.len += len;

					pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc.interface_value =
						bearer->pdrs[itr]->far.dst_intfc.interface_value;
				} else {
					pfcp_sess_mod_req->create_far[itr].apply_action.forw = NO_FORW_ACTION;
				}
			}
		} else
		if ((SGWC == cp_config->cp_type) &&
			(DESTINATION_INTERFACE_VALUE_CORE ==
			 bearer->pdrs[itr]->far.dst_intfc.interface_value) &&
			(bearer->s5s8_pgw_gtpu_teid != 0) &&
			(bearer->s5s8_pgw_gtpu_ipv4.s_addr != 0))
		{
			uint16_t len = 0;
			len += set_forwarding_param(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms));
			/* Currently take as hardcoded value */
			len += UPD_PARAM_HEADER_SIZE;
			pfcp_sess_mod_req->create_far[itr].header.len += len;

			pfcp_sess_mod_req->create_far[itr].apply_action.forw = PRESENT;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.outer_hdr_creation.ipv4_address =
					bearer->pdrs[itr]->far.outer_hdr_creation.ipv4_address;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.outer_hdr_creation.teid =
					bearer->pdrs[itr]->far.outer_hdr_creation.teid;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc.interface_value =
					bearer->pdrs[itr]->far.dst_intfc.interface_value;
		}

	} /*for loop*/

    if(cp_config->gx_enabled) {
        if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
            pfcp_sess_mod_req->create_qer_count = bearer->qer_count;
            qer_t *qer_context = NULL;
            for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_qer_count ; itr1++) {
                creating_qer(&(pfcp_sess_mod_req->create_qer[itr1]));
                pfcp_sess_mod_req->create_qer[itr1].qer_id.qer_id_value  =
                    bearer->qer_id[itr1].qer_id;
                qer_context = get_qer_entry(pfcp_sess_mod_req->create_qer[itr1].qer_id.qer_id_value);
                /* Assign the value from the PDR */
                if(qer_context){
                    pfcp_sess_mod_req->create_qer[itr1].maximum_bitrate.ul_mbr  =
                        qer_context->max_bitrate.ul_mbr;
                    pfcp_sess_mod_req->create_qer[itr1].maximum_bitrate.dl_mbr  =
                        qer_context->max_bitrate.dl_mbr;
                    pfcp_sess_mod_req->create_qer[itr1].guaranteed_bitrate.ul_gbr  =
                        qer_context->guaranteed_bitrate.ul_gbr;
                    pfcp_sess_mod_req->create_qer[itr1].guaranteed_bitrate.dl_gbr  =
                        qer_context->guaranteed_bitrate.dl_gbr;
                }
            }

            for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_pdr_count ; itr1++) {
                fill_sdf_rules_modification(pfcp_sess_mod_req, bearer, itr1);
            }
        }
    }

}

void fill_gate_status(pfcp_sess_estab_req_t *pfcp_sess_est_req,
	int qer_counter,
	enum flow_status f_status)
{
	switch(f_status)
	{
		case FL_ENABLED_UPLINK:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_OPEN;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_CLOSED;
			break;

		case FL_ENABLED_DOWNLINK:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_CLOSED;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_OPEN;
			break;

		case FL_ENABLED:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_OPEN;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_OPEN;
			break;

		case FL_DISABLED:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_CLOSED;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_CLOSED;
			break;
		case FL_REMOVED:
			/*TODO*/
			break;
	}
}

void sdf_pkt_filter_add(pfcp_sess_estab_req_t* pfcp_sess_est_req,
		dynamic_rule_t *dynamic_rules,
		int pdr_counter,
		int sdf_filter_count,
		int flow_cnt,
		uint8_t direction)
{
	int len = 0;
	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
	sdf_pkt_filter_to_string(&(dynamic_rules->flow_desc[flow_cnt].sdf_flw_desc),
			(char*)(pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pfcp_sess_est_req->create_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}

void sdf_pkt_filter_mod(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
		eps_bearer_t* bearer,
		int pdr_counter,
		int sdf_filter_count,
		int dynamic_filter_cnt,
		int flow_cnt,
		uint8_t direction)
{
	int len = 0;
	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
	sdf_pkt_filter_to_string(&(bearer->dynamic_rules[dynamic_filter_cnt]->flow_desc[flow_cnt].sdf_flw_desc),
			(char*)(pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pfcp_sess_mod_req->create_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}
void sdf_pkt_filter_gx_mod(pfcp_create_pdr_ie_t *pdr, dynamic_rule_t *dyn_rule, int sdf_filter_count, int flow_cnt, uint8_t direction)
{
	int len = 0;
	pdr->pdi.sdf_filter[sdf_filter_count].fd = 1;
	sdf_pkt_filter_to_string(&(dyn_rule->flow_desc[flow_cnt].sdf_flw_desc),
			(char*)(pdr->pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pdr->pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pdr->pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pdr->pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pdr->pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pdr->pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pdr->header.len += (len + sizeof(pfcp_ie_header_t));
}
int fill_sdf_rules_modification(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
	eps_bearer_t* bearer,
	int pdr_counter)
{
	int ret = 0;
	int sdf_filter_count = 0;
	/*VG convert pkt_filter_strucutre to char string*/
	for(int index = 0; index < bearer->num_dynamic_filters; index++) {

		pfcp_sess_mod_req->create_pdr[pdr_counter].precedence.prcdnc_val = bearer->dynamic_rules[index]->precedence;
		// itr is for flow information counter
		// sdf_filter_count is for SDF information counter
		for(int itr = 0; itr < bearer->dynamic_rules[index]->num_flw_desc; itr++) {

			if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {

				if((pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					sdf_pkt_filter_mod(
							pfcp_sess_mod_req, bearer, pdr_counter, sdf_filter_count, index, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}

			if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {
				if((pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					sdf_pkt_filter_mod(
							pfcp_sess_mod_req, bearer, pdr_counter, sdf_filter_count, index, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}
		}

		pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;

	}
	return ret;
}

int fill_sdf_rules(pfcp_sess_estab_req_t* pfcp_sess_est_req,
	dynamic_rule_t *dynamic_rules,
	int pdr_counter)
{
	int ret = 0;
	int sdf_filter_count = 0;
	/*VG convert pkt_filter_strucutre to char string*/

		pfcp_sess_est_req->create_pdr[pdr_counter].precedence.prcdnc_val = dynamic_rules->precedence;
		// itr is for flow information counter
		// sdf_filter_count is for SDF information counter
	for(int itr = 0; itr < dynamic_rules->num_flw_desc; itr++) {

			if(dynamic_rules->flow_desc[itr].sdf_flow_description != NULL) {

				if((pfcp_sess_est_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					sdf_pkt_filter_add(
							pfcp_sess_est_req, dynamic_rules, pdr_counter, sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}

			if(dynamic_rules->flow_desc[itr].sdf_flow_description != NULL) {
				if((pfcp_sess_est_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					sdf_pkt_filter_add(
							pfcp_sess_est_req, dynamic_rules, pdr_counter, sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Empty SDF rules\n", __file__, __func__, __LINE__);
			}
		}

		pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;

	return ret;
}

int
fill_qer_entry(pdn_connection_t *pdn, eps_bearer_t *bearer, uint8_t itr)
{
	int ret = -1;
	qer_t *qer_ctxt = NULL;
	qer_ctxt = rte_zmalloc_socket(NULL, sizeof(qer_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (qer_ctxt == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return ret;
	}
	qer_ctxt->qer_id = bearer->qer_id[itr].qer_id;;
	qer_ctxt->session_id = pdn->seid;
	qer_ctxt->max_bitrate.ul_mbr = bearer->qos.ul_mbr;
	qer_ctxt->max_bitrate.dl_mbr = bearer->qos.dl_mbr;
	qer_ctxt->guaranteed_bitrate.ul_gbr = bearer->qos.ul_gbr;
	qer_ctxt->guaranteed_bitrate.dl_gbr = bearer->qos.dl_gbr;

	ret = add_qer_entry(qer_ctxt->qer_id,qer_ctxt);
	if(ret != 0) {
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Adding qer entry Error: %d \n", __file__,
				__func__, __LINE__, ret);
		return ret;
	}

	return ret;
}

static int
add_qer_into_hash(qer_t *qer)
{
	int ret = -1;
	qer_t *qer_ctxt = NULL;
	qer_ctxt = rte_zmalloc_socket(NULL, sizeof(qer_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (qer_ctxt == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return ret;
	}

	qer_ctxt->qer_id = qer->qer_id;
	qer_ctxt->session_id = qer->session_id;
	qer_ctxt->max_bitrate.ul_mbr = qer->max_bitrate.ul_mbr;
	qer_ctxt->max_bitrate.dl_mbr = qer->max_bitrate.dl_mbr;
	qer_ctxt->guaranteed_bitrate.ul_gbr = qer->guaranteed_bitrate.ul_gbr;
	qer_ctxt->guaranteed_bitrate.dl_gbr = qer-> guaranteed_bitrate.dl_gbr;

	ret = add_qer_entry(qer_ctxt->qer_id, qer_ctxt);

	if(ret != 0) {
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Adding qer entry Error: %d \n", __file__,
				__func__, __LINE__, ret);
		return ret;
	}


	return ret;
}

int fill_pfcp_entry(eps_bearer_t *bearer, dynamic_rule_t *dyn_rule,
		enum rule_action_t rule_action)
{
	/*
	 * For ever PCC rule create 2 PDR and 2 QER and 2 FAR
	 * all these struture should be created and filled here
	 * also store its reference in rule itself
	 * May be pdr arrary in bearer not needed
	 */
	char mnc[4] = {0};
	char mcc[4] = {0};
	char nwinst[32] = {0};
	ue_context_t *context = bearer->pdn->context;
	pdn_connection_t *pdn = bearer->pdn;
	pdr_t *pdr_ctxt = NULL;
	int ret;
	uint16_t flow_len = 0;

	if (context->serving_nw.mnc_digit_3 == 15) {
		sprintf(mnc, "0%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2);
	} else {
		sprintf(mnc, "%u%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2,
				context->serving_nw.mnc_digit_3);
	}

	sprintf(mcc, "%u%u%u", context->serving_nw.mcc_digit_1,
			context->serving_nw.mcc_digit_2,
			context->serving_nw.mcc_digit_3);

	sprintf(nwinst, "mnc%s.mcc%s", mnc, mcc);

	for(int i =0; i < 2; i++)
	{

		pdr_ctxt = rte_zmalloc_socket(NULL, sizeof(pdr_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (pdr_ctxt == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
					"structure: %s (%s:%d)\n",
					rte_strerror(rte_errno),
					__FILE__,
					__LINE__);
			return -1;
		}
		memset(pdr_ctxt,0,sizeof(pdr_t));

		pdr_ctxt->rule_id =  generate_pdr_id();
		pdr_ctxt->prcdnc_val =  dyn_rule->precedence;
		pdr_ctxt->far.far_id_value = generate_far_id();
		pdr_ctxt->session_id = pdn->seid;
		/*to be filled in fill_sdf_rule*/
		pdr_ctxt->pdi.sdf_filter_cnt = 0;
		dyn_rule->pdr[i] = pdr_ctxt;
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++)
		{
			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL)
			{
				flow_len = dyn_rule->flow_desc[itr].flow_desc_len;
				memcpy(&(pdr_ctxt->pdi.sdf_filter[pdr_ctxt->pdi.sdf_filter_cnt].flow_desc),
						&(dyn_rule->flow_desc[itr].sdf_flow_description),
						flow_len);
				pdr_ctxt->pdi.sdf_filter[pdr_ctxt->pdi.sdf_filter_cnt].len_of_flow_desc = flow_len;
				pdr_ctxt->pdi.sdf_filter_cnt++;
			}
		}

		if (i == SOURCE_INTERFACE_VALUE_ACCESS) {

			if (cp_config->cp_type == PGWC) {
				pdr_ctxt->pdi.local_fteid.teid = bearer->s5s8_pgw_gtpu_teid;
				pdr_ctxt->pdi.local_fteid.ipv4_address =
						bearer->s5s8_pgw_gtpu_ipv4.s_addr;
			} else {
				pdr_ctxt->pdi.local_fteid.teid = bearer->s1u_sgw_gtpu_teid;
				pdr_ctxt->pdi.local_fteid.ipv4_address =
						bearer->s1u_sgw_gtpu_ipv4.s_addr;
			}
			pdr_ctxt->far.actions.forw = 0;

			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
		}
		else
		{
			pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;
			pdr_ctxt->pdi.local_fteid.teid = 0;
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
			pdr_ctxt->far.actions.forw = 0;
			if(cp_config->cp_type == PGWC)
			{
				pdr_ctxt->far.outer_hdr_creation.ipv4_address =
					bearer->s5s8_sgw_gtpu_ipv4.s_addr;
				pdr_ctxt->far.outer_hdr_creation.teid =
					bearer->s5s8_sgw_gtpu_teid;
				pdr_ctxt->far.dst_intfc.interface_value =
					DESTINATION_INTERFACE_VALUE_ACCESS;
			}
		}

		if(rule_action == RULE_ACTION_ADD)
		{
			bearer->pdrs[bearer->pdr_count++] = pdr_ctxt;
		}

		ret = add_pdr_entry(pdr_ctxt->rule_id, pdr_ctxt);
		if ( ret != 0) {
			clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Adding pdr entry Error: %d \n", __file__,
					__func__, __LINE__, ret);
			return -1;
		}

		pdr_ctxt->pdi.src_intfc.interface_value = i;
		strncpy((char * )pdr_ctxt->pdi.ntwk_inst.ntwk_inst, (char *)nwinst, 32);
		pdr_ctxt->qer.qer_id = bearer->qer_id[i].qer_id;
		pdr_ctxt->qer_id[0].qer_id = pdr_ctxt->qer.qer_id;
		pdr_ctxt->qer.session_id = pdn->seid;
		pdr_ctxt->qer.max_bitrate.ul_mbr = dyn_rule->qos.ul_mbr;
		pdr_ctxt->qer.max_bitrate.dl_mbr = dyn_rule->qos.dl_mbr;
		pdr_ctxt->qer.guaranteed_bitrate.ul_gbr = dyn_rule->qos.ul_gbr;
		pdr_ctxt->qer.guaranteed_bitrate.dl_gbr = dyn_rule->qos.dl_gbr;

		ret = add_qer_into_hash(&pdr_ctxt->qer);

		if(ret != 0) {
			clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Adding qer entry Error: %d \n", __file__,
					__func__, __LINE__, ret);
			return ret;
		}
		enum flow_status f_status = dyn_rule->flow_status;
		switch(f_status)
		{
			case FL_ENABLED_UPLINK:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_OPEN;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_CLOSED;
				break;

			case FL_ENABLED_DOWNLINK:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_CLOSED;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_OPEN;
				break;

			case FL_ENABLED:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_OPEN;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_OPEN;
				break;

			case FL_DISABLED:
				pdr_ctxt->qer.gate_status.ul_gate  = UL_GATE_CLOSED;
				pdr_ctxt->qer.gate_status.dl_gate  = UL_GATE_CLOSED;
				break;
			case FL_REMOVED:
				/*TODO*/
				break;
		}
	}
	return 0;
}

pdr_t *
fill_pdr_entry(ue_context_t *context, pdn_connection_t *pdn,
		eps_bearer_t *bearer, uint8_t iface, uint8_t itr)
{
	char mnc[4] = {0};
	char mcc[4] = {0};
	char nwinst[32] = {0};
	pdr_t *pdr_ctxt = NULL;
	int ret;

	if (context->serving_nw.mnc_digit_3 == 15) {
		sprintf(mnc, "0%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2);
	} else {
		sprintf(mnc, "%u%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2,
				context->serving_nw.mnc_digit_3);
	}

	sprintf(mcc, "%u%u%u", context->serving_nw.mcc_digit_1,
			context->serving_nw.mcc_digit_2,
			context->serving_nw.mcc_digit_3);

	sprintf(nwinst, "mnc%s.mcc%s", mnc, mcc);

	pdr_ctxt = rte_zmalloc_socket(NULL, sizeof(pdr_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (pdr_ctxt == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return NULL;
	}
	memset(pdr_ctxt,0,sizeof(pdr_t));

	pdr_ctxt->rule_id =  generate_pdr_id();
	pdr_ctxt->prcdnc_val =  1;
	pdr_ctxt->far.far_id_value = generate_far_id();
	pdr_ctxt->session_id = pdn->seid;
	/*to be filled in fill_sdf_rule*/
	pdr_ctxt->pdi.sdf_filter_cnt = 0;
	pdr_ctxt->pdi.src_intfc.interface_value = iface;
	strncpy((char * )pdr_ctxt->pdi.ntwk_inst.ntwk_inst, (char *)nwinst, 32);

	/* TODO: NS Add this changes after DP related changes of VS
	 * if(cp_config->cp_type != SGWC){
	 * pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;
	 * }
	 */

	if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
		/* TODO Hardcode 1 set because one PDR contain only 1 QER entry
		 * Revist again in case of multiple rule support
		 */
		pdr_ctxt->qer_id_count = 1;

		if(iface == SOURCE_INTERFACE_VALUE_ACCESS) {
				pdr_ctxt->qer_id[0].qer_id = bearer->qer_id[QER_INDEX_FOR_ACCESS_INTERFACE].qer_id;
		} else if(iface == SOURCE_INTERFACE_VALUE_CORE)
		{
			pdr_ctxt->qer_id[0].qer_id = bearer->qer_id[QER_INDEX_FOR_CORE_INTERFACE].qer_id;
		}
	}

	pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;

	if (iface == SOURCE_INTERFACE_VALUE_ACCESS) {
		pdr_ctxt->pdi.local_fteid.teid = bearer->s1u_sgw_gtpu_teid;
		pdr_ctxt->pdi.local_fteid.ipv4_address = 0;

		if ((SGWC == cp_config->cp_type) &&
				(bearer->s5s8_pgw_gtpu_ipv4.s_addr != 0) &&
				(bearer->s5s8_pgw_gtpu_teid != 0)) {
			pdr_ctxt->far.actions.forw = 2;
			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
			pdr_ctxt->far.outer_hdr_creation.ipv4_address =
				bearer->s5s8_pgw_gtpu_ipv4.s_addr;
			pdr_ctxt->far.outer_hdr_creation.teid =
				bearer->s5s8_pgw_gtpu_teid;
		} else {
			pdr_ctxt->far.actions.forw = 0;
		}

		if ((cp_config->cp_type == PGWC) ||
				(SAEGWC == cp_config->cp_type)) {
			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
		}
	} else{
		if(cp_config->cp_type == SGWC){
			pdr_ctxt->pdi.local_fteid.teid = (bearer->s5s8_sgw_gtpu_teid);
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
		}else{
			pdr_ctxt->pdi.local_fteid.teid = 0;
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
			pdr_ctxt->far.actions.forw = 0;
			if(cp_config->cp_type == PGWC){
				pdr_ctxt->far.outer_hdr_creation.ipv4_address =
					bearer->s5s8_sgw_gtpu_ipv4.s_addr;
				pdr_ctxt->far.outer_hdr_creation.teid =
					bearer->s5s8_sgw_gtpu_teid;
				pdr_ctxt->far.dst_intfc.interface_value =
					DESTINATION_INTERFACE_VALUE_ACCESS;
			}
		}
	}

	bearer->pdrs[itr] = pdr_ctxt;
	ret = add_pdr_entry(bearer->pdrs[itr]->rule_id, bearer->pdrs[itr]);
	if ( ret != 0) {
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Adding pdr entry Error: %d \n", __file__,
				__func__, __LINE__, ret);
		return NULL;
	}
	return pdr_ctxt;
}

eps_bearer_t* get_default_bearer(pdn_connection_t *pdn)
{
	return pdn->eps_bearers[pdn->default_bearer_id - 5];

}

eps_bearer_t* get_bearer(pdn_connection_t *pdn, bearer_qos_ie *qos)
{
	eps_bearer_t *bearer = NULL;
	for(uint8_t idx = 0; idx < MAX_BEARERS; idx++)
	{
		bearer = pdn->eps_bearers[idx];
		if(bearer != NULL)
		{
			/* Comparing each member in arp */
			if((bearer->qos.qci == qos->qci) &&
				(bearer->qos.arp.preemption_vulnerability == qos->arp.preemption_vulnerability) &&
				(bearer->qos.arp.priority_level == qos->arp.priority_level) &&
				(bearer->qos.arp.preemption_capability == qos->arp.preemption_capability))
			{
				return bearer;
			}
		}

	}
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
fill_pfcp_sess_est_req( pfcp_sess_estab_req_t *pfcp_sess_est_req,
		pdn_connection_t *pdn, uint32_t seq)
{
	/*TODO :generate seid value and store this in array
	  to send response from cp/dp , first check seid is there in array or not if yes then
	  fill that seid in response and if not then seid =0 */

	int ret = 0;
	eps_bearer_t *bearer = NULL;
	upf_context_t *upf_ctx = NULL;

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
			&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return;
	}
	assert(upf_ctx != NULL);

	memset(pfcp_sess_est_req,0,sizeof(pfcp_sess_estab_req_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_est_req->header), PFCP_SESSION_ESTABLISHMENT_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_est_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	char pAddr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(pAddr);

	set_node_id(&(pfcp_sess_est_req->node_id), node_value);

	set_fseid(&(pfcp_sess_est_req->cp_fseid), pdn->seid, node_value);

	if ((cp_config->cp_type == PGWC) ||
		(SAEGWC == cp_config->cp_type))
	{
		printf("[%s]-%d. Num Rules in pdn policy  = %d \n",__FUNCTION__,__LINE__,pdn->policy.num_charg_rule_install);
		pfcp_sess_est_req->create_pdr_count = pdn->policy.num_charg_rule_install * 2;
		/*
		 * For pgw create pdf, far and qer while handling pfcp messages
		 */
		for (int idx=0; idx <  pdn->policy.num_charg_rule_install; idx++)
		{
			bearer = NULL;
			if(compare_default_bearer_qos(&pdn->policy.default_bearer_qos, &pdn->policy.pcc_rule[idx].dyn_rule.qos) == 0)
			{
				printf("[%s]-%d. default bearer \n",__FUNCTION__,__LINE__);
				/*
				 * This means this Dynamic rule going to install in default bearer
				 */

				bearer = get_default_bearer(pdn);
			}
			else
			{
				uint8_t bearer_id = 0;
				/*
				 * dedicated bearer
				 */
				bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx].dyn_rule.qos);
				if(bearer == NULL)
				{
					/*
					 * create dedicated bearer
					 */
					bearer = rte_zmalloc_socket(NULL, sizeof(eps_bearer_t),
							RTE_CACHE_LINE_SIZE, rte_socket_id());
					if(bearer == NULL)
					{
						clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate bearer "
								"structure: %s (%s:%d)\n",
								rte_strerror(rte_errno),
								 __FILE__,
								  __LINE__);
						return;
						/* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
					}
					bzero(bearer,  sizeof(eps_bearer_t));
					bearer->pdn = pdn;
					bearer_id = get_new_bearer_id(pdn);
					pdn->eps_bearers[bearer_id] = bearer;
					pdn->context->eps_bearers[bearer_id] = bearer;
					pdn->num_bearer++;
					set_s5s8_pgw_gtpu_teid_using_pdn(bearer, pdn);
					fill_dedicated_bearer_info(bearer, pdn->context, pdn);
					memcpy(&(bearer->qos), &(pdn->policy.pcc_rule[idx].dyn_rule.qos), sizeof(bearer_qos_ie));
				}

			}
			if (cp_config->cp_type == SGWC) {
				set_s1u_sgw_gtpu_teid(bearer, bearer->pdn->context);
				update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, bearer->s1u_sgw_gtpu_ipv4.s_addr, SOURCE_INTERFACE_VALUE_ACCESS);
				set_s5s8_sgw_gtpu_teid(bearer, bearer->pdn->context);
				update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, bearer->s5s8_sgw_gtpu_ipv4.s_addr, SOURCE_INTERFACE_VALUE_CORE);
			} else if (cp_config->cp_type == SAEGWC) {
				set_s1u_sgw_gtpu_teid(bearer, bearer->pdn->context);
				update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, bearer->s1u_sgw_gtpu_ipv4.s_addr, SOURCE_INTERFACE_VALUE_ACCESS);
			} else if (cp_config->cp_type == PGWC){
				set_s5s8_pgw_gtpu_teid(bearer, bearer->pdn->context);
				update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, bearer->s5s8_pgw_gtpu_ipv4.s_addr, SOURCE_INTERFACE_VALUE_ACCESS);
			}

			bearer->dynamic_rules[bearer->num_dynamic_filters] = rte_zmalloc_socket(NULL, sizeof(dynamic_rule_t),
					RTE_CACHE_LINE_SIZE, rte_socket_id());
			if (bearer->dynamic_rules[bearer->num_dynamic_filters] == NULL)
			{
				clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate dynamic rule memory "
						"structure: %s (%s:%d)\n",
						rte_strerror(rte_errno),
						__FILE__,
						__LINE__);
				return;
				/* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
			}
			memcpy( (bearer->dynamic_rules[bearer->num_dynamic_filters]),
					&(pdn->policy.pcc_rule[idx].dyn_rule),
					sizeof(dynamic_rule_t));
			// Create 2 PDRs and 2 QERsfor every rule

			bearer->dynamic_rules[bearer->num_dynamic_filters]->pdr[0] = fill_pdr_entry(pdn->context, pdn, bearer, SOURCE_INTERFACE_VALUE_ACCESS, bearer->pdr_count++);
			bearer->qer_id[bearer->qer_count].qer_id = generate_qer_id();
			fill_qer_entry(pdn, bearer, bearer->qer_count++);

			enum flow_status f_status = bearer->dynamic_rules[bearer->num_dynamic_filters]->flow_status; // consider dynamic rule is 1 only /*TODO*/
			// assuming no of qer and pdr is same /*TODO*/
			fill_gate_status(pfcp_sess_est_req, bearer->qer_count, f_status);

			bearer->dynamic_rules[bearer->num_dynamic_filters]->pdr[1] = fill_pdr_entry(pdn->context, pdn, bearer, SOURCE_INTERFACE_VALUE_CORE, bearer->pdr_count++);
			bearer->qer_id[bearer->qer_count].qer_id = generate_qer_id();
			fill_qer_entry(pdn, bearer, bearer->qer_count++);

			f_status = bearer->dynamic_rules[bearer->num_dynamic_filters]->flow_status; // consider dynamic rule is 1 only /*TODO*/
			// assuming no of qer and pdr is same /*TODO*/
			fill_gate_status(pfcp_sess_est_req, bearer->qer_count, f_status);
			bearer->num_dynamic_filters++;
		}

		if (cp_config->cp_type == SAEGWC) {
			update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid,
					upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		} else if (cp_config->cp_type == PGWC) {
			update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid,
					upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		}
	} else {
		bearer = get_default_bearer(pdn);
		pfcp_sess_est_req->create_pdr_count = bearer->pdr_count;

		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, upf_ctx->s5s8_sgwu_ip, SOURCE_INTERFACE_VALUE_CORE);
	}

	{
		uint8_t pdr_idx =0;
		for(uint8_t i = 0; i < MAX_BEARERS; i++)
		{
			bearer = pdn->eps_bearers[i];
			if(bearer != NULL)
			{
				assert(bearer->pdr_count != 0);
				for(uint8_t idx = 0; idx < bearer->pdr_count; idx++)
				{
					pfcp_sess_est_req->create_pdr[pdr_idx].qer_id_count = 1;
					creating_pdr(&(pfcp_sess_est_req->create_pdr[pdr_idx]), bearer->pdrs[idx]->pdi.src_intfc.interface_value);
					pfcp_sess_est_req->create_far_count++;
					creating_far(&(pfcp_sess_est_req->create_far[pdr_idx]));
					pdr_idx++;
				}
			}
		}
	}

	/* SGW Relocation Case*/
	/*SP: Need to think of this*/
	//if(context->indication_flag.oi != 0) {
	//pfcp_sess_est_req->create_far_count = 2;
	//}

	{
		uint8_t pdr_idx =0;
		for(uint8_t i = 0; i < MAX_BEARERS; i++)
		{
			bearer = pdn->eps_bearers[i];
			if(bearer != NULL)
			{
				for(uint8_t idx = 0; idx < MAX_PDR_PER_RULE; idx++)
				{
					assert(bearer->pdrs[idx] != NULL);
					pfcp_sess_est_req->create_pdr[pdr_idx].pdr_id.rule_id  =
						bearer->pdrs[idx]->rule_id;
					pfcp_sess_est_req->create_pdr[pdr_idx].precedence.prcdnc_val =
						bearer->pdrs[idx]->prcdnc_val;

					if(((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) &&
							(bearer->pdrs[idx]->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE))
					{
						uint32_t size_teid = 0;
						size_teid = pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.header.len +
							sizeof(pfcp_ie_header_t);
						// TODO : check twice reduce teid size needed ?
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len =
							pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len - size_teid;
						pfcp_sess_est_req->create_pdr[pdr_idx].header.len =
							pfcp_sess_est_req->create_pdr[pdr_idx].header.len - size_teid;
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.header.len = 0;

					}
					else
					{
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.teid =
							bearer->pdrs[idx]->pdi.local_fteid.teid;
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.ipv4_address =
							bearer->pdrs[idx]->pdi.local_fteid.ipv4_address;
					}
					if((cp_config->cp_type == SGWC) ||
							(bearer->pdrs[idx]->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS)){
						/*No need to send ue ip and network instance for pgwc access interface or
						 * for any sgwc interface */
						uint32_t size_ie = 0;
						size_ie = pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.header.len +
							sizeof(pfcp_ie_header_t);
						size_ie = size_ie + pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.header.len +
							sizeof(pfcp_ie_header_t);
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len =
							pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len - size_ie;
						pfcp_sess_est_req->create_pdr[pdr_idx].header.len =
							pfcp_sess_est_req->create_pdr[pdr_idx].header.len - size_ie;
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.header.len = 0;
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.header.len = 0;
					}else{
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.ipv4_address =
							bearer->pdrs[idx]->pdi.ue_addr.ipv4_address;
						strncpy((char *)pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.ntwk_inst,
								(char *)&bearer->pdrs[idx]->pdi.ntwk_inst.ntwk_inst, 32);
					}

					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.src_intfc.interface_value =
						bearer->pdrs[idx]->pdi.src_intfc.interface_value;

					pfcp_sess_est_req->create_far[pdr_idx].far_id.far_id_value =
						bearer->pdrs[idx]->far.far_id_value;
					 pfcp_sess_est_req->create_pdr[pdr_idx].far_id.far_id_value =
						 bearer->pdrs[idx]->far.far_id_value;

					/* SGW Relocation*/
					if(pdn->context->indication_flag.oi != 0) {
						uint8_t len  = 0;
						//if(itr == 1)
						//TODO :: Betting on stars allignments to make below code works
						if(pdr_idx%2)
						{
							pfcp_sess_est_req->create_far[pdr_idx].apply_action.forw = PRESENT;
							len += set_forwarding_param(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms));
							len += UPD_PARAM_HEADER_SIZE;
							pfcp_sess_est_req->create_far[pdr_idx].header.len += len;

							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.ipv4_address =
								bearer->s1u_enb_gtpu_ipv4.s_addr;

							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.teid =
								bearer->s1u_enb_gtpu_teid;
							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
								DESTINATION_INTERFACE_VALUE_ACCESS;

						}
						//else if(itr == 0)
					    else
						{
							pfcp_sess_est_req->create_far[pdr_idx].apply_action.forw = PRESENT;
							len += set_forwarding_param(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms));
							len += UPD_PARAM_HEADER_SIZE;
							pfcp_sess_est_req->create_far[pdr_idx].header.len += len;

							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.ipv4_address =
								bearer->s5s8_pgw_gtpu_ipv4.s_addr;
							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.teid =
								bearer->s5s8_pgw_gtpu_teid;
							pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
								DESTINATION_INTERFACE_VALUE_CORE;
						}
					}
					if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
						pfcp_sess_est_req->create_pdr[pdr_idx].qer_id_count =
							bearer->pdrs[idx]->qer_id_count;
						for(int itr1 = 0; itr1 < bearer->pdrs[idx]->qer_id_count; itr1++) {
							pfcp_sess_est_req->create_pdr[pdr_idx].qer_id[itr1].qer_id_value =
								//bearer->pdrs[idx]->qer_id[itr1].qer_id;
								bearer->qer_id[idx].qer_id;
						}
					}

					if ((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) {
						pfcp_sess_est_req->create_far[pdr_idx].apply_action.forw = PRESENT;
						if (pfcp_sess_est_req->create_far[pdr_idx].apply_action.forw == PRESENT) {
							uint16_t len = 0;

							if ((SAEGWC == cp_config->cp_type) ||
									(SOURCE_INTERFACE_VALUE_ACCESS == bearer->pdrs[idx]->pdi.src_intfc.interface_value)) {
								set_destination_interface(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc));
								pfcp_set_ie_header(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.header),
										PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

								pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

								len += sizeof(pfcp_dst_intfc_ie_t);
								len += UPD_PARAM_HEADER_SIZE;

								pfcp_sess_est_req->create_far[pdr_idx].header.len += len;
								pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
									bearer->pdrs[pdr_idx]->far.dst_intfc.interface_value;
							} else {
								len += set_forwarding_param(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms));
								/* Currently take as hardcoded value */
								len += UPD_PARAM_HEADER_SIZE;
								pfcp_sess_est_req->create_far[pdr_idx].header.len += len;

								pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.ipv4_address =
									bearer->pdrs[pdr_idx]->far.outer_hdr_creation.ipv4_address;
								pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.teid =
									bearer->pdrs[pdr_idx]->far.outer_hdr_creation.teid;
								pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
									bearer->pdrs[pdr_idx]->far.dst_intfc.interface_value;
							}
						}
					}

					pdr_idx++;
				}
			}
		}
	} /*for loop*/

	{
		uint8_t qer_idx = 0;
		qer_t *qer_context;
		if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC)
		{
			for(uint8_t i = 0; i < MAX_BEARERS; i++)
			{
				bearer = pdn->eps_bearers[i];
				if(bearer != NULL)
				{
					pfcp_sess_est_req->create_qer_count += bearer->qer_count;
						for(uint8_t idx = 0; idx < bearer->qer_count; idx++)
						{
							creating_qer(&(pfcp_sess_est_req->create_qer[qer_idx]));
							pfcp_sess_est_req->create_qer[qer_idx].qer_id.qer_id_value  =
								bearer->qer_id[qer_idx].qer_id;
							qer_context = get_qer_entry(pfcp_sess_est_req->create_qer[qer_idx].qer_id.qer_id_value);
							/* Assign the value from the PDR */
							if(qer_context){
								//pfcp_sess_est_req->create_pdr[qer_idx].qer_id[0].qer_id_value =
								//	pfcp_sess_est_req->create_qer[qer_idx].qer_id.qer_id_value;
								pfcp_sess_est_req->create_qer[qer_idx].maximum_bitrate.ul_mbr  =
									qer_context->max_bitrate.ul_mbr;
								pfcp_sess_est_req->create_qer[qer_idx].maximum_bitrate.dl_mbr  =
									qer_context->max_bitrate.dl_mbr;
								pfcp_sess_est_req->create_qer[qer_idx].guaranteed_bitrate.ul_gbr  =
									qer_context->guaranteed_bitrate.ul_gbr;
								pfcp_sess_est_req->create_qer[qer_idx].guaranteed_bitrate.dl_gbr  =
									qer_context->guaranteed_bitrate.dl_gbr;
							}
							qer_idx++;
						}
				}
			}
		}
	}

		uint8_t pdr_idx = 0;
		for(int itr1 = 0; itr1 <  pdn->policy.num_charg_rule_install; itr1++) {
			/*
			 * call fill_sdf_rules twice because one rule create 2 PDRs
			 */
			enum flow_status f_status = pdn->policy.pcc_rule[itr1].dyn_rule.flow_status;

			fill_sdf_rules(pfcp_sess_est_req, &pdn->policy.pcc_rule[itr1].dyn_rule, pdr_idx);
			fill_gate_status(pfcp_sess_est_req, pdr_idx, f_status);
			pdr_idx++;

			fill_sdf_rules(pfcp_sess_est_req, &pdn->policy.pcc_rule[itr1].dyn_rule, pdr_idx);
			fill_gate_status(pfcp_sess_est_req, pdr_idx, f_status);
			pdr_idx++;

		}


	/* VS: Set the pdn connection type */
	set_pdn_type(&(pfcp_sess_est_req->pdn_type), &(pdn->pdn_type));

#if 0
	creating_bar(&(pfcp_sess_est_req->create_bar));

	char sgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	unsigned long sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_est_req->sgw_c_fqcsid), sgwc_value);

	char mme_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config.s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_est_req->mme_fqcsid), mme_value);

	char pgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_est_req->pgw_c_fqcsid), pgwc_value);

	//TODO : IP addres for epdgg is hardcoded
	const char* epdg_addr = "0.0.0.0";
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_est_req->epdg_fqcsid), epdg_value);

	//TODO : IP addres for twan is hardcoded
	const char* twan_addr = "0.0.0.0";
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_est_req->twan_fqcsid), twan_value);

	set_up_inactivity_timer(&(pfcp_sess_est_req->user_plane_inact_timer));

	set_user_id(&(pfcp_sess_est_req->user_id));
#endif

	if (upf_ctx->up_supp_features & UP_TRACE)
		set_trace_info(&(pfcp_sess_est_req->trc_info));

    pfcp_sess_est_req->create_urr_count = 1;
    creating_urr(&pfcp_sess_est_req->create_urr[0]);
}


int
fill_dedicated_bearer_info(eps_bearer_t *bearer,
		ue_context_t *context, pdn_connection_t *pdn)
{
	int ret = 0;
	upf_context_t *upf_ctx = NULL;

	bearer->s5s8_sgw_gtpu_ipv4.s_addr = context->eps_bearers[pdn->default_bearer_id - 5]->s5s8_sgw_gtpu_ipv4.s_addr;

	/* TODO: Revisit this for change in yang*/
    if(cp_config->gx_enabled) {
        if (cp_config->cp_type != SGWC){
            bearer->qer_count = NUMBER_OF_QER_PER_BEARER;
            for(uint8_t itr=0; itr < bearer->qer_count; itr++){
                bearer->qer_id[itr].qer_id = generate_qer_id();
                fill_qer_entry(pdn, bearer, itr);
            }
        }
    }

	/*SP: As per discussion Per bearer two pdrs and fars will be there*/
	/************************************************
	 *  cp_type  count      FTEID_1        FTEID_2 *
	 *************************************************
	 SGWC         2      s1u  SGWU      s5s8 SGWU
	 PGWC         2      s5s8 PGWU          NA
	 SAEGWC       2      s1u SAEGWU         NA
	 ************************************************/

	if (cp_config->cp_type == SGWC){
	bearer->pdr_count = NUMBER_OF_PDR_PER_BEARER;
	for(uint8_t itr=0; itr < bearer->pdr_count; itr++){
		switch(itr){
			case SOURCE_INTERFACE_VALUE_ACCESS:
				fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_ACCESS, itr);
				break;
			case SOURCE_INTERFACE_VALUE_CORE:
				fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_CORE, itr);
				break;
			default:
				break;
		}
	}
	}

	bearer->pdn = pdn;

	ret = upf_context_entry_lookup((pdn->upf_ipv4.s_addr), &(upf_ctx));
	if (ret < 0) {
		clLog(sxlogger, eCLSeverityDebug, "%s:%d NO ENTRY FOUND IN UPF HASH [%u]\n",
			__func__, __LINE__, (pdn->upf_ipv4.s_addr));
		return GTPV2C_CAUSE_INVALID_PEER;
	}

	if (cp_config->cp_type == SGWC) {
		bearer->s5s8_sgw_gtpu_ipv4.s_addr = upf_ctx->s5s8_sgwu_ip;
		bearer->s1u_sgw_gtpu_ipv4.s_addr = upf_ctx->s1u_ip;

		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);

		set_s5s8_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, upf_ctx->s5s8_sgwu_ip, SOURCE_INTERFACE_VALUE_CORE);
	}else if (cp_config->cp_type == SAEGWC) {
		bearer->s1u_sgw_gtpu_ipv4.s_addr = upf_ctx->s1u_ip;
		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	} else if (cp_config->cp_type == PGWC) {
		bearer->s5s8_pgw_gtpu_ipv4.s_addr = upf_ctx->s5s8_pgwu_ip;

		set_s5s8_pgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	}

	RTE_SET_USED(context);
	return 0;
}


void
process_pfcp_sess_est_request_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    msg_info_t *msg = calloc(1, sizeof(msg_info_t));
    SET_PROC_MSG(proc_context,msg);
    msg->proc_context = proc_context;
    msg->event = PFCP_SESS_EST_RESP_TIMEOUT_EVNT;
    SET_PROC_MSG(proc_context, msg);
    proc_context->handler(proc_context, msg); 
    return;
}

transData_t*
process_pfcp_sess_est_request(proc_context_t *proc_context, upf_context_t *upf_ctx)
{
	eps_bearer_t *bearer = NULL;
	pdn_connection_t *pdn = proc_context->pdn_context;
	ue_context_t *context = proc_context->ue_context;
	pfcp_sess_estab_req_t pfcp_sess_est_req = {0};
	uint32_t sequence = 0;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

	bearer = pdn->eps_bearers[pdn->default_bearer_id - 5];
	if (cp_config->cp_type == SGWC) {
		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		set_s5s8_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, upf_ctx->s5s8_sgwu_ip, SOURCE_INTERFACE_VALUE_CORE);
	} else if (cp_config->cp_type == SAEGWC) {
		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	} else if (cp_config->cp_type == PGWC){
		set_s5s8_pgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	}

	sequence = get_pfcp_sequence_number(PFCP_SESSION_ESTABLISHMENT_REQUEST, sequence);

	/* Need to discuss with himanshu */
	if (cp_config->cp_type == PGWC) {
		/* VS: Update the PGWU IP address */
		bearer->s5s8_pgw_gtpu_ipv4.s_addr =
			htonl(upf_ctx->s5s8_pgwu_ip);

		/* Filling PDN structure*/
		pfcp_sess_est_req.pdn_type.header.type = PFCP_IE_PDN_TYPE;
		pfcp_sess_est_req.pdn_type.header.len = sizeof(uint8_t);
		pfcp_sess_est_req.pdn_type.pdn_type_spare = 0;
		pfcp_sess_est_req.pdn_type.pdn_type =  1;
	} else {
		bearer->s5s8_sgw_gtpu_ipv4.s_addr = htonl(upf_ctx->s5s8_sgwu_ip);
		bearer->s1u_sgw_gtpu_ipv4.s_addr = htonl(upf_ctx->s1u_ip);
	}

	fill_pfcp_sess_est_req(&pfcp_sess_est_req, pdn, sequence);


#ifdef USE_CSID
	/* Add the entry for peer nodes */
	if (fill_peer_node_info(pdn, bearer)) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Failed to fill peer node info and assignment of the CSID Error: %s\n",
				ERR_MSG,
				strerror(errno));
		return NULL;
	}

	/* Add entry for cp session id with link local csid */
	sess_csid *tmp = NULL;
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
		tmp = get_sess_csid_entry(
				(context->sgw_fqcsid)->local_csid[(context->sgw_fqcsid)->num_csid - 1]);
	} else {
		/* PGWC */
		tmp = get_sess_csid_entry(
				(context->pgw_fqcsid)->local_csid[(context->pgw_fqcsid)->num_csid - 1]);
	}

	if (tmp == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
				strerror(errno));
		return NULL;
	}

	/* Link local csid with session id */
	tmp->cp_seid[tmp->seid_cnt++] = pdn->seid;

	/* Fill the fqcsid into the session est request */
	if (fill_fqcsid_sess_est_req(&pfcp_sess_est_req, context)) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Failed to fill FQ-CSID in Sess EST Req ERROR: %s\n",
				ERR_MSG,
				strerror(errno));
		return NULL;
	}
#endif /* USE_CSID */

	/* Update UE State */
	bearer->pdn->state = PFCP_SESS_EST_REQ_SNT_STATE;

	/* Set create session response */
	//if (cp_config->cp_type == PGWC)
	//	resp->sequence = (htonl(context->sequence) >> 8);
	//else
	//	resp->sequence = context->sequence;


	//proc->eps_bearer_id = pdn->default_bearer_id - 5;
	//resp->s11_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
	//resp->context = context;
	//proc_context->msg_type = GTP_CREATE_SESSION_REQ;
	proc_context->state = PFCP_SESS_EST_REQ_SNT_STATE;
	//proc->proc = context->pdns[pdn->default_bearer_id - 5]->proc;

    transData_t *trans_entry = NULL;
	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_estab_req_t(&pfcp_sess_est_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error sending: %i\n",
				__func__, __LINE__, errno);
		return NULL;
	} 

    /*pfcp-session-estab-req-sent*/
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSESTREQ,GET_UPF_ADDR(context->upf_context)); 

    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, process_pfcp_sess_est_request_timeout);
    trans_entry->sequence = sequence;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  

    if (add_sess_entry_seid(pdn->seid, context) != 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to add response in entry in SM_HASH\n",
                __func__, __LINE__);
        assert(0);
    }

    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	return trans_entry;
}


int8_t
process_pfcp_sess_est_resp(msg_info_t *msg, 
                           pfcp_sess_estab_rsp_t *pfcp_sess_est_rsp, 
                           gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint64_t sess_id = pfcp_sess_est_rsp->header.seid_seqno.has_seid.seid;
	uint64_t dp_sess_id = pfcp_sess_est_rsp->up_fseid.seid;
	uint32_t teid = UE_SESS_ID(sess_id);

	printf("%s %d - sess_id %lu teid = %u \n", __FUNCTION__, __LINE__, sess_id, teid);
    proc_context_t *proc_context;

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
    assert(context != NULL);
    proc_context = msg->proc_context; 
    assert(proc_context != NULL);
	proc_context->state = PFCP_SESS_EST_RESP_RCVD_STATE;

#ifdef USE_CSID
	if (cp_config->cp_type == PGWC) {
		memcpy(&pfcp_sess_est_rsp->pgw_u_fqcsid,
			&pfcp_sess_est_rsp->sgw_u_fqcsid, sizeof(pfcp_fqcsid_ie_t));
		memset(&pfcp_sess_est_rsp->sgw_u_fqcsid, 0, sizeof(pfcp_fqcsid_ie_t));
	}

	fqcsid_t *tmp = NULL;
	fqcsid_t *fqcsid = NULL;
	fqcsid = rte_zmalloc_socket(NULL, sizeof(fqcsid_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (fqcsid == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Failed to allocate the memory for fqcsids entry\n",
				ERR_MSG);
		return -1;
	}
	/* SGW FQ-CSID */
	if (pfcp_sess_est_rsp->sgw_u_fqcsid.header.len) {
		/* Stored the SGW CSID by SGW Node address */
		tmp = get_peer_addr_csids_entry(pfcp_sess_est_rsp->sgw_u_fqcsid.node_address,
				ADD);

		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = pfcp_sess_est_rsp->sgw_u_fqcsid.node_address;

		for(uint8_t itr = 0; itr < pfcp_sess_est_rsp->sgw_u_fqcsid.number_of_csids; itr++) {
			uint8_t match = 0;
			for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
				if (tmp->local_csid[itr1] == pfcp_sess_est_rsp->sgw_u_fqcsid.pdn_conn_set_ident[itr]) {
					match = 1;
				}
			}
			if (!match) {
				tmp->local_csid[tmp->num_csid++] =
					pfcp_sess_est_rsp->sgw_u_fqcsid.pdn_conn_set_ident[itr];
			}
		}
		memcpy(fqcsid, tmp, sizeof(fqcsid_t));
	} else {
		if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
			tmp = get_peer_addr_csids_entry(pfcp_sess_est_rsp->up_fseid.ipv4_address,
					ADD);
			if (tmp == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
						strerror(errno));
				return -1;
			}
			tmp->node_addr = pfcp_sess_est_rsp->sgw_u_fqcsid.node_address;
			memcpy(fqcsid, tmp, sizeof(fqcsid_t));
		}
	}

	/* PGW FQ-CSID */
	if (pfcp_sess_est_rsp->pgw_u_fqcsid.header.len) {
		/* Stored the PGW CSID by PGW Node address */
		tmp = get_peer_addr_csids_entry(pfcp_sess_est_rsp->pgw_u_fqcsid.node_address,
				ADD);

		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = pfcp_sess_est_rsp->pgw_u_fqcsid.node_address;

		for(uint8_t itr = 0; itr < pfcp_sess_est_rsp->pgw_u_fqcsid.number_of_csids; itr++) {
			uint8_t match = 0;
			for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
				if (tmp->local_csid[itr1] == pfcp_sess_est_rsp->pgw_u_fqcsid.pdn_conn_set_ident[itr]) {
					match = 1;
				}
			}
			if (!match) {
				tmp->local_csid[tmp->num_csid++] =
					pfcp_sess_est_rsp->pgw_u_fqcsid.pdn_conn_set_ident[itr];
			}
		}
		memcpy(fqcsid, tmp, sizeof(fqcsid_t));
	} else {
		if (cp_config->cp_type == PGWC) {
			tmp = get_peer_addr_csids_entry(pfcp_sess_est_rsp->up_fseid.ipv4_address,
					ADD);
			if (tmp == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
						strerror(errno));
				return -1;
			}
			tmp->node_addr = pfcp_sess_est_rsp->pgw_u_fqcsid.node_address;
			memcpy(fqcsid, tmp, sizeof(fqcsid_t));
		}
	}

	/* TODO: Add the handling if SGW or PGW not support Partial failure */
	/* Link peer node SGW or PGW csid with local csid */
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
		ret = update_peer_csid_link(fqcsid, context->sgw_fqcsid);
	} else if (cp_config->cp_type == PGWC) {
		ret = update_peer_csid_link(fqcsid, context->pgw_fqcsid);
	}

	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
				strerror(errno));
		return -1;
	}

	/* Update entry for up session id with link local csid */
	sess_csid *sess_t = NULL;
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
		if (context->sgw_fqcsid) {
			sess_t = get_sess_csid_entry(
					(context->sgw_fqcsid)->local_csid[(context->sgw_fqcsid)->num_csid - 1]);
		}
	} else {
		/* PGWC */
		if (context->pgw_fqcsid) {
			sess_t = get_sess_csid_entry(
					(context->pgw_fqcsid)->local_csid[(context->pgw_fqcsid)->num_csid - 1]);
		}
	}

	if (sess_t == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
				strerror(errno));
		return -1;
	}

	/* Link local csid with session id */
	sess_t->up_seid[sess_t->seid_cnt - 1] = dp_sess_id;

	/* Update the UP CSID in the context */
	context->up_fqcsid = fqcsid;

#endif /* USE_CSID */

    pdn = proc_context->pdn_context;
    assert(pdn != NULL);
    assert(sess_id == pdn->seid);
    uint8_t ebi_index = pdn->default_bearer_id - 5; 
    assert(context->eps_bearers[ebi_index] != NULL);
	pdn = context->eps_bearers[ebi_index]->pdn;
    assert(proc_context->pdn_context == pdn);
	bearer = context->eps_bearers[ebi_index];
    assert(bearer != NULL);
	pdn->dp_seid = dp_sess_id;
    struct in_addr temp_addr = pdn->ipv4; 
    temp_addr.s_addr = htonl(temp_addr.s_addr);
	printf("%s %d DP session id %lu. UE address = %s  \n", __FUNCTION__, __LINE__, dp_sess_id, inet_ntoa(temp_addr));

	/* Update the UE state */
	pdn->state = PFCP_SESS_EST_RESP_RCVD_STATE;

	if (cp_config->cp_type == SAEGWC) {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
		set_create_session_response(
				gtpv2c_tx, gtpc_trans->sequence, context, pdn, bearer);
	}
    else if (cp_config->cp_type == PGWC) {
		/*TODO: This needs to be change after support libgtpv2 on S5S8*/
		/* set_pgwc_s5s8_create_session_response(gtpv2c_tx,
				(htonl(context->sequence) >> 8), pdn, bearer); */

		create_sess_rsp_t cs_resp = {0};

		//uint32_t  seq_no = 0;
		//seq_no = bswap_32(resp->sequence);
		//seq_no = seq_no >> 8;
        uint32_t sequence = 0; // TODO...use transactions 
		fill_pgwc_create_session_response(&cs_resp,
			sequence, context, ebi_index);

#ifdef USE_CSID
		if ((context->pgw_fqcsid)->num_csid) {
			set_gtpc_fqcsid_t(&cs_resp.pgw_fqcsid, IE_INSTANCE_ZERO,
					context->pgw_fqcsid);
		}
#endif /* USE_CSID */

		uint16_t msg_len = encode_create_sess_rsp(&cs_resp, (uint8_t*)gtpv2c_tx);
		msg_len = msg_len - 4;
		gtpv2c_header_t *header;
		header = (gtpv2c_header_t*) gtpv2c_tx;
			header->gtpc.message_len = htons(msg_len);

		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
						htonl(pdn->s5s8_sgw_gtpc_ipv4.s_addr);
	} else if (cp_config->cp_type == SGWC) {
		uint16_t msg_len = 0;
		upf_context_t *upf_context = NULL;

		ret = upf_context_entry_lookup(((context->pdns[ebi_index])->upf_ipv4.s_addr),
				&(upf_context));

		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityDebug, "%s:%d NO ENTRY FOUND IN UPF HASH [%u]\n", __func__,
					__LINE__, (context->pdns[ebi_index])->upf_ipv4.s_addr);
			return GTPV2C_CAUSE_INVALID_PEER;
		}

		ret = add_bearer_entry_by_sgw_s5s8_tied(pdn->s5s8_sgw_gtpc_teid, &context->eps_bearers[ebi_index]);
		if(ret) {
			return ret;
		}

		if(context->indication_flag.oi == 1) {

			memset(gtpv2c_tx, 0, MAX_GTPV2C_UDP_LEN);
			set_modify_bearer_request(gtpv2c_tx, pdn, bearer);

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);

			//resp->state = MBR_REQ_SNT_STATE;
			//pdn->state = resp->state;
			//pdn->proc = SGW_RELOCATION_PROC;
			return 0;
		}

		/*Add procedure based call here
		 * for pdn -> CSR
		 * for sgw relocation -> MBR
		 */

		create_sess_req_t cs_req = {0};

		ret = fill_cs_request(&cs_req, context, ebi_index);

#ifdef USE_CSID
		/* Set the SGW FQ-CSID */
		if ((context->sgw_fqcsid)->num_csid) {
			set_gtpc_fqcsid_t(&cs_req.sgw_fqcsid, IE_INSTANCE_ONE,
					context->sgw_fqcsid);
			cs_req.sgw_fqcsid.node_address = ntohl(cp_config->s5s8_ip.s_addr);
		}

		/* Set the MME FQ-CSID */
		if ((context->mme_fqcsid)->num_csid) {
			set_gtpc_fqcsid_t(&cs_req.mme_fqcsid, IE_INSTANCE_ZERO,
					context->mme_fqcsid);
		}
#endif /* USE_CSID */
		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityDebug, "%s:Failed to create the CSR request \n", __func__);
			return 0;
		}

		msg_len = encode_create_sess_req(
				&cs_req,
				(uint8_t*)gtpv2c_tx);

		msg_len = msg_len - 4;
		gtpv2c_header_t *header;
		header = (gtpv2c_header_t*) gtpv2c_tx;
		header->gtpc.message_len = htons(msg_len);

		if (ret < 0)
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to generate S5S8 SGWC CSR.\n",
					__func__, __LINE__);

		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(context->pdns[ebi_index]->s5s8_pgw_gtpc_ipv4.s_addr);

#ifdef TEMP
		/* Update the session state */
		resp->state = CS_REQ_SNT_STATE;
		/* stored teid in csr header for clean up */
		resp->gtpc_msg.csr.header.teid.has_teid.teid = context->pdns[ebi_index]->s5s8_sgw_gtpc_teid;
#endif

		/* Update the UE state */
		pdn->state = CS_REQ_SNT_STATE;
		return 0;
	}

    increment_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);

	/* Update the UE state */
	pdn->state = CONNECTED_STATE;
	return 0;
}



#ifdef FUTURE_NEED
// some of the required code is rleady moved under service request and rab release procedure 
uint8_t
process_pfcp_sess_mod_resp(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
    RTE_SET_USED(gtpv2c_tx);
	int ret = 0;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);

	/* Retrive the session information based on session id. */
	if (get_sess_entry_seid(sess_id, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, sess_id);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					teid);
	}

	ebi_index = UE_BEAR_ID(sess_id) - 5;
	bearer = context->eps_bearers[ebi_index];
	/* Update the UE state */
	pdn = GET_PDN(context, ebi_index);
	pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s:%d Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if (resp->msg_type == GTP_MODIFY_BEARER_REQ) 
    {
		/* Fill the modify bearer response */
		set_modify_bearer_response(gtpv2c_tx,
				context->sequence, context, bearer);

		/* Update the UE state */
		pdn->state = CONNECTED_STATE;

		/* Update the next hop IP address */
		if (PGWC != cp_config->cp_type) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		}
		return 0;

	} else if (resp->msg_type == GTP_CREATE_SESSION_RSP) {
		/* Fill the Create session response */
		set_create_session_response(
				gtpv2c_tx, context->sequence, context, bearer->pdn, bearer);

	} else if (resp->msg_type == GX_RAR_MSG) {
		uint8_t ebi = 0;
		get_bearer_info_install_rules(pdn, &ebi);
		bearer = context->eps_bearers[ebi];
		if (!bearer) {
			clLog(clSystemLog, eCLSeverityCritical,
				"%s:%d Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
		}

		/* TODO: NC Need to remove hard coded pti value */
		set_create_bearer_request(gtpv2c_tx, context->sequence, context,
				bearer, pdn->default_bearer_id, 0, NULL, 0);

		resp->state = CREATE_BER_REQ_SNT_STATE;
		pdn->state = CREATE_BER_REQ_SNT_STATE;

		if (SAEGWC == cp_config->cp_type) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		} else {
		    my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(bearer->pdn->s5s8_sgw_gtpc_ipv4.s_addr);
		}

		return 0;
	} else if (resp->msg_type == GTP_CREATE_BEARER_REQ) {
		bearer = context->eps_bearers[resp->eps_bearer_id - 5];
		set_create_bearer_request(
				gtpv2c_tx, context->sequence, context, bearer,
				pdn->default_bearer_id, 0, resp->eps_bearer_lvl_tft, resp->tft_header_len);

		resp->state = CREATE_BER_REQ_SNT_STATE;
		pdn->state = CREATE_BER_REQ_SNT_STATE;

		s11_mme_sockaddr.sin_addr.s_addr =
					htonl(context->s11_mme_gtpc_ipv4.s_addr);

		return 0;

	} else if (resp->msg_type == GTP_CREATE_BEARER_RSP) {
		if ((SAEGWC == cp_config->cp_type) || (PGWC == cp_config->cp_type)) {
			gen_reauth_response(context, resp->eps_bearer_id - 5);
			struct sockaddr_in saddr_in;
			saddr_in.sin_family = AF_INET;
			inet_aton("127.0.0.1", &(saddr_in.sin_addr));
            increment_gx_peer_stats(MSG_TX_DIAMETER_GX_RAA, saddr_in.sin_addr.s_addr);

			resp->msg_type = GX_RAA_MSG;
			resp->state = CONNECTED_STATE;
			pdn->state = CONNECTED_STATE;

			return 0;
		} else {
			bearer = context->eps_bearers[resp->eps_bearer_id - 5];
			set_create_bearer_response(
				gtpv2c_tx, context->sequence, context, bearer, resp->eps_bearer_id, 0);

			resp->state = CONNECTED_STATE;
			pdn->state = CONNECTED_STATE;

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(context->pdns[0]->s5s8_pgw_gtpc_ipv4.s_addr);

			return 0;
		}
	} else if(resp->msg_type == GTP_DELETE_SESSION_REQ){
		if (cp_config->cp_type == SGWC) {
			uint8_t encoded_msg[512];

			/* Indication flags not required in DSR for PGWC */
			resp->gtpc_msg.dsr.indctn_flgs.header.len = 0;
			encode_del_sess_req(
					(del_sess_req_t *)&(resp->gtpc_msg.dsr),
					encoded_msg);

			gtpv2c_header_t *header;
			header =(gtpv2c_header_t*) encoded_msg;

			ret =
				gen_sgwc_s5s8_delete_session_request((gtpv2c_header_t *)encoded_msg,
						gtpv2c_tx, htonl(bearer->pdn->s5s8_pgw_gtpc_teid),
						header->teid.has_teid.seq,
						resp->eps_bearer_id);

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				resp->s5s8_pgw_gtpc_ipv4;

			/* Update the session state */
			resp->state = DS_REQ_SNT_STATE;

			/* Update the UE state */
			ret = update_ue_state(context->s11_sgw_gtpc_teid,
					DS_REQ_SNT_STATE, resp->eps_bearer_id - 5);
			if (ret < 0) {
				clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State.\n", __func__);
			}

			clLog(sxlogger, eCLSeverityDebug, "SGWC:%s: "
					"s5s8_recv_sockaddr.sin_addr.s_addr :%s\n", __func__,
					inet_ntoa(*((struct in_addr *)&my_sock.s5s8_recv_sockaddr.sin_addr.s_addr)));

			return ret;
		}
	} else {
		/* Fill the release bearer response */
		set_release_access_bearer_response(gtpv2c_tx,
				context->sequence, context->s11_mme_gtpc_teid);

		/* Update the session state */
		resp->state = IDEL_STATE;

		/* Update the UE state */
		pdn->state = IDEL_STATE;

		s11_mme_sockaddr.sin_addr.s_addr =
						htonl(context->s11_mme_gtpc_ipv4.s_addr);

		clLog(sxlogger, eCLSeverityDebug, "%s:%d s11_mme_sockaddr.sin_addr.s_addr :%s\n",
				__func__, __LINE__,
				inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));

		return 0;
	}

	/* Update the session state */
	resp->state = CONNECTED_STATE;

	/* Update the UE state */
	pdn->state = CONNECTED_STATE;

	s11_mme_sockaddr.sin_addr.s_addr =
					htonl(context->s11_mme_gtpc_ipv4.s_addr);

	clLog(sxlogger, eCLSeverityDebug, "%s:%d s11_mme_sockaddr.sin_addr.s_addr :%s\n", __func__,
				__LINE__, inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));

	return 0;
}
#endif


int
del_rule_entries(ue_context_t *context, uint8_t ebi_index)
{
	int ret = 0;
	pdr_t *pdr_ctx =  NULL;

	/*Delete all pdr, far, qer entry from table */
    if(cp_config->gx_enabled) {
        for(uint8_t itr = 0; itr < context->eps_bearers[ebi_index]->qer_count ; itr++) {
            if( del_qer_entry(context->eps_bearers[ebi_index]->qer_id[itr].qer_id) != 0 ){
                clLog(clSystemLog, eCLSeverityCritical,
                        "%s %s %d %s - Error del_pdr_entry deletion\n",__file__,
                        __func__, __LINE__, strerror(ret));
            }
        }
    }
	for(uint8_t itr = 0; itr < context->eps_bearers[ebi_index]->pdr_count ; itr++) {
		pdr_ctx = context->eps_bearers[ebi_index]->pdrs[itr];
		if(pdr_ctx == NULL) {
			clLog(clSystemLog, eCLSeverityCritical,
					"%s %s %d %s - Error no pdr entry \n",__file__,
					__func__, __LINE__, strerror(ret));
		}
		if( del_pdr_entry(context->eps_bearers[ebi_index]->pdrs[itr]->rule_id) != 0 ){
			clLog(clSystemLog, eCLSeverityCritical,
					"%s %s %d %s - Error del_pdr_entry deletion\n",__file__,
					__func__, __LINE__, strerror(ret));
		}
	}
	return 0;
}




void
fill_pfcp_sess_mod_req_pgw_init_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer = NULL;

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->update_far_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
		bearer = NULL;
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case PGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->update_far_count){
				for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count; itr1++) {
					pfcp_sess_mod_req->update_far[itr1].apply_action.drop = PRESENT;
				}
			}
			break;

		default :
			clLog(clSystemLog, eCLSeverityDebug,"default pfcp sess mod req\n");
			break;
	}

	#if 0
	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}
	#endif
}


void
fill_pfcp_sess_mod_req_pgw_init_remove_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	int ret = 0;
	uint32_t seq = 0;
	eps_bearer_t *bearer;
	pdr_t *pdr_ctxt = NULL;
	upf_context_t *upf_ctx = NULL;

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->remove_pdr_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					removing_pdr(&(pfcp_sess_mod_req->remove_pdr[pfcp_sess_mod_req->remove_pdr_count]));
					pfcp_sess_mod_req->remove_pdr[pfcp_sess_mod_req->remove_pdr_count].pdr_id.rule_id = pdr_ctxt->rule_id;
					pfcp_sess_mod_req->remove_pdr_count++;
				}
			}
		}

		bearer = NULL;
	}
}

void
fill_pfcp_sess_mod_req_pgw_del_cmd_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer = NULL;

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->update_far_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
		bearer = NULL;
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case PGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->update_far_count){
				for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count; itr1++) {
					pfcp_sess_mod_req->update_far[itr1].apply_action.drop = PRESENT;
				}
			}
			break;

		default :
			clLog(clSystemLog, eCLSeverityDebug,"default pfcp sess mod req\n");
			break;
	}

	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}

}

int8_t
get_new_bearer_id(pdn_connection_t *pdn_cntxt)
{
	return pdn_cntxt->num_bearer;
}

