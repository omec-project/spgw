// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <rte_debug.h>
#include "rte_hash.h"
#include "gtp_messages_decoder.h"
#include "gtpv2_set_ie.h"
#include "ue.h"
#include "vepc_cp_dp_api.h"
#include "clogger.h"
#include "gtpv2_ie_parsing.h"
#include "gtpv2_interface.h"
#include "cp_config_defs.h"

/**
 * @brief  : Maintatins data from parsed delete bearer response
 */
struct parse_delete_bearer_rsp_t {
	ue_context_t *context;
	pdn_connection_t *pdn;
	eps_bearer_t *ded_bearer;

	gtpv2c_ie *cause_ie;
	gtpv2c_ie *bearer_context_ebi_ie;
	gtpv2c_ie *bearer_context_cause_ie;
};

/**
 * @brief  : parses gtpv2c message and populates parse_delete_bearer_rsp_t structure
 * @param  : gtpv2c_rx
 *           buffer containing delete bearer response message
 * @param  : dbr
 *           data structure to contain required information elements from parsed
 *           delete bearer response
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
static int
parse_delete_bearer_response(gtpv2c_header_t *gtpv2c_rx,
		struct parse_delete_bearer_rsp_t *dbr)
{
	gtpv2c_ie *current_ie;
	gtpv2c_ie *current_group_ie;
	gtpv2c_ie *limit_ie;
	gtpv2c_ie *limit_group_ie;

	int ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
	    (const void *) &gtpv2c_rx->teid.has_teid.teid,
	    (void **) &dbr->context);

	if (ret < 0 || !dbr->context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	/** TODO: we should fully verify mandatory fields within received
	 *  message */
	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_ie, limit_ie)
	{
		if (current_ie->type == GTP_IE_CAUSE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			dbr->cause_ie = current_ie;
		} else if (current_ie->type == GTP_IE_BEARER_CONTEXT &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			FOR_EACH_GROUPED_IE(current_ie, current_group_ie,
					limit_group_ie)
			{
				if (current_group_ie->type == GTP_IE_EPS_BEARER_ID &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					dbr->bearer_context_ebi_ie =
							current_group_ie;
				} else if (current_group_ie->type == GTP_IE_CAUSE &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					dbr->bearer_context_cause_ie =
							current_group_ie;
				}
			}
		}
	}


	if (!dbr->cause_ie || !dbr->bearer_context_ebi_ie
	    || !dbr->bearer_context_cause_ie) {
		clLog(clSystemLog, eCLSeverityCritical, "Received Delete Bearer Response without "
				"mandatory IEs\n");
		return -EPERM;
	}


	if (IE_TYPE_PTR_FROM_GTPV2C_IE(cause_ie,
			dbr->cause_ie)->cause_ie_hdr.cause_value
	    != GTPV2C_CAUSE_REQUEST_ACCEPTED)
		return IE_TYPE_PTR_FROM_GTPV2C_IE(cause_ie,
				dbr->cause_ie)->cause_ie_hdr.cause_value;


	return 0;
}


int
process_delete_bearer_response(gtpv2c_header_t *gtpv2c_rx)
{
	struct parse_delete_bearer_rsp_t delete_bearer_rsp = { 0 };
	int ret = parse_delete_bearer_response(gtpv2c_rx, &delete_bearer_rsp);
	if (ret)
		return ret;

	uint8_t ebi =
	    IE_TYPE_PTR_FROM_GTPV2C_IE(eps_bearer_id_ie,
			    delete_bearer_rsp.bearer_context_ebi_ie)->ebi;
	uint8_t ebi_index = ebi - 5;

	delete_bearer_rsp.ded_bearer =
	    delete_bearer_rsp.context->eps_bearers[ebi_index];

	if (delete_bearer_rsp.ded_bearer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
		    "Received Delete Bearer Response for"
		    " non-existant EBI: %"PRIu8"\n",
		    ebi);
		return -EPERM;
	}
	delete_bearer_rsp.pdn = delete_bearer_rsp.ded_bearer->pdn;

	if (delete_bearer_rsp.context->eps_bearers[ebi_index]
	    != delete_bearer_rsp.pdn->eps_bearers[ebi_index])
		rte_panic("Incorrect provisioning of bearers\n");


	if (delete_bearer_rsp.ded_bearer->eps_bearer_id
	    ==
	    IE_TYPE_PTR_FROM_GTPV2C_IE(eps_bearer_id_ie,
			    delete_bearer_rsp.bearer_context_ebi_ie)->ebi) {
		delete_bearer_rsp.context->bearer_bitmap &= ~(1
		    << (delete_bearer_rsp.ded_bearer->eps_bearer_id - 5));
		delete_bearer_rsp.context->eps_bearers[ebi_index] = NULL;
		delete_bearer_rsp.pdn->eps_bearers[ebi_index] = NULL;
		uint8_t index = ((0x0f000000
		    & delete_bearer_rsp.ded_bearer->s1u_sgw_gtpu_teid) >> 24);
		delete_bearer_rsp.context->teid_bitmap &= ~(0x01 << index);

		struct dp_id dp_id = { .id = DPN_ID };

        RTE_SET_USED(dp_id);

		struct session_info si;
		memset(&si, 0, sizeof(si));

		si.ue_addr.u.ipv4_addr =
		     htonl(delete_bearer_rsp.pdn->ipv4.s_addr);
		si.sess_id =
			SESS_ID(delete_bearer_rsp.context->s11_sgw_gtpc_teid,
				delete_bearer_rsp.ded_bearer->eps_bearer_id);
#ifdef OBSOLETE_API
		session_delete(dp_id, si);
#endif

		rte_free(delete_bearer_rsp.ded_bearer);
	}

	return 0;
}


