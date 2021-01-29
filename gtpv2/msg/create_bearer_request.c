// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_decoder.h"
#include "ue.h"
#include "vepc_cp_dp_api.h"
#include "gtpv2_ie_parsing.h"
#include "tables/tables.h"

extern uint32_t num_adc_rules;
extern uint32_t adc_rule_id[];

/**
 * @brief  : parses gtpv2c message and populates parse_cb_rsp structure
 * @param  : gtpv2c_rx
 *           buffer containing create bearer response message
 * @param  : cbr
 *           data structure to contain required information elements from parsed
 *           create bearer response
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
#if 0
static int
parse_cb_rsp(gtpv2c_header_t *gtpv2c_rx,
	struct parse_cb_rsp_t *cbr)
{
	gtpv2c_ie *current_ie;
	gtpv2c_ie *current_group_ie;
	gtpv2c_ie *limit_ie;
	gtpv2c_ie *limit_group_ie;

	int ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
	    (const void *) &gtpv2c_rx->teid.has_teid.teid,
	    (void **) &cbr->context);

	if (ret < 0 || !cbr->context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	cbr->ded_bearer = cbr->context->ded_bearer;
	if (cbr->ded_bearer == NULL) {
		LOG_MSG(LOG_ERROR, "Received unexpected Create "
				"Bearer Response!\n");
		return -EPERM;
	}
	cbr->context->ded_bearer = NULL;

	/** TODO: we should fully verify mandatory fields within
	 * received message */
	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_ie, limit_ie)
	{
		if (current_ie->type == GTP_IE_BEARER_CONTEXT &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			cbr->bearer_contexts_ie = current_ie;
			FOR_EACH_GROUPED_IE(
					current_ie, current_group_ie,
					limit_group_ie)
			{
				if (current_group_ie->type == GTP_IE_EPS_BEARER_ID &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					cbr->ebi_ie = current_group_ie;
				} else if (current_group_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					cbr->s1u_enb_gtpu_fteid_ie =
							current_group_ie;
				} else if (current_group_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
						current_group_ie->instance ==
							IE_INSTANCE_ONE) {
					cbr->s1u_sgw_gtpu_fteid_ie =
							current_group_ie;
				}
			}
		} else if (current_ie->type == GTP_IE_CAUSE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			cbr->cause_ie = current_ie;
		}
	}

	if (cbr->ebi_ie == NULL || cbr->s1u_enb_gtpu_fteid_ie == NULL
	    || cbr->s1u_sgw_gtpu_fteid_ie == NULL) {
		LOG_MSG(LOG_ERROR, "Received Create Bearer "
				"response without expected IEs");
		return -EPERM;
	}

	cbr->ded_bearer->eps_bearer_id =
		IE_TYPE_PTR_FROM_GTPV2C_IE(eps_bearer_id_ie, cbr->ebi_ie)->ebi;
	cbr->ded_bearer->s1u_enb_gtpu_ipv4 =
		IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
				cbr->s1u_enb_gtpu_fteid_ie)->ip_u.ipv4;
	cbr->ded_bearer->s1u_enb_gtpu_teid =
		IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
			cbr->s1u_enb_gtpu_fteid_ie)->fteid_ie_hdr.teid_or_gre;

	return 0;
}
#endif


