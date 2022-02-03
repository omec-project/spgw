// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "pfcp.h"
#include "gx_error_rsp.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "sm_structs_api.h"
#include "sm_struct.h"
#include "gx_error_rsp.h"
#include "cp_log.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp.h"
#include "cp_io_poll.h"
#include "gx_interface.h"
#include "spgw_cpp_wrapper.h"

void send_ccr_t_req(msg_info_t *msg, uint8_t ebi, uint32_t teid) 
{

	pdn_connection_t *pdn =  NULL;
	ue_context_t *context = NULL;

	/* Retrieve the UE context */
	context = (ue_context_t *)get_ue_context(teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to get UE State for teid: %u", teid);
	}else{
		int ebi_index = ebi - 5;
		if(ebi_index >= 0) {
			if(context != NULL && context->eps_bearers[ebi_index] != NULL
				&& context->eps_bearers[ebi_index]->pdn != NULL ) {
				pdn = context->eps_bearers[ebi_index]->pdn;
			}
			else { 
                return; 
            }
			uint16_t msglen = 0;
			char *buffer = NULL;
			/* Retrive Gx_context based on Sess ID. */
			ue_context_t *ue_context  = (ue_context_t *)get_ue_context_from_gxsessid((uint8_t *)pdn->gx_sess_id);
			if (ue_context == NULL) {
				LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
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
					LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
					return;
				}
				msglen = gx_ccr_calc_length(&ccr_request.data.ccr);
				buffer = (char *)calloc(1, msglen + sizeof(ccr_request.msg_type) +  sizeof(ccr_request.seq_num));
				if (buffer == NULL) {
				    LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
					return;
				}

				memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

				if (gx_ccr_pack(&(ccr_request.data.ccr),
					(unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msglen) == 0) {
					LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
					return;
				}

				gx_send(my_sock.gx_app_sock, buffer, msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));

				if(remove_gxsessid_to_context((uint8_t*)pdn->gx_sess_id) < 0){
					LOG_MSG(LOG_ERROR, " Error on gx_context_by_sess_id_hash deletion");
				}
			}
		}else {
			LOG_MSG(LOG_ERROR, "NO ENTRY FOUND FOR EBI VALUE [%d], msg = %p ", ebi, msg);
			return;
		}
	}
}

void gen_reauth_error_response(pdn_connection_t *pdn, int16_t error, uint16_t seq_num)
{
/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg raa = {0};
	uint16_t msg_type_ofs = 0;
	uint16_t msg_body_ofs = 0;
	uint16_t rqst_ptr_ofs = 0;
	uint16_t msg_len_total = 0;


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

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_raa_calc_length(&raa.data.cp_raa);
	msg_body_ofs = sizeof(raa.msg_type) + sizeof(raa.seq_num);
	rqst_ptr_ofs = msg_len + msg_body_ofs;
	msg_len_total = rqst_ptr_ofs + sizeof(seq_num);

	buffer = (char *)calloc(1, msg_len_total);
	if (buffer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
		return;
	}

	memcpy(buffer + msg_type_ofs, &raa.msg_type, sizeof(raa.msg_type));

	if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + msg_body_ofs), msg_len) == 0 )
		LOG_MSG(LOG_DEBUG,"RAA Packing failure");

	memcpy((unsigned char *)(buffer + rqst_ptr_ofs), &seq_num,
			sizeof(seq_num));

	/* VS: Write or Send CCR msg to Gx_App */
	gx_send(my_sock.gx_app_sock, buffer, msg_len_total);

	return;
}
