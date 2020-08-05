// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <assert.h>
#include <errno.h>
#include <rte_debug.h>
#include "gtp_messages_decoder.h"
#include "clogger.h"
#include "gw_adapter.h"

/* TODO: Verify */
#include "ue.h"
#include "packet_filters.h"
#include "gtp_messages.h"
#include "gtpv2c_set_ie.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "pfcp_cp_set_ie.h"
#include "cp_config.h"
#include "cp_config_apis.h"  /* fetch APIs to build PCO */
#include "cp_stats.h"
#include "csid_api.h"
#include "stdint.h"
#include "stdbool.h"
#include "gtpv2_interface.h"
#include "ip_pool.h"
#include "upf_struct.h"
#include "cp_config_defs.h"
#include "spgw_cpp_wrapper.h"

extern uint32_t num_adc_rules;
extern uint32_t adc_rule_id[];

/*
 * @param: req_pco : this is the received PCO in the Request message (e.g. CSReq)
 * @dpId : This is DP id of the user context. 
 * @pco_buf : should have encoded pco which can be sent towards UE in GTP message (e.g. CSRsp)
 @ return : length of the PCO buf 
 */
static int16_t 
build_pco_response(char *pco_buf, pco_ie_t *req_pco, ue_context_t *context)
{
	uint16_t index = 0;
	int i = 0;
	uint8_t byte;
	byte = (req_pco->ext<<7) | (req_pco->config_proto & 0x03);
	memcpy(&pco_buf[index], &byte, sizeof(byte));
	index++;

    sub_profile_t *sub_prof = context->sub_prof;
    assert(sub_prof != NULL);
    apn_profile_t *apn_prof = sub_prof->apn_profile;
    assert(apn_prof != NULL);
    bool ipv4_link_mtu = false;
 
	for (i = 0; i < req_pco->num_of_opt; i++) {
		uint16_t pco_type = htons(req_pco->ids[i].type);
		uint8_t total_len;
		switch(req_pco->ids[i].type) {
			case PCO_ID_INTERNET_PROTOCOL_CONTROL_PROTOCOL:
				if (req_pco->ids[i].data[0] == 1) { /* Code : Configuration Request */
					total_len = 16;
					memcpy(&pco_buf[index], &pco_type, sizeof(pco_type));
					index += sizeof(pco_type);

					uint8_t len = total_len; 
					memcpy(&pco_buf[index], &len, sizeof(len));
					index += sizeof(len);

					uint8_t code=3;
					memcpy(&pco_buf[index], &code, sizeof(code));
					index += sizeof(code);

					uint8_t id=0;
					memcpy(&pco_buf[index], &id, sizeof(id));
					index += sizeof(id);

					uint16_t ppp_len = htons(total_len); 
					memcpy(&pco_buf[index], &ppp_len, sizeof(ppp_len));
					index += sizeof(ppp_len);

					/* Primary DNS Server IP Address */
					{
						uint8_t type=129; /* RFC 1877 Section 1.1  */
						memcpy(&pco_buf[index], &type, sizeof(type));
						index += sizeof(type);

						uint8_t len = 6; 
						memcpy(&pco_buf[index], &len, sizeof(len));
						index += sizeof(len);

						memcpy(&pco_buf[index], &apn_prof->dns_primary, 4);
						index += 4;
					}

					/* Secondary DNS Server IP Address */
					{
						uint8_t type=131; /* RFC 1877 Section 1.3 */
						memcpy(&pco_buf[index], &type, sizeof(type));
						index += sizeof(type);

						uint8_t len = 6; 
						memcpy(&pco_buf[index], &len, sizeof(len));
						index += sizeof(len);

						memcpy(&pco_buf[index], &apn_prof->dns_secondary, 4);
						index += 4;
					}
				}
				break;
			case PCO_ID_DNS_SERVER_IPV4_ADDRESS_REQUEST:
				{
					memcpy(&pco_buf[index], &pco_type, sizeof(pco_type));
					index += sizeof(pco_type);
				}
				{
					uint8_t len = 4; 
					memcpy(&pco_buf[index], &len, sizeof(len));
					index += sizeof(len);

					memcpy(&pco_buf[index], &apn_prof->dns_primary, 4);
					index += 4;
				}
				break;
			case PCO_ID_IP_ADDRESS_ALLOCATION_VIA_NAS_SIGNALLING:
				// allocation is always through NAS as of now 
				break;
			case PCO_ID_IPV4_LINK_MTU_REQUEST:
            {
                    ipv4_link_mtu=true;
                    memcpy(&pco_buf[index], &pco_type, sizeof(pco_type));
                    index += sizeof(pco_type);
                    uint16_t mtu = htons(apn_prof->mtu);
                    uint8_t len = 2;
                    memcpy(&pco_buf[index], &len, sizeof(len));
                    index += sizeof(len);
                    memcpy(&pco_buf[index], &mtu, 2);
                    index += 2;
            }
			break;
			default:
			    clLog(s11logger, eCLSeverityDebug,"Unknown PCO ID:(0x%x) received ", 
                                                req_pco->ids[i].type);
		}
	}
    // Lets add some additional parameters even if UE has requested for it
    if(ipv4_link_mtu == false)
    {
            uint16_t pco_type = htons(PCO_ID_IPV4_LINK_MTU_REQUEST);
            ipv4_link_mtu=true;
            memcpy(&pco_buf[index], &pco_type, sizeof(pco_type));
            index += sizeof(pco_type);
            uint16_t mtu = htons(apn_prof->mtu);
            uint8_t len = 2;
            memcpy(&pco_buf[index], &len, sizeof(len));
            index += sizeof(len);
            memcpy(&pco_buf[index], &mtu, 2);
            index += 2;
    }

	return index;
}

