// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_decoder.h"
#include "ue.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "vepc_cp_dp_api.h"
#include "pfcp_cp_set_ie.h"
#include "cp_log.h"
#include "gtpv2_internal.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "cp_config_defs.h"
#include "cp_peer.h"
#include "spgw_config_struct.h"
#include "spgw_cpp_wrapper.h"
#include "proc_service_request.h"
#include "sm_structs_api.h"
#include "gtpv2_error_rsp.h"
#include "util.h"
#include "proc.h"

/**
 * @brief  : Maintains parsed data from modify bearer request
 */
struct parse_modify_bearer_request_t {
	ue_context_t *context;
	pdn_connection_t *pdn;
	eps_bearer_t *bearer;

	gtpv2c_ie *bearer_context_to_be_created_ebi;
	gtpv2c_ie *s1u_enb_fteid;
	uint8_t *delay;
	uint32_t *s11_mme_gtpc_fteid;
};
extern uint32_t num_adc_rules;
extern uint32_t adc_rule_id[];

/**
 * @brief  : from parameters, populates gtpv2c message 'modify bearer response' and
 *           populates required information elements as defined by
 *           clause 7.2.8 3gpp 29.274
 * @param  : gtpv2c_tx
 *           transmission buffer to contain 'modify bearer request' message
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : context
 *           UE Context data structure pertaining to the bearer to be modified
 * @param  : bearer
 *           bearer data structure to be modified
 * @return : Returns nothing
 */
void
set_modify_bearer_response(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, ue_context_t *context, eps_bearer_t *bearer)
{
	uint64_t _ebi = bearer->eps_bearer_id;
	uint64_t ebi_index = _ebi - 5;
	upf_context_t *upf_ctx = NULL;

	/*Retrive bearer id from bearer --> context->pdns[]->upf_ip*/
	upf_ctx = (upf_context_t *)upf_context_entry_lookup(context->pdns[ebi_index]->upf_ipv4.s_addr);
    if(upf_ctx == NULL) {
		return;
	}

	mod_bearer_rsp_t mb_resp = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *) &mb_resp, GTP_MODIFY_BEARER_RSP,
	    context->s11_mme_gtpc_teid, sequence);

	set_cause_accepted(&mb_resp.cause, IE_INSTANCE_ZERO);

	set_ie_header(&mb_resp.bearer_contexts_modified.header, GTP_IE_BEARER_CONTEXT,
			IE_INSTANCE_ZERO, 0);

	set_cause_accepted(&mb_resp.bearer_contexts_modified.cause, IE_INSTANCE_ZERO);
	mb_resp.bearer_contexts_modified.header.len +=
		sizeof(struct cause_ie_hdr_t) + IE_HEADER_SIZE;

	set_ebi(&mb_resp.bearer_contexts_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			bearer->eps_bearer_id);
	mb_resp.bearer_contexts_modified.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

	struct in_addr ip;
	ip.s_addr = upf_ctx->s1u_ip;

	set_ipv4_fteid(&mb_resp.bearer_contexts_modified.s1u_sgw_fteid,
			GTPV2C_IFTYPE_S1U_SGW_GTPU, IE_INSTANCE_ZERO, ip,
			bearer->s1u_sgw_gtpu_teid);

	mb_resp.bearer_contexts_modified.header.len += sizeof(struct fteid_ie_hdr_t) +
		sizeof(struct in_addr) + IE_HEADER_SIZE;

	uint16_t msg_len = 0;
	msg_len = encode_mod_bearer_rsp(&mb_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}
/*MODIFY RESPONSE FUNCTION WHEN PGWC returns MBR RESPONSE to SGWC
 * in HANDOVER SCENARIO*/

