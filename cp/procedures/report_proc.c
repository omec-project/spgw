// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "sm_hand.h"
#include "sm_struct.h"
#include "ue.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "rte_common.h"
#include "stdint.h"
#include "pfcp_cp_set_ie.h"
#include "sm_structs_api.h"
#include "gtpv2_interface.h"
#include "sm_structs_api.h"
#include "sm_structs_api.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "tables/tables.h"
#include "cp_io_poll.h"

int
process_rpt_req_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pfcp_report_req(&msg->pfcp_msg.pfcp_sess_rep_req);
	if (ret)
		return ret;

	RTE_SET_USED(unused_param);
	return 0;
}

static void
fill_pfcp_sess_report_resp(pfcp_sess_rpt_rsp_t *pfcp_sess_rep_resp,
		 uint32_t seq)
{
	memset(pfcp_sess_rep_resp, 0, sizeof(pfcp_sess_rpt_rsp_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_rep_resp->header),
		PFCP_SESSION_REPORT_RESPONSE, HAS_SEID, seq);

	set_cause(&(pfcp_sess_rep_resp->cause), REQUESTACCEPTED);

	//pfcp_sess_rep_resp->header.message_len = pfcp_sess_rep_resp->cause.header.len + 4;

	//pfcp_sess_rep_resp->header.message_len += sizeof(pfcp_sess_rep_resp->header.seid_seqno.has_seid);
}

uint8_t
process_pfcp_report_req(pfcp_sess_rpt_req_t *pfcp_sess_rep_req)
{
	/*DDN Handling */
	uint8_t ebi_index;
	int ret = 0, encoded = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint8_t pfcp_msg[250] = {0};
	pfcp_sess_rpt_rsp_t pfcp_sess_rep_resp = {0};
	uint64_t sess_id = pfcp_sess_rep_req->header.seid_seqno.has_seid.seid;

	uint32_t sequence = 0;
	uint32_t s11_sgw_gtpc_teid = UE_SESS_ID(sess_id);

	/* Stored the session information*/
	if (get_sess_entry_seid(sess_id, &context) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	ebi_index =  UE_BEAR_ID(sess_id) - 5;
	/* Retrive the s11 sgwc gtpc teid based on session id.*/
	sequence = pfcp_sess_rep_req->header.seid_seqno.has_seid.seq_no;
#if 0
	resp->msg_type = PFCP_SESSION_REPORT_REQUEST;
#endif

	clLog(sxlogger, eCLSeverityDebug, "DDN Request recv from DP for sess:%lu\n", sess_id);

	if (pfcp_sess_rep_req->report_type.dldr == 1) {
		ret = ddn_by_session_id(sess_id);
		if (ret) {
			clLog(clSystemLog, eCLSeverityCritical, "DDN %s: (%d) \n", __func__, ret);
			return -1;
		}
#if 0
		/* Update the Session state */
		pdn->state = DDN_REQ_SNT_STATE;
#endif
	}

	/* Update the UE State */
	ret = update_ue_state(s11_sgw_gtpc_teid,
			DDN_REQ_SNT_STATE, ebi_index);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State for teid: %u\n", __func__,
				s11_sgw_gtpc_teid);
	}

	/* Retrieve the UE context */
	ret = get_ue_context(s11_sgw_gtpc_teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					s11_sgw_gtpc_teid);
	}
	pdn = GET_PDN(context, ebi_index);
	pdn->state = DDN_REQ_SNT_STATE;

	/*Fill and send pfcp session report response. */
	fill_pfcp_sess_report_resp(&pfcp_sess_rep_resp,
			sequence);

	pfcp_sess_rep_resp.header.seid_seqno.has_seid.seid = pdn->dp_seid;

	encoded =  encode_pfcp_sess_rpt_rsp_t(&pfcp_sess_rep_resp, pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ) {
		clLog(sxlogger, eCLSeverityCritical, "Error REPORT REPONSE message: %i\n", errno);
		return -1;
	}
	else {
		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_rep_resp.header.message_type,ACC,SX);
	}

	return 0;
}
