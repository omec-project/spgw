// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

// saegw, INITIAL_PDN_ATTACH_PROC, CS_REQ_SNT_STATE, CS_RESP_RCVD_EVNT, process_cs_resp_handler
// saegw, SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT -> process_cs_resp_handler 
// pgw - INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 
// pgw - SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw   INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 

#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "cp_log.h"
#include "ue.h"
#include "pfcp_enum.h"
#include "spgw_cpp_wrapper.h"
#include "util.h"
#include "gtpv2_error_rsp.h"
#include "cp_transactions.h"
#include "cp_io_poll.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_encoder.h"
#include "spgw_config_struct.h"
#include "sm_structs_api.h"
#include "pfcp_cp_session.h"
#include "cp_peer.h"
#include "pfcp_cp_interface.h"
#include "proc.h"


void
fill_pgwc_create_session_response(create_sess_rsp_t *cs_resp,
		uint32_t sequence, ue_context_t *context, uint8_t ebi_index)
{

	set_gtpv2c_header(&cs_resp->header, 1, GTP_CREATE_SESSION_RSP,
			context->pdns[ebi_index]->s5s8_sgw_gtpc_teid, sequence);

	set_cause_accepted(&cs_resp->cause, IE_INSTANCE_ZERO);

	set_ipv4_fteid(
			&cs_resp->pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc,
			GTPV2C_IFTYPE_S5S8_PGW_GTPC, IE_INSTANCE_ONE,
			context->pdns[ebi_index]->s5s8_pgw_gtpc_ipv4,
			context->pdns[ebi_index]->s5s8_pgw_gtpc_teid);

	/* TODO: Added Temp Fix for the UE IP*/
	struct in_addr ipv4 = {0};
	context->pdns[ebi_index]->ipv4.s_addr = htonl(context->pdns[ebi_index]->ipv4.s_addr);
	ipv4.s_addr = context->pdns[ebi_index]->ipv4.s_addr;
	set_ipv4_paa(&cs_resp->paa, IE_INSTANCE_ZERO, ipv4);
			//context->pdns[ebi_index]->ipv4);

	set_apn_restriction(&cs_resp->apn_restriction, IE_INSTANCE_ZERO,
			context->pdns[ebi_index]->apn_restriction);

	set_ebi(&cs_resp->bearer_contexts_created.eps_bearer_id,
			IE_INSTANCE_ZERO,
			context->eps_bearers[ebi_index]->eps_bearer_id);
	set_cause_accepted(&cs_resp->bearer_contexts_created.cause,
			IE_INSTANCE_ZERO);
	set_ie_header(&cs_resp->bearer_contexts_created.bearer_lvl_qos.header,
			GTP_IE_BEARER_QLTY_OF_SVC, IE_INSTANCE_ZERO,
			sizeof(gtp_bearer_qlty_of_svc_ie_t) - sizeof(ie_header_t));
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pvi =
		context->eps_bearers[ebi_index]->qos.arp.preemption_vulnerability;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.spare2 = 0;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pl =
		context->eps_bearers[ebi_index]->qos.arp.priority_level;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.pci =
		context->eps_bearers[ebi_index]->qos.arp.preemption_capability;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.spare3 = 0;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.qci =
		context->eps_bearers[ebi_index]->qos.qci;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_uplnk =
		context->eps_bearers[ebi_index]->qos.ul_mbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.max_bit_rate_dnlnk =
		context->eps_bearers[ebi_index]->qos.dl_mbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_uplnk =
		context->eps_bearers[ebi_index]->qos.ul_gbr;
	cs_resp->bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk =
		context->eps_bearers[ebi_index]->qos.dl_gbr;

	context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4.s_addr =
		        htonl(context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4.s_addr);
	set_ipv4_fteid(&cs_resp->bearer_contexts_created.s5s8_u_pgw_fteid,
			GTPV2C_IFTYPE_S5S8_PGW_GTPU, IE_INSTANCE_TWO,
			context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_ipv4,
			context->eps_bearers[ebi_index]->s5s8_pgw_gtpu_teid);

	set_ie_header(&cs_resp->bearer_contexts_created.header,
			GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO,
			(cs_resp->bearer_contexts_created.eps_bearer_id.header.len
			 + sizeof(ie_header_t)
			 + cs_resp->bearer_contexts_created.cause.header.len
			 + sizeof(ie_header_t)
			 + cs_resp->bearer_contexts_created.s5s8_u_pgw_fteid.header.len
			 + sizeof(ie_header_t))
			 + cs_resp->bearer_contexts_created.bearer_lvl_qos.header.len
			 + sizeof(ie_header_t));
}