void
set_modify_bearer_response_handover(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, ue_context_t *context, eps_bearer_t *bearer)
{
	uint64_t _ebi = bearer->eps_bearer_id;
	uint64_t ebi_index = _ebi - 5;
	upf_context_t *upf_ctx = NULL;

	/*Retrive bearer id from bearer --> context->pdns[]->upf_ip*/
    
	upf_ctx = (upf_context_t *)upf_context_entry_lookup(context->pdns[ebi_index]->upf_ipv4.s_addr);
	if(upf_ctx == NULL) {
		return;
	}

	mod_bearer_rsp_t mb_resp = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *) &mb_resp, GTP_MODIFY_BEARER_RSP,
		bearer->pdn->s5s8_sgw_gtpc_teid, sequence);

	if(context->msisdn !=0) {
		set_ie_header(&mb_resp.msisdn.header, GTP_IE_MSISDN, IE_INSTANCE_ZERO, BINARY_MSISDN_LEN);
		mb_resp.msisdn.msisdn_number_digits = context->msisdn;
	}

	set_cause_accepted(&mb_resp.cause, IE_INSTANCE_ZERO);

	set_ie_header(&mb_resp.bearer_contexts_modified.header, GTP_IE_BEARER_CONTEXT,
			IE_INSTANCE_ZERO, 0);

	set_cause_accepted(&mb_resp.bearer_contexts_modified.cause, IE_INSTANCE_ZERO);
	mb_resp.bearer_contexts_modified.header.len +=
		sizeof(struct cause_ie_hdr_t) + IE_HEADER_SIZE;

	set_ebi(&mb_resp.bearer_contexts_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			bearer->eps_bearer_id);
	mb_resp.bearer_contexts_modified.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

	uint16_t msg_len = 0;
	msg_len = encode_mod_bearer_rsp(&mb_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}

int
process_modify_bearer_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx)
{
	mod_bearer_req_t mb_req = {0};
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	//pdn_connection_t *pdn = NULL;

	decode_mod_bearer_req((uint8_t *) gtpv2c_rx, &mb_req);

	context = (ue_context_t *)get_ue_context(mb_req.header.teid.has_teid.teid);

	if (context == NULL) {
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

	if (!mb_req.bearer_contexts_to_be_modified.eps_bearer_id.header.len
			|| !mb_req.bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len) {
			LOG_MSG(LOG_ERROR, "Dropping packet");
			return -EPERM;
	}

	uint8_t ebi_index = mb_req.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR,
			"Received modify bearer on non-existent EBI - "
			"Dropping packet");
		return -EPERM;
	}

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		LOG_MSG(LOG_ERROR,
			"Received modify bearer on non-existent EBI - "
			"Bitmap Inconsistency - Dropping packet");
		return -EPERM;
	}

	//pdn = bearer->pdn;

	/* TODO something with modify_bearer_request.delay if set */

	if (mb_req.sender_fteid_ctl_plane.header.len &&
			(context->s11_mme_gtpc_teid != mb_req.bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key))
		context->s11_mme_gtpc_teid = mb_req.bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key;

	bearer->s1u_enb_gtpu_ipv4.s_addr =
			mb_req.bearer_contexts_to_be_modified.s1_enodeb_fteid.ipv4_address;

	bearer->s1u_enb_gtpu_teid =
			mb_req.bearer_contexts_to_be_modified.s1_enodeb_fteid.teid_gre_key;

	bearer->eps_bearer_id = mb_req.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;

	set_modify_bearer_response(gtpv2c_tx, mb_req.header.teid.has_teid.seq,
	    context, bearer);