#if 0
int
process_create_bearer_response(gtpv2c_header_t *gtpv2c_rx)
{
	struct parse_create_bearer_rsp_t cb_rsp = {0};
	uint8_t i;
	int ret = parse_cb_rsp(gtpv2c_rx, &cb_rsp);

	if (ret)
		return ret;

	uint8_t ebi_index = cb_rsp.ded_bearer->eps_bearer_id - 5;

	if (cb_rsp.context->eps_bearers[ebi_index]) {
		/* TODO: Investigate correct behavior when new bearers are
		 * created with an ID of existing bearer
		 */
		rte_free(cb_rsp.context->eps_bearers[ebi_index]);
	}

	cb_rsp.context->eps_bearers[ebi_index] =
	    cb_rsp.ded_bearer;
	cb_rsp.context->bearer_bitmap |= (1 << ebi_index);

	cb_rsp.ded_bearer->pdn->eps_bearers[ebi_index] =
	    cb_rsp.ded_bearer;
	cb_rsp.context->eps_bearers[ebi_index] =
	    cb_rsp.ded_bearer;

	struct dp_id dp_id = { .id = DPN_ID };
	/* using the s1u_sgw_gtpu_teid as unique identifier to the session */
	struct session_info session;
	memset(&session, 0, sizeof(session));

	session.ue_addr.iptype = IPTYPE_IPV4;
	session.ue_addr.u.ipv4_addr =
		htonl(cb_rsp.ded_bearer->pdn->ipv4.s_addr);
	session.ul_s1_info.sgw_teid =
		htonl(cb_rsp.ded_bearer->s1u_sgw_gtpu_teid),
	session.ul_s1_info.sgw_addr.iptype = IPTYPE_IPV4;
	session.ul_s1_info.sgw_addr.u.ipv4_addr =
		htonl(cb_rsp.ded_bearer->s1u_sgw_gtpu_ipv4.s_addr);
	session.ul_s1_info.enb_addr.iptype = IPTYPE_IPV4;
	session.ul_s1_info.enb_addr.u.ipv4_addr =
		htonl(cb_rsp.ded_bearer->s1u_enb_gtpu_ipv4.s_addr);
	session.dl_s1_info.enb_teid =
		htonl(cb_rsp.ded_bearer->s1u_enb_gtpu_teid);
	session.dl_s1_info.enb_addr.iptype = IPTYPE_IPV4;
	session.dl_s1_info.enb_addr.u.ipv4_addr =
		htonl(cb_rsp.ded_bearer->s1u_enb_gtpu_ipv4.s_addr);
	session.dl_s1_info.sgw_addr.iptype = IPTYPE_IPV4,
	session.dl_s1_info.sgw_addr.u.ipv4_addr =
		htonl(cb_rsp.ded_bearer->s1u_sgw_gtpu_ipv4.s_addr),
	session.ul_apn_mtr_idx = ulambr_idx;
	session.dl_apn_mtr_idx = dlambr_idx;

	for (i = 0; i < cb_rsp.ded_bearer->num_packet_filters; ++i) {
		uint8_t packet_filter_direction = get_packet_filter_direction(
		    cb_rsp.ded_bearer->packet_filter_map[i]);
		if (packet_filter_direction & TFT_DIRECTION_DOWNLINK_ONLY) {
			session.dl_pcc_rule_id[session.num_dl_pcc_rules++] =
			    cb_rsp.ded_bearer->packet_filter_map[i];
		}
		if (packet_filter_direction & TFT_DIRECTION_UPLINK_ONLY) {
			session.ul_pcc_rule_id[session.num_ul_pcc_rules++] =
					FIRST_FILTER_ID;
		}
	}
	session.num_adc_rules = num_adc_rules;
	for (i = 0; i < num_adc_rules; ++i)
		session.adc_rule_id[i] = adc_rule_id[i];
	/* using ue ipv4 addr as unique identifier for an UE.
	 * and sess_id is combination of ue addr and bearer id.
	 * formula to set sess_id = (ue_ipv4_addr << 4) | bearer_id
	 */
	session.sess_id = SESS_ID(cb_rsp.context->s11_sgw_gtpc_teid,
				cb_rsp.ded_bearer->eps_bearer_id);

	if (session_create(dp_id, session) < 0)
        assert(0);

	if (session_modify(dp_id, session) < 0)
        assert(0);
	return 0;
}
#endif

#ifdef FUTURE_NEED
// sgw - DED_BER_ACTIVATION_PROC CONNECTED_STATE CREATE_BER_REQ_RCVD_EVNT : process_create_bearer_request_handler 
// sgw : DED_BER_ACTIVATION_PROC IDEL_STATE CREATE_BER_REQ_RCVD_EVNT - process_create_bearer_request_handler 
int handle_create_bearer_request_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);

    if((ret = decode_create_bearer_req((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.cb_req) == 0))
        return -1;


	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
	uint8_t ebi_index = msg->gtpc_msg.cb_req.lbi.ebi_ebi - 5;

	if(get_ue_context_by_sgw_s5s8_teid(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		LOG_MSG(LOG_ERROR, "UE Context not found... 0x%x",
					gtpv2c_rx->teid.has_teid.teid);
		return -1;
	}
	msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->proc =  DED_BER_ACTIVATION_PROC;
	msg->event = CREATE_BER_REQ_RCVD_EVNT;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s",
			gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

#endif
