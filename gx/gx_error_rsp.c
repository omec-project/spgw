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
#include "tables/tables.h"
extern udp_sock_t my_sock;

void send_ccr_t_req(msg_info_t *msg, uint8_t ebi, uint32_t teid) 
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
			ret = get_gx_context((uint8_t *)pdn->gx_sess_id,&gx_context);
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

				if(remove_gx_context((uint8_t*)pdn->gx_sess_id) < 0){
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