#ifdef OBSELETE_APIS
	/* using the s1u_sgw_gtpu_teid as unique identifier to the session */
	struct session_info session;
	memset(&session, 0, sizeof(session));
	 session.ue_addr.iptype = IPTYPE_IPV4;
	 session.ue_addr.u.ipv4_addr =
		 pdn->ipv4.s_addr;
	 session.ul_s1_info.sgw_teid =
		htonl(bearer->s1u_sgw_gtpu_teid);
	 session.ul_s1_info.sgw_addr.iptype = IPTYPE_IPV4;
	 session.ul_s1_info.sgw_addr.u.ipv4_addr =
		 htonl(bearer->s1u_sgw_gtpu_ipv4.s_addr);
	 session.ul_s1_info.enb_addr.iptype = IPTYPE_IPV4;
	 session.ul_s1_info.enb_addr.u.ipv4_addr =
		 bearer->s1u_enb_gtpu_ipv4.s_addr;
	 session.dl_s1_info.enb_teid =
		 bearer->s1u_enb_gtpu_teid;
	 session.dl_s1_info.enb_addr.iptype = IPTYPE_IPV4;
	 session.dl_s1_info.enb_addr.u.ipv4_addr =
		 bearer->s1u_enb_gtpu_ipv4.s_addr;
	 session.dl_s1_info.sgw_addr.iptype = IPTYPE_IPV4;
	 session.dl_s1_info.sgw_addr.u.ipv4_addr =
		 htonl(bearer->s1u_sgw_gtpu_ipv4.s_addr);
	 session.ul_apn_mtr_idx = 0;
	 session.dl_apn_mtr_idx = 0;
	 session.num_ul_pcc_rules = 1;
	 session.ul_pcc_rule_id[0] = FIRST_FILTER_ID;
	 session.num_dl_pcc_rules = 1;
	 session.dl_pcc_rule_id[0] = FIRST_FILTER_ID;

	 session.num_adc_rules = num_adc_rules;
	 uint32_t i;
	 for (i = 0; i < num_adc_rules; ++i)
			 session.adc_rule_id[i] = adc_rule_id[i];

	 session.sess_id = SESS_ID(
			context->s11_sgw_gtpc_teid,
			bearer->eps_bearer_id);

	struct dp_id dp_id = { .id = DPN_ID };
	if (session_modify(dp_id, session) < 0)
        assert(0);
#endif
	return 0;
}


