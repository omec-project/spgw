// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <errno.h>
#include "gtp_messages_decoder.h"
#include "cp_log.h"
#include "ue.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "vepc_cp_dp_api.h"
#include "pfcp_cp_set_ie.h"
#include "spgw_config_struct.h"
#include "cp_config_apis.h"  /* fetch APIs to build PCO */
#include "stdint.h"
#include "stdbool.h"
#include "gtpv2_interface.h"
#include "ip_pool.h"
#include "upf_struct.h"
#include "cp_config_defs.h"
#include "spgw_cpp_wrapper.h"
#include "cp_peer.h"
#include "gtpv2_error_rsp.h"
#include "proc_initial_attach.h"
#include "util.h"
#include "gtpv2_session.h"
#include "proc_sgw_relocation.h"
#include "proc.h"

extern uint32_t num_adc_rules;
extern uint32_t adc_rule_id[];

/*
 * @param: req_pco : this is the received PCO in the Request message (e.g. CSReq)
 * @pco_buf : should have encoded pco which can be sent towards UE in GTP message (e.g. CSRsp)
 @ return : length of the PCO buf 
 */
static int16_t 
build_pco_response(proc_context_t *proc_ctxt, char *pco_buf, ue_context_t *context)
{
	uint16_t index = 0;
	int i = 0;
	uint8_t byte;
	pco_ie_t *req_pco = (pco_ie_t *)proc_ctxt->req_pco;
	byte = (req_pco->ext<<7) | (req_pco->config_proto & 0x03);
	memcpy(&pco_buf[index], &byte, sizeof(byte));
	index++;

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

						sub_config_t *sub_conf = (sub_config_t *)proc_ctxt->sub_config;
						memcpy(&pco_buf[index], &sub_conf->dns_primary, 4);
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

  						sub_config_t *sub_conf = (sub_config_t *)proc_ctxt->sub_config;
						memcpy(&pco_buf[index], &sub_conf->dns_secondary, 4);
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

					sub_config_t *sub_conf = (sub_config_t *)proc_ctxt->sub_config;
					memcpy(&pco_buf[index], &sub_conf->dns_primary, 4);
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
                    sub_config_t *sub_conf = (sub_config_t *)proc_ctxt->sub_config;
                    uint16_t mtu = htons(sub_conf->mtu);
                    uint8_t len = 2;
                    memcpy(&pco_buf[index], &len, sizeof(len));
                    index += sizeof(len);
                    memcpy(&pco_buf[index], &mtu, 2);
                    index += 2;
            }
			break;
			default:
			    LOG_MSG(LOG_DEBUG,"Unknown PCO ID:(0x%x) received ", 
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
            sub_config_t *sub_conf = (sub_config_t *)proc_ctxt->sub_config;
            uint16_t mtu = htons(sub_conf->mtu);
            uint8_t len = 2;
            memcpy(&pco_buf[index], &len, sizeof(len));
            index += sizeof(len);
            memcpy(&pco_buf[index], &mtu, 2);
            index += 2;
    }

	return index;
}

void
set_create_session_response(void *proc, gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, ue_context_t *context, pdn_connection_t *pdn,
		eps_bearer_t *bearer)
{
	uint8_t ebi_index = 0;
	struct in_addr ip = {0};
	upf_context_t *upf_ctx = context->upf_context;;
	create_sess_rsp_t cs_resp = {0};
	proc_context_t *proc_ctxt = (proc_context_t*) proc;

	set_gtpv2c_teid_header((gtpv2c_header_t *)&cs_resp.header,
			GTP_CREATE_SESSION_RSP, context->s11_mme_gtpc_teid,
			sequence);

	set_cause_accepted(&cs_resp.cause, IE_INSTANCE_ZERO);
    set_apn_ambr(&cs_resp.apn_ambr, pdn->apn_ambr.ambr_uplink, pdn->apn_ambr.ambr_downlink); 

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

	LOG_MSG(LOG_DEBUG, "UE address %s, pdn = %p",inet_ntoa(pdn->ipv4), pdn);
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
			LOG_MSG(LOG_DEBUG,"S11U Detect- set_create_session_response-"
					"\tbearer->s11u_mme_gtpu_teid= %X;"
					"\tGTPV2C_IFTYPE_S11U_MME_GTPU= %X",
					htonl(bearer->s11u_mme_gtpu_teid),
					GTPV2C_IFTYPE_S11U_SGW_GTPU);

			/* TODO: set fteid values to create session response member */
			/*
			LOG_MSG(LOG_DEBUG,"S11U Detect- set_create_session_response-"
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

    if(proc_ctxt->req_pco != NULL) {
        char *pco_buf = (char *)calloc(1, 260);
        if (pco_buf != NULL) {
            uint16_t len = build_pco_response(proc_ctxt, pco_buf, context);
            set_pco(&cs_resp.pco_new, IE_INSTANCE_ZERO, pco_buf, len);
        }
    } 
	uint16_t msg_len = 0;
	msg_len = encode_create_sess_rsp(&cs_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
}

static 
int validate_csreq_msg(create_sess_req_t *csr) 
{
	if (csr->indctn_flgs.header.len &&
			csr->indctn_flgs.indication_uimsi) {
		LOG_MSG(LOG_ERROR, "Unauthenticated IMSI Not Yet Implemented - Dropping packet");
	
		return GTPV2C_CAUSE_IMSI_NOT_KNOWN;
	}

	if (/*!csr->max_apn_rstrct.header.len
			||*/ !csr->bearer_contexts_to_be_created.header.len
			|| !csr->sender_fteid_ctl_plane.header.len
			|| !csr->imsi.header.len
			|| !csr->apn_ambr.header.len
			|| !csr->pdn_type.header.len
			|| !csr->bearer_contexts_to_be_created.bearer_lvl_qos.header.len
			|| !csr->rat_type.header.len
			|| !(csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4) ) {
		LOG_MSG(LOG_ERROR, "Mandatory IE missing. Dropping packet");
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;
	}

	if (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV6 ||
			csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4V6) {
		LOG_MSG(LOG_ERROR, "IPv6 Not Yet Implemented - Dropping packet");
		return GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED;
	}
    return 0;
}

