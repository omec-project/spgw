/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2019 Sprint
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "pfcp.h"
#include "gx_error_rsp.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "sm_structs_api.h"
#include "sm_struct.h"
#include "gx_error_rsp.h"
#include "clogger.h"
#include "rte_errno.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp.h"
extern udp_sock_t my_sock;

#if 0
void get_error_rsp_info(msg_info *msg, err_rsp_info *rsp_info) 
{

	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	struct resp_info *resp = NULL;

	switch(msg->msg_type) {

		case GTP_CREATE_SESSION_REQ:{

			rsp_info->sender_teid = msg->gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key;
			rsp_info->seq = msg->gtpc_msg.csr.header.teid.has_teid.seq;
			rsp_info->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid =  msg->gtpc_msg.csr.header.teid.has_teid.teid;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.header.len)
				rsp_info->offending = GTP_IE_CREATE_SESS_REQUEST_BEARER_CTXT_TO_BE_CREATED;

			if (!msg->gtpc_msg.csr.sender_fteid_ctl_plane.header.len)
				rsp_info->offending = GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT;

			if (!msg->gtpc_msg.csr.imsi.header.len)
				rsp_info->offending = GTP_IE_IMSI;

			if (!msg->gtpc_msg.csr.apn_ambr.header.len)
				rsp_info->offending = GTP_IE_AGG_MAX_BIT_RATE;

			if (!msg->gtpc_msg.csr.pdn_type.header.len)
					rsp_info->offending = GTP_IE_PDN_TYPE;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.bearer_lvl_qos.header.len)
				rsp_info->offending = GTP_IE_BEARER_QLTY_OF_SVC;

			if (!msg->gtpc_msg.csr.rat_type.header.len)
				rsp_info->offending = GTP_IE_RAT_TYPE;

			if (!msg->gtpc_msg.csr.apn.header.len)
				rsp_info->offending = GTP_IE_ACC_PT_NAME;

			break;
		}

		case PFCP_ASSOCIATION_SETUP_RESPONSE:{

			upf_context_t *upf_context = NULL;

			/*Retrive association state based on UPF IP. */
			ret = rte_hash_lookup_data(upf_context_by_ip_hash,
					(const void*) &(msg->upf_ipv4.s_addr), (void **) &(upf_context));
			if(ret < 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UPF context not found for Msg_Type:%u, UPF IP:%u\n",
										    __file__, __func__, __LINE__,msg->msg_type, msg->upf_ipv4.s_addr);
				return;
			}

	        RTE_SET_USED(index);
            context_key *key = LIST_FIRST(&upf_context->pendingCSRs);
            assert(key == NULL);
            LIST_REMOVE(key, csrentries);
			if (get_ue_context(key->teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->ebi_index = key->ebi_index + 5;
			rsp_info->teid = key->teid;
            // ajay - memleak .. Prio1.. Shopuld we not free the key ?
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {


			if(get_sess_entry(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid, &resp) != 0) {

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						 __file__, __func__, __LINE__, msg->msg_type, msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid, ret);
			}

			if(get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid), &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid);
			if(resp)
				rsp_info->ebi_index = resp->eps_bearer_id + 5;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{


			if (get_ue_context_while_error(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			if(msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				rsp_info->ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid = msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid;
			break;
		}

		case GTP_MODIFY_BEARER_REQ:{

			rsp_info->seq = msg->gtpc_msg.mbr.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mbr.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mbr.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;
			if (get_ue_context(msg->gtpc_msg.mbr.header.teid.has_teid.teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case GTP_MODIFY_BEARER_RSP: {

			rsp_info->seq = msg->gtpc_msg.mb_rsp.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi;

			if (get_ue_context_while_error(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0){
							clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);

			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_MODIFICATION_RESPONSE: {

			if(get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid, &resp) != 0) {

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						 __file__, __func__, __LINE__, msg->msg_type, msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid, ret);
			}


			if (get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid), &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
			if(resp)
				rsp_info->ebi_index = resp->eps_bearer_id;
			break;
		}

		case GTP_DELETE_SESSION_REQ: {

			rsp_info->seq = msg->gtpc_msg.dsr.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.dsr.header.teid.has_teid.teid;

			if(get_ue_context(msg->gtpc_msg.dsr.header.teid.has_teid.teid,
							&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {

			if (get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid), &context)!= 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			rsp_info->ebi_index = UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			break;

		}

		case GTP_DELETE_SESSION_RSP: {

			if(get_ue_context_while_error(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid, &context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid;
			break;
		}
		case GX_CCA_MSG: {
			if(parse_gx_cca_msg(&msg->gx_msg.cca, &pdn) < 0) {
				return;
			}
			if(pdn != NULL && pdn->context != NULL ) {
				context = pdn->context;
				rsp_info->ebi_index = pdn->default_bearer_id;
				rsp_info->sender_teid = context->s11_mme_gtpc_teid;
				rsp_info->seq = context->sequence;
				rsp_info->teid = context->s11_sgw_gtpc_teid;
			}

			break;
		}

		case GTP_UPDATE_BEARER_REQ :{
			if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.ub_req.header.teid.has_teid.teid,
																				&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_req.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_req.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_req.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
			break;
		}

		case GTP_UPDATE_BEARER_RSP:{

			if(get_ue_context(msg->gtpc_msg.ub_rsp.header.teid.has_teid.teid, &context)){

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_rsp.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_rsp.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_rsp.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
			break;
		}
	}
}
#endif

void send_ccr_t_req(msg_info *msg, uint8_t ebi, uint32_t teid) 
{

	int ret = 0;
	pdn_connection_t *pdn =  NULL;
	ue_context_t *context = NULL;

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to get UE State for teid: %u\n",
			__func__, __LINE__, teid);
	}else{
		int ebi_index = ebi - 5;
		if(ebi_index >= 0) {
			if(context != NULL && context->eps_bearers[ebi_index] != NULL
				&& context->eps_bearers[ebi_index]->pdn != NULL ) {
				pdn = context->eps_bearers[ebi_index]->pdn;
			}
			else { return; }
			gx_context_t *gx_context = NULL;
			uint16_t msglen = 0;
			char *buffer = NULL;
			/* Retrive Gx_context based on Sess ID. */
			ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
									(const void*)(pdn->gx_sess_id),
									(void **)&gx_context);
			if (ret < 0) {
				clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
					pdn->gx_sess_id);
			}else{
				gx_msg ccr_request = {0};
				/* VS: Set the Msg header type for CCR-T */
				ccr_request.msg_type = GX_CCR_MSG ;
				/* VS: Set Credit Control Request type */
				ccr_request.data.ccr.presence.cc_request_type = PRESENT;
				ccr_request.data.ccr.cc_request_type = TERMINATION_REQUEST ;
				/* VG: Set Credit Control Bearer opertaion type */
				ccr_request.data.ccr.presence.bearer_operation = PRESENT;
				ccr_request.data.ccr.bearer_operation = TERMINATION ;
				if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, pdn->gx_sess_id) != 0) {
					clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
					return;
				}
				msglen = gx_ccr_calc_length(&ccr_request.data.ccr);
				buffer = rte_zmalloc_socket(NULL, msglen + sizeof(ccr_request.msg_type),
											RTE_CACHE_LINE_SIZE, rte_socket_id());
				if (buffer == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
								"structure: %s (%s:%d)\n",
								 rte_strerror(rte_errno),
								 __FILE__,
								 __LINE__);
					return;
				}

				memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

				if (gx_ccr_pack(&(ccr_request.data.ccr),
					(unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msglen) == 0) {
					clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
					return;
				}

				send_to_ipc_channel(my_sock.gx_app_sock, buffer, msglen + sizeof(ccr_request.msg_type));

				if(rte_hash_del_key(gx_context_by_sess_id_hash, pdn->gx_sess_id) < 0){
					clLog(clSystemLog, eCLSeverityCritical, "%s %s - Error on gx_context_by_sess_id_hash deletion\n"
									,__file__, strerror(ret));
				}
				RTE_SET_USED(msg);
				rte_free(gx_context);
			}
		}else {
			clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND FOR EBI VALUE [%d]\n", __func__,
					ebi);
			return;
		}
	}
}

void gen_reauth_error_response(pdn_connection_t *pdn, int16_t error)
{
/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg raa = {0};
	gx_context_t *gx_context = NULL;
	uint16_t msg_type_ofs = 0;
	uint16_t msg_body_ofs = 0;
	uint16_t rqst_ptr_ofs = 0;
	uint16_t msg_len_total = 0;


	/* Clear Policy in PDN */
	pdn->policy.count = 0;
	pdn->policy.num_charg_rule_install = 0;
	pdn->policy.num_charg_rule_modify = 0;
	pdn->policy.num_charg_rule_delete = 0;

	/* Allocate the memory for Gx Context */
	gx_context = rte_malloc_socket(NULL,
			sizeof(gx_context_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());

	//strncpy(gx_context->gx_sess_id, context->pdns[ebi_index]->gx_sess_id, strlen(context->pdns[ebi_index]->gx_sess_id));


	raa.data.cp_raa.session_id.len = strlen(pdn->gx_sess_id);
	memcpy(raa.data.cp_raa.session_id.val, pdn->gx_sess_id, raa.data.cp_raa.session_id.len);

	raa.data.cp_raa.presence.session_id = PRESENT;

	/* VK: Set the Msg header type for CCR */
	raa.msg_type = GX_RAA_MSG;

	/* Result code */
	raa.data.cp_raa.result_code = error;
	raa.data.cp_raa.presence.result_code = PRESENT;

	/* Update UE State */
	pdn->state = RE_AUTH_ANS_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = RE_AUTH_ANS_SNT_STATE;

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_raa_calc_length(&raa.data.cp_raa);
	msg_body_ofs = sizeof(raa.msg_type);
	rqst_ptr_ofs = msg_len + msg_body_ofs;
	msg_len_total = rqst_ptr_ofs + sizeof(pdn->rqst_ptr);

	buffer = rte_zmalloc_socket(NULL, msg_len_total,
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (buffer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return;
	}

	memcpy(buffer + msg_type_ofs, &raa.msg_type, sizeof(raa.msg_type));

	if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + msg_body_ofs), msg_len) == 0 )
		clLog(clSystemLog, eCLSeverityDebug,"RAA Packing failure\n");

	memcpy((unsigned char *)(buffer + rqst_ptr_ofs), &(pdn->rqst_ptr),
			sizeof(pdn->rqst_ptr));

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len_total);

	return;
}