void
set_delete_bearer_request(gtpv2c_header_t *gtpv2c_tx, uint32_t sequence,
	ue_context_t *context, uint8_t linked_eps_bearer_id,
	uint8_t ded_eps_bearer_ids[], uint8_t ded_bearer_counter)
{
	del_bearer_req_t db_req = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *) &db_req, GTP_DELETE_BEARER_REQ,
	    context->s11_mme_gtpc_teid, sequence);

	if (linked_eps_bearer_id > 0) {
		set_ebi(&db_req.lbi, IE_INSTANCE_ZERO, linked_eps_bearer_id);
	} else {
		for (uint8_t iCnt = 0; iCnt < ded_bearer_counter; ++iCnt) {
			set_ebi(&db_req.eps_bearer_ids[iCnt], IE_INSTANCE_ONE,
				ded_eps_bearer_ids[iCnt]);
		}

		db_req.bearer_count = ded_bearer_counter;
	}

	uint16_t msg_len = 0;
	msg_len = encode_del_bearer_req(&db_req, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}

void
set_delete_bearer_response(gtpv2c_header_t *gtpv2c_tx, uint32_t sequence,
	uint8_t linked_eps_bearer_id, uint8_t ded_eps_bearer_ids[],
	uint8_t ded_bearer_counter, uint32_t s5s8_pgw_gtpc_teid)
{
	del_bearer_rsp_t db_resp = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *) &db_resp, GTP_DELETE_BEARER_RSP,
		s5s8_pgw_gtpc_teid , sequence);

	set_cause_accepted(&db_resp.cause, IE_INSTANCE_ZERO);

	//db_resp..header.gtpc.message_len += db_resp.cause.header.len + sizeof(ie_header_t);
	if (linked_eps_bearer_id > 0) {
		set_ebi(&db_resp.lbi, IE_INSTANCE_ZERO, linked_eps_bearer_id);
	} else {
		for (uint8_t iCnt = 0; iCnt < ded_bearer_counter; ++iCnt) {
			set_ie_header(&db_resp.bearer_contexts[iCnt].header,
				GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO, 0);

			set_ebi(&db_resp.bearer_contexts[iCnt].eps_bearer_id,
				IE_INSTANCE_ZERO, ded_eps_bearer_ids[iCnt]);
			db_resp.bearer_contexts[iCnt].header.len +=
				sizeof(uint8_t) + IE_HEADER_SIZE;

			set_cause_accepted(&db_resp.bearer_contexts[iCnt].cause,
				IE_INSTANCE_ZERO);
			db_resp.bearer_contexts[iCnt].header.len +=
				sizeof(uint16_t) + IE_HEADER_SIZE;
	//		db_resp..header.gtpc.message_len += db_resp.bearer_contexts[iCnt].header.len +
	//					sizeof(ie_header_t);

		}

		db_resp.bearer_count = ded_bearer_counter;
	}

	uint16_t msg_len = 0;
	msg_len = encode_del_bearer_rsp(&db_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}

#ifdef FUTURE_NEED

// sgw :  PDN_GW_INIT_BEARER_DEACTIVATION CONNECTED_STATE DELETE_BER_REQ_RCVD_EVNT - process_delete_bearer_request_handler 
// sgw : PDN_GW_INIT_BEARER_DEACTIVATION IDEL_STATE DELETE_BER_REQ_RCVD_EVNT - process_delete_bearer_request_handler
// sgw : MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_REQ_RCVD_EVNT : process_delete_bearer_req_handler 
int handle_delete_bearer_request_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    uint8_t ebi_index;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);

    if((ret = decode_del_bearer_req((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.db_req) == 0))
        return -1;



	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	if (msg->gtpc_msg.db_req.lbi.header.len) {
		ebi_index = msg->gtpc_msg.db_req.lbi.ebi_ebi - 5;
	} else {
		ebi_index = msg->gtpc_msg.db_req.eps_bearer_ids[0].ebi_ebi - 5;
	}

	if(get_ue_context_by_sgw_s5s8_teid(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		clLog(sxlogger, eCLSeverityCritical,
			"%s:%d UE Context not found... 0x%x\n",__func__,
			__LINE__, gtpv2c_rx->teid.has_teid.teid);
		return -1;
	}

	if(context->eps_bearers[ebi_index]->pdn->proc == MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC){
		msg->proc = context->eps_bearers[ebi_index]->pdn->proc;
	}else{
		msg->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		context->eps_bearers[ebi_index]->pdn->proc = msg->proc;
	}
	msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->event = DELETE_BER_REQ_RCVD_EVNT;

	context->eps_bearers[ebi_index]->pdn->proc = msg->proc;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));


    return 0;
}

int
process_delete_bearer_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_delete_bearer_request(&msg->gtpc_msg.db_req ,0);
	if (ret) {
		clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
			__func__, __LINE__, ret);
		return -1;
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);

	return 0;
}

/*PGWC send Delete Bearer Request to SGWC*/
int process_delete_bearer_req_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = process_delete_bearer_request(&msg->gtpc_msg.db_req, 1);
	if(ret !=0 ) {
		/*TODO: set error response*/
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
	}

	RTE_SET_USED(unused_param);
	return 0;
}

#endif