void
set_create_session_response(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, ue_context_t *context, pdn_connection_t *pdn,
		eps_bearer_t *bearer)
{
	uint8_t ebi_index = 0;
	struct in_addr ip = {0};
	upf_context_t *upf_ctx = context->upf_context;;
	create_sess_rsp_t cs_resp = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *)&cs_resp.header,
			GTP_CREATE_SESSION_RSP, context->s11_mme_gtpc_teid,
			sequence);

	set_cause_accepted(&cs_resp.cause, IE_INSTANCE_ZERO);

	ip.s_addr = ntohl(cp_config->s11_ip.s_addr);

	if ((context->s11_sgw_gtpc_teid != 0) && (ip.s_addr != 0)) {
		set_ipv4_fteid(&cs_resp.sender_fteid_ctl_plane,
				GTPV2C_IFTYPE_S11S4_SGW_GTPC,
				IE_INSTANCE_ZERO,
				ip, context->s11_sgw_gtpc_teid);
	}

	if ((pdn->s5s8_pgw_gtpc_teid != 0) && (pdn->s5s8_pgw_gtpc_ipv4.s_addr != 0)) {
		set_ipv4_fteid(&cs_resp.pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc,
				GTPV2C_IFTYPE_S5S8_PGW_GTPC,
				IE_INSTANCE_ONE,
				pdn->s5s8_pgw_gtpc_ipv4, pdn->s5s8_pgw_gtpc_teid);
	}

	pdn->ipv4.s_addr = htonl(pdn->ipv4.s_addr);
	printf("%s %d - UE address %s, pdn = %p\n",__FUNCTION__,__LINE__,inet_ntoa(pdn->ipv4), pdn);
	set_ipv4_paa(&cs_resp.paa, IE_INSTANCE_ZERO, pdn->ipv4);

	set_apn_restriction(&cs_resp.apn_restriction, IE_INSTANCE_ZERO,
			pdn->apn_restriction);
	{

		set_ie_header(&cs_resp.bearer_contexts_created.header, GTP_IE_BEARER_CONTEXT,
				IE_INSTANCE_ZERO, 0);


		set_ebi(&cs_resp.bearer_contexts_created.eps_bearer_id, IE_INSTANCE_ZERO,
				bearer->eps_bearer_id);

		cs_resp.bearer_contexts_created.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

		set_cause_accepted(&cs_resp.bearer_contexts_created.cause, IE_INSTANCE_ZERO);

		cs_resp.bearer_contexts_created.header.len += sizeof(struct cause_ie_hdr_t) + IE_HEADER_SIZE;

		if (bearer->s11u_mme_gtpu_teid) {
			clLog(s11logger, eCLSeverityDebug,"S11U Detect- set_create_session_response-"
					"\n\tbearer->s11u_mme_gtpu_teid= %X;"
					"\n\tGTPV2C_IFTYPE_S11U_MME_GTPU= %X\n",
					htonl(bearer->s11u_mme_gtpu_teid),
					GTPV2C_IFTYPE_S11U_SGW_GTPU);

			/* TODO: set fteid values to create session response member */
			/*
			clLog(s11logger, eCLSeverityDebug,"S11U Detect- set_create_session_response-"
					"\n\tbearer->s11u_mme_gtpu_teid= %X;"
					"\n\tGTPV2C_IFTYPE_S11U_MME_GTPU= %X\n",
					bearer->s11u_mme_gtpu_teid,
					GTPV2C_IFTYPE_S11U_SGW_GTPU);
			add_grouped_ie_length(bearer_context_group,
		    set_ipv4_fteid_ie(gtpv2c_tx, GTPV2C_IFTYPE_S11U_SGW_GTPU,
				    IE_INSTANCE_SIX, s1u_sgw_ip,
				    bearer->s1u_sgw_gtpu_teid));
			*/

		} else {

			ip.s_addr = upf_ctx->s1u_ip;

			if ((bearer->s1u_sgw_gtpu_teid != 0) && (ip.s_addr != 0)) {
				set_ipv4_fteid(&cs_resp.bearer_contexts_created.s1u_sgw_fteid,
				GTPV2C_IFTYPE_S1U_SGW_GTPU,
					IE_INSTANCE_ZERO, ip,
					bearer->s1u_sgw_gtpu_teid);

				cs_resp.bearer_contexts_created.header.len += sizeof(struct fteid_ie_hdr_t) +
				/* Merge conflict
					(bearer->s1u_sgw_gtpu_teid));
				cs_resp.bearer_context.header.len += sizeof(struct fteid_ie_hdr_t) +
				*/
					sizeof(struct in_addr) + IE_HEADER_SIZE;
			}
		}

		if ((bearer->s5s8_pgw_gtpu_teid != 0) && (bearer->s5s8_pgw_gtpu_ipv4.s_addr != 0)) {
			set_ipv4_fteid(&cs_resp.bearer_contexts_created.s5s8_u_pgw_fteid,
					GTPV2C_IFTYPE_S5S8_PGW_GTPU,
					IE_INSTANCE_TWO, bearer->s5s8_pgw_gtpu_ipv4,
					bearer->s5s8_pgw_gtpu_teid);

			cs_resp.bearer_contexts_created.header.len += sizeof(struct fteid_ie_hdr_t) +
					sizeof(struct in_addr) + IE_HEADER_SIZE;
		}

		ebi_index = bearer->eps_bearer_id - 5;
		set_ie_header(&cs_resp.bearer_contexts_created.bearer_lvl_qos.header,
				GTP_IE_BEARER_QLTY_OF_SVC, IE_INSTANCE_ZERO,
				sizeof(gtp_bearer_qlty_of_svc_ie_t) - sizeof(ie_header_t));

		cs_resp.bearer_contexts_created.bearer_lvl_qos.pvi =
			context->eps_bearers[ebi_index]->qos.arp.preemption_vulnerability;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.spare2 = 0;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.pl =
			context->eps_bearers[ebi_index]->qos.arp.priority_level;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.pci =
			context->eps_bearers[ebi_index]->qos.arp.preemption_capability;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.spare3 = 0;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.qci =
			context->eps_bearers[ebi_index]->qos.qci;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.max_bit_rate_uplnk =
			context->eps_bearers[ebi_index]->qos.ul_mbr;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.max_bit_rate_dnlnk =
			context->eps_bearers[ebi_index]->qos.dl_mbr;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_uplnk =
			context->eps_bearers[ebi_index]->qos.ul_gbr;
		cs_resp.bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk =
			context->eps_bearers[ebi_index]->qos.dl_gbr;

		cs_resp.bearer_contexts_created.header.len +=
			cs_resp.bearer_contexts_created.bearer_lvl_qos.header.len
			+ sizeof(ie_header_t);
	}