static int
decode_check_csr(gtpv2c_header_t *gtpv2c_rx,
		create_sess_req_t *csr)
{
	int ret = 0;
	ret = decode_create_sess_req((uint8_t *) gtpv2c_rx,
			csr);

	if (!ret){
		LOG_MSG(LOG_ERROR, "Decoding for csr req failed");
		return -1;
	}

	return 0;
}

int
handle_create_session_request(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    int ret;
    struct sockaddr_in *peer_addr;
    proc_context_t *csreq_proc = NULL;

    msg->source_interface = S11_IFACE;
    peer_addr = &msg->peer_addr;

    increment_mme_peer_stats(MSG_RX_GTPV2_S11_CSREQ, peer_addr->sin_addr.s_addr);


    peer_addr = &msg->peer_addr;

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    // check for invalid length and invalid version 

	msg->msg_type = gtpv2c_rx->gtpc.message_type;

    if ((ret = decode_check_csr(gtpv2c_rx, &msg->rx_msg.csr)) != 0) {
        if(ret != -1)
            cs_error_response(msg, ret,
                    cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        return -1;
    }

    if(msg->rx_msg.csr.up_func_sel_indctn_flgs.dcnr) {
        LOG_MSG(LOG_DEBUG, "Received CSReq for dcnr capable UE ");
    }

    ret = validate_csreq_msg(&msg->rx_msg.csr);
    if(ret != 0 ) {
        cs_error_response(msg, ret,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        return -1;
    }

    // update statistics ??
    assert(msg->msg_type == GTP_CREATE_SESSION_REQ);

    /* Find old transaction */
    uint32_t source_addr = msg->peer_addr.sin_addr.s_addr;
    uint16_t source_port = msg->peer_addr.sin_port;
    uint32_t seq_num = msg->rx_msg.csr.header.teid.has_teid.seq;  
    transData_t *old_trans = (transData_t*)find_gtp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL) {
        LOG_MSG(LOG_DEBUG0, "Retransmitted CSReq received. Old CSReq is in progress");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_CSREQ_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }

    /*
     * Requirement1 : Create transaction. If new CSReq is received and previous
     *                CSreq is already in progress then drop retransmitted CSReq 
     * Requirement1 : Detect Context Replacement 
     * Requirement2 : Support guard timer. If retransmitted CSReq is received 
     *       after sending CSRsp then just sent back same CSrsp or drop CSReq.
     */
    ue_context_t *context = NULL; 
    uint64_t imsi;
    imsi = msg->rx_msg.csr.imsi.imsi_number_digits;
    context = (ue_context_t *)ue_context_entry_lookup_imsiKey(imsi);
    if(context != NULL) {
        LOG_MSG(LOG_DEBUG0, "[IMSI - %lu] Detected context replacement ", context->imsi64);
        msg->msg_type = GTP_RESERVED; 
        msg->ue_context = context;
        decrement_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
        uint8_t ebi = msg->rx_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
        pdn_connection_t *pdn_old = GET_PDN(context, ebi);
        if (pdn_old && pdn_old->ipv4.s_addr != 0) {
          decrement_ue_info_stats(SUBSCRIBERS_INFO_SPGW_PDN, context->imsi64, pdn_old->ipv4.s_addr);
        }
        process_error_occured_handler_new((void *)msg, NULL);
        LOG_MSG(LOG_DEBUG0, "[IMSI - %lu] Deleted old call due to context replacement ", context->imsi64);
        msg->msg_type = GTP_CREATE_SESSION_REQ; 
        msg->ue_context = NULL;
    }
   
    if(msg->source_interface == S5S8_IFACE) {
        /* when CSR received from SGWC add it as a peer*/
	    add_node_conn_entry(ntohl(msg->rx_msg.csr.sender_fteid_ctl_plane.ipv4_address),
								S5S8_SGWC_PORT_ID);
    } else if(msg->source_interface == S11_IFACE) {
        /* when CSR received from MME add it as a peer*/
	    add_node_conn_entry(ntohl(msg->rx_msg.csr.sender_fteid_ctl_plane.ipv4_address),
								S11_SGW_PORT_ID );
    }

    /* Create new transaction */
    transData_t *trans = (transData_t *) calloc(1, sizeof(transData_t));  
    RESET_TRANS_SELF_INITIATED(trans);
    add_gtp_transaction(source_addr, source_port, seq_num, trans);

	/*Set the appropriate event type.*/
	msg->event = CS_REQ_RCVD_EVNT;

    if (1 == msg->rx_msg.csr.indctn_flgs.indication_oi) {
        /* SGW Relocation Case */
        csreq_proc = alloc_sgw_relocation_proc(msg);
    } else { 
        /* SGW + PGW + SAEGW case */
        csreq_proc = alloc_initial_proc(msg);
    }

    assert(csreq_proc != NULL);
    trans->proc_context = (void *)csreq_proc;
    csreq_proc->gtpc_trans = trans;
    trans->sequence = seq_num;
    trans->peer_sockaddr = msg->peer_addr;
    start_procedure(csreq_proc);
    // NOTE : this is important so that caller does not free msg pointer 
    *msg_p = NULL;
    return 0;
}