void set_modify_bearer_request(gtpv2c_header_t *gtpv2c_tx, /*create_sess_req_t *csr,*/
		pdn_connection_t *pdn, eps_bearer_t *bearer)
{
	int len = 0 ;
	mod_bearer_req_t mbr = {0};
	ue_context_t *context = NULL;

    uint32_t sequence = 0; // TODO BUG - add transaction support
	set_gtpv2c_teid_header((gtpv2c_header_t *)&mbr.header, GTP_MODIFY_BEARER_REQ,
			0, sequence);

	mbr.header.teid.has_teid.teid = pdn->s5s8_pgw_gtpc_teid;
	bearer->s5s8_sgw_gtpu_ipv4.s_addr = htonl(bearer->s5s8_sgw_gtpu_ipv4.s_addr);
	set_ipv4_fteid(&mbr.sender_fteid_ctl_plane, GTPV2C_IFTYPE_S5S8_SGW_GTPC,
			IE_INSTANCE_ZERO,
			pdn->s5s8_sgw_gtpc_ipv4, pdn->s5s8_sgw_gtpc_teid);
	if(pdn == NULL &&  pdn->context == NULL ) {
		LOG_MSG(LOG_ERROR, "UE contex not found : Warnning ");

		return;
	}
	context = pdn->context;
	if (context->uli.lai) {
		mbr.uli.lai = context->uli.lai;
		mbr.uli.lai2.lai_mcc_digit_2 = context->uli.lai2.lai_mcc_digit_2;
		mbr.uli.lai2.lai_mcc_digit_1 = context->uli.lai2.lai_mcc_digit_1;
		mbr.uli.lai2.lai_mnc_digit_3 = context->uli.lai2.lai_mnc_digit_3;
		mbr.uli.lai2.lai_mcc_digit_3 = context->uli.lai2.lai_mcc_digit_3;
		mbr.uli.lai2.lai_mnc_digit_2 = context->uli.lai2.lai_mnc_digit_2;
		mbr.uli.lai2.lai_mnc_digit_1 = context->uli.lai2.lai_mnc_digit_1;
		mbr.uli.lai2.lai_lac = context->uli.lai2.lai_lac;

		len += sizeof(mbr.uli.lai2);
	}
	if (context->uli.tai) {
		mbr.uli.tai = context->uli.tai;
		mbr.uli.tai2.tai_mcc_digit_2 = context->uli.tai2.tai_mcc_digit_2;
		mbr.uli.tai2.tai_mcc_digit_1 = context->uli.tai2.tai_mcc_digit_1;
		mbr.uli.tai2.tai_mnc_digit_3 = context->uli.tai2.tai_mnc_digit_3;
		mbr.uli.tai2.tai_mcc_digit_3 = context->uli.tai2.tai_mcc_digit_3;
		mbr.uli.tai2.tai_mnc_digit_2 = context->uli.tai2.tai_mnc_digit_2;
		mbr.uli.tai2.tai_mnc_digit_1 = context->uli.tai2.tai_mnc_digit_1;
		mbr.uli.tai2.tai_tac = context->uli.tai2.tai_tac;
		len += sizeof(mbr.uli.tai2);
	}
	if (context->uli.rai) {
		mbr.uli.rai = context->uli.rai;
		mbr.uli.rai2.ria_mcc_digit_2 = context->uli.rai2.ria_mcc_digit_2;
		mbr.uli.rai2.ria_mcc_digit_1 = context->uli.rai2.ria_mcc_digit_1;
		mbr.uli.rai2.ria_mnc_digit_3 = context->uli.rai2.ria_mnc_digit_3;
		mbr.uli.rai2.ria_mcc_digit_3 = context->uli.rai2.ria_mcc_digit_3;
		mbr.uli.rai2.ria_mnc_digit_2 = context->uli.rai2.ria_mnc_digit_2;
		mbr.uli.rai2.ria_mnc_digit_1 = context->uli.rai2.ria_mnc_digit_1;
		mbr.uli.rai2.ria_lac = context->uli.rai2.ria_lac;
		mbr.uli.rai2.ria_rac = context->uli.rai2.ria_rac;
		len += sizeof(mbr.uli.rai2);
	}
	if (context->uli.sai) {
		mbr.uli.sai = context->uli.sai;
		mbr.uli.sai2.sai_mcc_digit_2 = context->uli.sai2.sai_mcc_digit_2;
		mbr.uli.sai2.sai_mcc_digit_1 = context->uli.sai2.sai_mcc_digit_1;
		mbr.uli.sai2.sai_mnc_digit_3 = context->uli.sai2.sai_mnc_digit_3;
		mbr.uli.sai2.sai_mcc_digit_3 = context->uli.sai2.sai_mcc_digit_3;
		mbr.uli.sai2.sai_mnc_digit_2 = context->uli.sai2.sai_mnc_digit_2;
		mbr.uli.sai2.sai_mnc_digit_1 = context->uli.sai2.sai_mnc_digit_1;
		mbr.uli.sai2.sai_lac = context->uli.sai2.sai_lac;
		mbr.uli.sai2.sai_sac = context->uli.sai2.sai_sac;
		len += sizeof(mbr.uli.sai2);
	}
	if (context->uli.cgi) {
		mbr.uli.cgi = context->uli.cgi;
		mbr.uli.cgi2.cgi_mcc_digit_2 = context->uli.cgi2.cgi_mcc_digit_2;
		mbr.uli.cgi2.cgi_mcc_digit_1 = context->uli.cgi2.cgi_mcc_digit_1;
		mbr.uli.cgi2.cgi_mnc_digit_3 = context->uli.cgi2.cgi_mnc_digit_3;
		mbr.uli.cgi2.cgi_mcc_digit_3 = context->uli.cgi2.cgi_mcc_digit_3;
		mbr.uli.cgi2.cgi_mnc_digit_2 = context->uli.cgi2.cgi_mnc_digit_2;
		mbr.uli.cgi2.cgi_mnc_digit_1 = context->uli.cgi2.cgi_mnc_digit_1;
		mbr.uli.cgi2.cgi_lac = context->uli.cgi2.cgi_lac;
		mbr.uli.cgi2.cgi_ci = context->uli.cgi2.cgi_ci;
		len += sizeof(mbr.uli.cgi2);
	}
	if (context->uli.ecgi) {
		mbr.uli.ecgi = context->uli.ecgi;
		mbr.uli.ecgi2.ecgi_mcc_digit_2 = context->uli.ecgi2.ecgi_mcc_digit_2;
		mbr.uli.ecgi2.ecgi_mcc_digit_1 = context->uli.ecgi2.ecgi_mcc_digit_1;
		mbr.uli.ecgi2.ecgi_mnc_digit_3 = context->uli.ecgi2.ecgi_mnc_digit_3;
		mbr.uli.ecgi2.ecgi_mcc_digit_3 = context->uli.ecgi2.ecgi_mcc_digit_3;
		mbr.uli.ecgi2.ecgi_mnc_digit_2 = context->uli.ecgi2.ecgi_mnc_digit_2;
		mbr.uli.ecgi2.ecgi_mnc_digit_1 = context->uli.ecgi2.ecgi_mnc_digit_1;
		mbr.uli.ecgi2.ecgi_spare = context->uli.ecgi2.ecgi_spare;
		mbr.uli.ecgi2.eci = context->uli.ecgi2.eci;
		len += sizeof(mbr.uli.ecgi2);
	}
	if (context->uli.macro_enodeb_id) {
		mbr.uli.macro_enodeb_id = context->uli.macro_enodeb_id;
		mbr.uli.macro_enodeb_id2.menbid_mcc_digit_2 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_2;
		mbr.uli.macro_enodeb_id2.menbid_mcc_digit_1 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_1;
		mbr.uli.macro_enodeb_id2.menbid_mnc_digit_3 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_3;
		mbr.uli.macro_enodeb_id2.menbid_mcc_digit_3 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_3;
		mbr.uli.macro_enodeb_id2.menbid_mnc_digit_2 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_2;
		mbr.uli.macro_enodeb_id2.menbid_mnc_digit_1 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_1;
		mbr.uli.macro_enodeb_id2.menbid_spare =
			context->uli.macro_enodeb_id2.menbid_spare;
		mbr.uli.macro_enodeb_id2.menbid_macro_enodeb_id =
			context->uli.macro_enodeb_id2.menbid_macro_enodeb_id;
		mbr.uli.macro_enodeb_id2.menbid_macro_enb_id2 =
			context->uli.macro_enodeb_id2.menbid_macro_enb_id2;
		len += sizeof(mbr.uli.macro_enodeb_id2);
	}
	if (context->uli.extnded_macro_enb_id) {
		mbr.uli.extnded_macro_enb_id = context->uli.extnded_macro_enb_id;
		mbr.uli.extended_macro_enodeb_id2.emenbid_mcc_digit_1 =
			context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_1;
		mbr.uli.extended_macro_enodeb_id2.emenbid_mnc_digit_3 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_3;
		mbr.uli.extended_macro_enodeb_id2.emenbid_mcc_digit_3 =
			context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_3;
		mbr.uli.extended_macro_enodeb_id2.emenbid_mnc_digit_2 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_2;
		mbr.uli.extended_macro_enodeb_id2.emenbid_mnc_digit_1 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_1;
		mbr.uli.extended_macro_enodeb_id2.emenbid_smenb =
			context->uli.extended_macro_enodeb_id2.emenbid_smenb;
		mbr.uli.extended_macro_enodeb_id2.emenbid_spare =
			context->uli.extended_macro_enodeb_id2.emenbid_spare;
		mbr.uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id =
			context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id;
		mbr.uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2 =
			context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2;
		len += sizeof(mbr.uli.extended_macro_enodeb_id2);
	}
	len += 1;
	set_ie_header(&mbr.uli.header, GTP_IE_USER_LOC_INFO, IE_INSTANCE_ZERO, len);


	set_ie_header(&mbr.serving_network.header, GTP_IE_SERVING_NETWORK, IE_INSTANCE_ZERO,
			sizeof(gtp_serving_network_ie_t) - sizeof(ie_header_t));
	mbr.serving_network.mnc_digit_1 = context->serving_nw.mnc_digit_1;
	mbr.serving_network.mnc_digit_2 = context->serving_nw.mnc_digit_2;
	mbr.serving_network.mnc_digit_3 = context->serving_nw.mnc_digit_3;
	mbr.serving_network.mcc_digit_1 = context->serving_nw.mcc_digit_1;
	mbr.serving_network.mcc_digit_2 = context->serving_nw.mcc_digit_2;
	mbr.serving_network.mcc_digit_3 = context->serving_nw.mcc_digit_3;

	if (context->selection_flag) {
		mbr.selection_mode.spare2 = context->select_mode.spare2;
		mbr.selection_mode.selec_mode = context->select_mode.selec_mode;

		set_ie_header(&mbr.selection_mode.header, GTP_IE_SELECTION_MODE, IE_INSTANCE_ZERO,
			sizeof(uint8_t));
	}

	if(pdn->ue_time_zone_flag == true) {

		mbr.ue_time_zone.time_zone = pdn->ue_tz.tz;
		mbr.ue_time_zone.spare2 = 0;
		mbr.ue_time_zone.daylt_svng_time = pdn->ue_tz.dst;

		mbr.ue_time_zone.spare2 = 0;
		set_ie_header(&mbr.ue_time_zone.header, GTP_IE_UE_TIME_ZONE,
				IE_INSTANCE_ZERO, (sizeof(uint8_t) * 2));
	}

	set_ie_header(&mbr.bearer_contexts_to_be_modified.header, GTP_IE_BEARER_CONTEXT,
			IE_INSTANCE_ZERO, 0);
	set_ebi(&mbr.bearer_contexts_to_be_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			bearer->eps_bearer_id);

	mbr.bearer_contexts_to_be_modified.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

	/* Refer spec 23.274.Table 7.2.7-2 */
	bearer->s5s8_sgw_gtpu_ipv4.s_addr = (bearer->s5s8_sgw_gtpu_ipv4.s_addr);
	set_ipv4_fteid(&mbr.bearer_contexts_to_be_modified.s58_u_sgw_fteid,
			GTPV2C_IFTYPE_S5S8_SGW_GTPU,
			IE_INSTANCE_ONE,bearer->s5s8_sgw_gtpu_ipv4,
			(bearer->s5s8_sgw_gtpu_teid));

	mbr.bearer_contexts_to_be_modified.header.len += sizeof(struct fteid_ie_hdr_t) +
		sizeof(struct in_addr) + IE_HEADER_SIZE;

	uint16_t msg_len = 0;
	msg_len = encode_mod_bearer_req(&mbr, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

}


#if 0
void set_modify_bearer_request(gtpv2c_header *gtpv2c_tx, create_sess_req_t *csr,
		pdn_connection_t *pdn, eps_bearer_t *bearer)
{
	mod_bearer_req_t  mbr = {0};
	set_gtpv2c_teid_header((gtpv2c_header *)&mbr.header, GTP_MODIFY_BEARER_REQ,
			0, csr->header.teid.has_teid.seq);

	//AAQUIL need to remove
	mbr.header.teid.has_teid.teid = csr->pgw_s5s8_addr_ctl_plane_or_pmip.teid_gre_key;
	//0xeeffd000;

	//set_indication(&mbr.indication, IE_INSTANCE_ZERO);
	//mbr.indication.indication_value.sgwci = 1;

	pdn->s5s8_sgw_gtpc_ipv4.s_addr = htonl(pdn->s5s8_sgw_gtpc_ipv4.s_addr);


	if(csr->uli.header.len !=0) {
		//set_ie_copy(gtpv2c_tx, current_rx_ie);
		set_uli(&mbr.uli, csr, IE_INSTANCE_ZERO);
	}

	if(csr->serving_network.header.len !=0) {
		set_serving_network(&mbr.serving_network, csr, IE_INSTANCE_ZERO);
	}
	if(csr->ue_time_zone.header.len !=0) {
		set_ue_timezone(&mbr.ue_time_zone, csr, IE_INSTANCE_ZERO);
	}

	set_ipv4_fteid(&mbr.sender_fteid_ctl_plane, GTPV2C_IFTYPE_S5S8_SGW_GTPC,
			IE_INSTANCE_ZERO,
			(pdn->s5s8_sgw_gtpc_ipv4), pdn->s5s8_sgw_gtpc_teid);

	set_ie_header(&mbr.bearer_contexts_to_be_modified.header, GTP_IE_BEARER_CONTEXT,
			IE_INSTANCE_ZERO, 0);
	set_ebi(&mbr.bearer_contexts_to_be_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			bearer->eps_bearer_id);

	mbr.bearer_contexts_to_be_modified.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

	/* Refer spec 23.274.Table 7.2.7-2 */
	bearer->s5s8_sgw_gtpu_ipv4.s_addr = htonl(bearer->s5s8_sgw_gtpu_ipv4.s_addr);
	set_ipv4_fteid(&mbr.bearer_contexts_to_be_modified.s58_u_sgw_fteid,
			GTPV2C_IFTYPE_S5S8_SGW_GTPU,
			IE_INSTANCE_ONE,(bearer->s5s8_sgw_gtpu_ipv4),
			htonl(bearer->s5s8_sgw_gtpu_teid));
	//htonl(bearer->s1u_sgw_gtpu_teid));

	mbr.bearer_contexts_to_be_modified.header.len += sizeof(struct fteid_ie_hdr_t) +
		sizeof(struct in_addr) + IE_HEADER_SIZE;

	uint16_t msg_len = 0;
	encode_mod_bearer_req(&mbr, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.length = htons(msg_len - 4);
	LOG_MSG(LOG_DEBUG,"The length of mbr is %d and gtpv2c is %d", mbr.bearer_contexts_to_be_modified.header.len,
			gtpv2c_tx->gtpc.length);
}

#endif

static
int validate_mbreq_msg(msg_info_t *msg, mod_bearer_req_t *mb_req)
{
    ue_context_t *context = NULL;

	context = (ue_context_t *)get_ue_context(mb_req->header.teid.has_teid.teid);

	if (context == NULL) {
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    msg->ue_context = context;

	if (!mb_req->bearer_contexts_to_be_modified.eps_bearer_id.header.len
			|| !mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len) {
		LOG_MSG(LOG_ERROR, "Mandatory IE lbi/fteid missing in MBReq Dropping packet");
		return GTPV2C_CAUSE_INVALID_LENGTH;
	}

	uint8_t ebi_index = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	eps_bearer_t *bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		LOG_MSG(LOG_ERROR,
				"Received modify bearer on non-existent EBI - "
				"Bitmap Inconsistency - Dropping packet");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
    msg->bearer_context = bearer; 

    pdn_connection_t *pdn = GET_PDN(context, ebi_index);
    msg->pdn_context = pdn;
    return 0;
}

// Saegw, INITIAL_PDN_ATTACH_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT =>                   process_mb_req_handler
// saegw, INITIAL_PDN_ATTACH_PROC IDEL_STATE MB_REQ_RCVD_EVNT =>                        process_mb_req_handler 
// saegw, INITIAL_PDN_ATTACH_PROC,DDN_ACK_RCVD_STATE,MB_REQ_RCVD_EVNT,                  process_mb_req_handler 
// saegw, SGW_RELOCATION_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT                          process_mb_req_sgw_reloc_handler 
// saegw, SGW_RELOCATION_PROC IDEL_STATE MB_REQ_RCVD_EVNT                               process_mb_req_sgw_reloc_handler 
// saegw, SGW_RELOCATION_PROC DDN_ACK_RCVD_STATE MB_REQ_RCVD_EVNT DDN_ACK_RCVD_STATE=>  process_mb_req_handler   
// saegw  CONN_SUSPEND_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT =>process_mb_req_handler
// saegw  CONN_SUSPEND_PROC IDEL_STATE MB_REQ_RCVD_EVNT => process_mb_req_handler  
// saegw  CONN_SUSPEND_PROC DDN_ACK_RCVD_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler
// pgw SGW_RELOCATION_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT => process_mb_req_sgw_reloc_handler
// pgw SGW_RELOCATION_PROC IDEL_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_sgw_reloc_handler
// sgw INITIAL_PDN_ATTACH_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler
// sgw INITIAL_PDN_ATTACH_PROC IDEL_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler 
// sgw INITIAL_PDN_ATTACH_PROC DDN_ACK_RCVD_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler 
// sgw SGW_RELOCATION_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler
// sgw SGW_RELOCATION_PROC IDEL_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler 
// sgw CONN_SUSPEND_PROC CONNECTED_STATE MB_REQ_RCVD_EVNT - process_mb_req_handler 
// sgw CONN_SUSPEND_PROC IDEL_STATE MB_REQ_RCVD_EVNT ==> process_mb_req_handler
// sgw CONN_SUSPEND_PROC DDN_ACK_RCVD_STATE MB_REQ_RCVD_EVNT => process_mb_req_handler 


int
handle_modify_bearer_request(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    int ret;
    struct sockaddr_in *peer_addr;
    ue_context_t *context = NULL;
    uint32_t iface = msg->source_interface; 

    peer_addr = &msg->peer_addr;
    if(iface == S11_IFACE) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_MBREQ, peer_addr->sin_addr.s_addr);
    } else {
        increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_MBREQ, peer_addr->sin_addr.s_addr);
    }

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    /*Decode the received msg and stored into the struct. */
    if((ret = decode_mod_bearer_req((uint8_t *) gtpv2c_rx,
                    &msg->rx_msg.mbr) == 0)) {
        if(iface == S11_IFACE) {
            increment_mme_peer_stats(MSG_RX_GTPV2_S11_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        } else {
            increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        }
        return -1;
    }

    ret = validate_mbreq_msg(msg, &msg->rx_msg.mbr);
    if(ret != 0 ) {
        mbr_error_response(msg, ret,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        if(iface == S11_IFACE) {
            increment_mme_peer_stats(MSG_RX_GTPV2_S11_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        } else {
            increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        }
        return -1; 
    }
    assert(msg->msg_type == GTP_MODIFY_BEARER_REQ);

    uint32_t source_addr = msg->peer_addr.sin_addr.s_addr;
    uint16_t source_port = msg->peer_addr.sin_port;
    uint32_t seq_num = msg->rx_msg.mbr.header.teid.has_teid.seq;  
    transData_t *old_trans = (transData_t*)find_gtp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL)
    {
        if(iface == S11_IFACE) {
            increment_mme_peer_stats(MSG_RX_GTPV2_S11_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        } else {
            increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_MBREQ_DROP, peer_addr->sin_addr.s_addr);
        }
        LOG_MSG(LOG_DEBUG3, "Retransmitted MBReq received. Old MBReq is in progress");
        return -1;
    }

    /* TODO : identity correct procedure */
	if(cp_config->cp_type == PGWC) {
		msg->proc = SGW_RELOCATION_PROC;  

		gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);//0xd0ffee;


		context = (ue_context_t *)get_ue_context(msg->rx_msg.mbr.header.teid.has_teid.teid);
		if(context == NULL) {
            increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_MBRSP_REJ, peer_addr->sin_addr.s_addr);
			mbr_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
					    cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			return -1;
		}
		//uint8_t ebi_index = msg->rx_msg.mbr.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5;
		//pdn = GET_PDN(context, ebi_index);
		msg->event = MB_REQ_RCVD_EVNT;
		add_node_conn_entry(ntohl(msg->rx_msg.mbr.sender_fteid_ctl_plane.ipv4_address),
									S5S8_PGWC_PORT_ID);

        assert(NULL); /* ajay - add function */ 
	} else {
		/*Set the appropriate event type.*/
		msg->event = MB_REQ_RCVD_EVNT;

        /* Allocate new Proc */
        proc_context_t *mbreq_proc = alloc_service_req_proc(msg);

        /* allocate transaction */
        transData_t *trans = (transData_t *) calloc(1, sizeof(transData_t));  
        RESET_TRANS_SELF_INITIATED(trans);
        add_gtp_transaction(source_addr, source_port, seq_num, trans);
        trans->sequence = seq_num;
        trans->peer_sockaddr = msg->peer_addr;

        /* link transaction and proc */
        trans->proc_context = (void *)mbreq_proc;
        mbreq_proc->gtpc_trans = trans;
        
        start_procedure(mbreq_proc);
        // Note : important to note that we are holding on this msg now 
        *msg_p = NULL;
	}
    return 0;
}