#ifdef USE_CSID
	fqcsid_t *csid = NULL;
	/* Get peer CSID associated with node */
	csid = get_peer_addr_csids_entry(context->s11_mme_gtpc_ipv4.s_addr,
			MOD);
	if ((csid != NULL) && (csid->num_csid)) {
		/* Set the SGW FQ-CSID */
		if ((context->sgw_fqcsid)->num_csid) {
			set_gtpc_fqcsid_t(&cs_resp.sgw_fqcsid, IE_INSTANCE_ONE,
					context->sgw_fqcsid);
		}

		/* Set the PGW FQ-CSID */
		if (cp_config->cp_type != SAEGWC) {
			if ((context->pgw_fqcsid)->num_csid) {
				set_gtpc_fqcsid_t(&cs_resp.pgw_fqcsid, IE_INSTANCE_ZERO,
						context->pgw_fqcsid);
				cs_resp.pgw_fqcsid.node_address = ntohl((context->pgw_fqcsid)->node_addr);
			}
		}
	}

#endif /* USE_CSID */
    if(context->pco != NULL)
    {
        char *pco_buf = calloc(1, 260);
        if (pco_buf != NULL) {
            //Should we even pass the CSReq in case of PCO not able to allocate ?
            //TODO -  pass upf context to build PCO 
            uint16_t len = build_pco_response(pco_buf, context->pco, context);
            set_pco(&cs_resp.pco_new, IE_INSTANCE_ZERO, pco_buf, len);
        }
    } 
	uint16_t msg_len = 0;
	msg_len = encode_create_sess_rsp(&cs_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}

int
process_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_s11_tx, gtpv2c_header_t *gtpv2c_s5s8_tx)
{
	create_sess_req_t csr = { 0 };
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	struct in_addr ue_ip;
	int ret;
	static uint32_t process_sgwc_s5s8_cs_req_cnt;
	static uint32_t process_spgwc_s11_cs_res_cnt;

	ret = decode_create_sess_req((uint8_t *) gtpv2c_rx,
			&csr);
	if (!ret)
		 return ret;

	if (csr.indctn_flgs.header.len &&
			csr.indctn_flgs.indication_uimsi) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d Unauthenticated IMSI Not Yet Implemented - "
				"Dropping packet\n",
			   __FILE__, __func__, __LINE__);
		return -EPERM;
	}

	if (!csr.indctn_flgs.header.len
			|| !csr.max_apn_rstrct.header.len
			|| !csr.bearer_contexts_to_be_created.header.len
			|| !csr.sender_fteid_ctl_plane.header.len
			|| !csr.pgw_s5s8_addr_ctl_plane_or_pmip.header.len
			|| !csr.imsi.header.len
			|| !csr.apn_ambr.header.len
			|| !csr.pdn_type.header.len
			|| !csr.bearer_contexts_to_be_created.bearer_lvl_qos.header.len
			|| !csr.msisdn.header.len
			|| !(csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4) ) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d Mandatory IE missing. Dropping packet\n",
			   __FILE__, __func__, __LINE__);
		return -EPERM;
	}

	if (csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV6 ||
			csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4V6) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d IPv6 Not Yet Implemented - Dropping packet\n",
			   __FILE__, __func__, __LINE__);
			return GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED;
	}

	apn_profile_t *apn_requested = match_apn_profile((char *)csr.apn.apn, csr.apn.header.len);

	if (!apn_requested)
		return GTPV2C_CAUSE_MISSING_UNKNOWN_APN;

	uint8_t ebi_index = csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;

	printf("\n acquire ip %d %s",__LINE__,__FUNCTION__);
	ret = acquire_ip(&ue_ip);
	if (ret)
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;

	/* set s11_sgw_gtpc_teid= key->ue_context_by_fteid_hash */
	ret = create_ue_context(&csr.imsi.imsi_number_digits, csr.imsi.header.len,
			csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi, &context, apn_requested,
			csr.header.teid.has_teid.seq);
	if (ret)
		return ret;

	if (csr.mei.header.len)
		memcpy(&context->mei, &csr.mei.mei, csr.mei.header.len);

	memcpy(&context->msisdn, &csr.msisdn.msisdn_number_digits, csr.msisdn.header.len);

	context->s11_sgw_gtpc_ipv4 = cp_config->s11_ip;
	context->s11_mme_gtpc_teid = csr.sender_fteid_ctl_plane.teid_gre_key;
	struct in_addr peer_addr_temp; 
    peer_addr_temp.s_addr = csr.sender_fteid_ctl_plane.ipv4_address;
	context->s11_mme_gtpc_ipv4 = peer_addr_temp; 

	pdn = context->eps_bearers[ebi_index]->pdn;
	{
		pdn->apn_in_use = apn_requested;
		pdn->apn_ambr.ambr_downlink = csr.apn_ambr.apn_ambr_dnlnk;
		pdn->apn_ambr.ambr_uplink = csr.apn_ambr.apn_ambr_uplnk;
		pdn->apn_restriction = csr.max_apn_rstrct.rstrct_type_val;
		pdn->ipv4.s_addr = htonl(ue_ip.s_addr);

		if (csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4)
			pdn->pdn_type.ipv4 = 1;
		else if (csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV6)
			pdn->pdn_type.ipv6 = 1;
		else if (csr.pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4V6) {
			pdn->pdn_type.ipv4 = 1;
			pdn->pdn_type.ipv6 = 1;
		}

		if (csr.chrgng_char.header.len)
			memcpy(&pdn->charging_characteristics,
					&csr.chrgng_char.chrgng_char_val,
					sizeof(csr.chrgng_char.chrgng_char_val));

		pdn->s5s8_sgw_gtpc_ipv4 = cp_config->s5s8_ip;
		/* Note: s5s8_sgw_gtpc_teid =
		 * s11_sgw_gtpc_teid
		 */
		pdn->s5s8_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
		/* Note: s5s8_pgw_gtpc_teid updated by
		 * process_sgwc_s5s8_create_session_response (...)
		 */
		pdn->s5s8_pgw_gtpc_ipv4.s_addr = csr.pgw_s5s8_addr_ctl_plane_or_pmip.ipv4_address;
	}

	bearer = context->eps_bearers[ebi_index];
	{
		/* TODO: Implement TFTs on default bearers
		   if (create_session_request.bearer_tft_ie) {
		   }
		   */

		bearer->qos.ul_mbr =
			csr.bearer_contexts_to_be_created.bearer_lvl_qos.max_bit_rate_uplnk;
		bearer->qos.dl_mbr =
			csr.bearer_contexts_to_be_created.bearer_lvl_qos.max_bit_rate_dnlnk;
		bearer->qos.ul_gbr =
			csr.bearer_contexts_to_be_created.bearer_lvl_qos.guarntd_bit_rate_uplnk;
		bearer->qos.dl_gbr =
			csr.bearer_contexts_to_be_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk;

		set_s1u_sgw_gtpu_teid(bearer, context);
		/* Note: s5s8_sgw_gtpu_teid based s11_sgw_gtpc_teid
		 * Computation same as s1u_sgw_gtpu_teid
		 */
		set_s5s8_sgw_gtpu_teid(bearer, context);
		bearer->pdn = pdn;
	}

	if (cp_config->cp_type == SGWC) {
		char sgwu_fqdn[MAX_HOSTNAME_LENGTH] = {0};
		ret =
			gen_sgwc_s5s8_create_session_request(gtpv2c_rx,
				gtpv2c_s5s8_tx, csr.header.teid.has_teid.seq,
				pdn, bearer, sgwu_fqdn);

		 clLog(s5s8logger, eCLSeverityDebug, "NGIC- create_session.c::"
				"\n\tprocess_create_session_request::case= %d;"
				"\n\tprocess_sgwc_s5s8_cs_req_cnt= %u;"
				"\n\tgen_create_s5s8_session_request= %d\n",
				cp_config->cp_type, process_sgwc_s5s8_cs_req_cnt++,
				ret);
		return ret;
	}

	printf("\n ******* call set_create_session_response from %d %s \n",__LINE__,__FUNCTION__);
	set_create_session_response(
			gtpv2c_s11_tx, csr.header.teid.has_teid.seq,
			context, pdn, bearer);

	clLog(s11logger, eCLSeverityDebug, "NGIC- create_session.c::"
			"\n\tprocess_create_session_request::case= %d;"
			"\n\tprocess_spgwc_s11_cs_res_cnt= %u;"
			"\n\tset_create_session_response::done...\n",
			cp_config->cp_type, process_spgwc_s11_cs_res_cnt++);

	/* using the s1u_sgw_gtpu_teid as unique identifier to the session */
	struct session_info session;
	memset(&session, 0, sizeof(session));

	session.ue_addr.iptype = IPTYPE_IPV4;
	session.ue_addr.u.ipv4_addr = pdn->ipv4.s_addr;
	session.ul_s1_info.sgw_teid = htonl(bearer->s1u_sgw_gtpu_teid);
	session.ul_s1_info.sgw_addr.iptype = IPTYPE_IPV4;
	session.ul_s1_info.sgw_addr.u.ipv4_addr =
			htonl(bearer->s1u_sgw_gtpu_ipv4.s_addr);

	if (bearer->s11u_mme_gtpu_teid) {
		/* If CIOT: [enb_addr,enb_teid] =
		 * s11u[mme_gtpu_addr, mme_gtpu_teid]
		 */
		session.ul_s1_info.enb_addr.iptype = IPTYPE_IPV4;
		session.ul_s1_info.enb_addr.u.ipv4_addr =
			htonl(bearer->s11u_mme_gtpu_ipv4.s_addr);
		session.dl_s1_info.enb_teid =
			htonl(bearer->s11u_mme_gtpu_teid);
		session.dl_s1_info.enb_addr.iptype = IPTYPE_IPV4;
		session.dl_s1_info.enb_addr.u.ipv4_addr =
			htonl(bearer->s11u_mme_gtpu_ipv4.s_addr);
	} else {
		session.ul_s1_info.enb_addr.iptype = IPTYPE_IPV4;
		session.ul_s1_info.enb_addr.u.ipv4_addr =
			htonl(bearer->s1u_enb_gtpu_ipv4.s_addr);
		session.dl_s1_info.enb_teid =
			htonl(bearer->s1u_enb_gtpu_teid);
		session.dl_s1_info.enb_addr.iptype = IPTYPE_IPV4;
		session.dl_s1_info.enb_addr.u.ipv4_addr =
			htonl(bearer->s1u_enb_gtpu_ipv4.s_addr);
	}

	session.dl_s1_info.sgw_addr.iptype = IPTYPE_IPV4;
	session.dl_s1_info.sgw_addr.u.ipv4_addr =
			htonl(bearer->s1u_sgw_gtpu_ipv4.s_addr);
	session.ul_apn_mtr_idx = ulambr_idx;
	session.dl_apn_mtr_idx = dlambr_idx;
	session.num_ul_pcc_rules = 1;
	session.num_dl_pcc_rules = 1;
	session.ul_pcc_rule_id[0] = FIRST_FILTER_ID;
	session.dl_pcc_rule_id[0] = FIRST_FILTER_ID;

	/* using ue ipv4 addr as unique identifier for an UE.
	 * and sess_id is combination of ue addr and bearer id.
	 * formula to set sess_id = (ue_ipv4_addr << 4) | bearer_id
	 */
	session.sess_id = SESS_ID(context->s11_sgw_gtpc_teid,
						bearer->eps_bearer_id);

	struct dp_id dp_id = { .id = DPN_ID };
    RTE_SET_USED(dp_id);
    

#ifdef OBSOLETE_APIS
	if (session_create(dp_id, session) < 0)
		rte_exit(EXIT_FAILURE,"Bearer Session create fail !!!");
#endif

	if (bearer->s11u_mme_gtpu_teid) {
		session.num_dl_pcc_rules = 1;
		session.dl_pcc_rule_id[0] = FIRST_FILTER_ID;

		session.num_adc_rules = num_adc_rules;
		uint32_t i;
		for (i = 0; i < num_adc_rules; ++i)
			        session.adc_rule_id[i] = adc_rule_id[i];

#ifdef OBSOLETE_APIS
		if (session_modify(dp_id, session) < 0)
			rte_exit(EXIT_FAILURE, "Bearer Session create CIOT implicit modify fail !!!");
#endif
	}

	return 0;
}
