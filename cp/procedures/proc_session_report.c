// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "sm_hand.h"
#include "sm_struct.h"
#include "ue.h"
#include "stdint.h"
#include "pfcp_cp_set_ie.h"
#include "sm_structs_api.h"
#include "gtpv2_interface.h"
#include "sm_structs_api.h"
#include "sm_structs_api.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "proc_session_report.h"
#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "assert.h"

proc_context_t*
alloc_session_report_proc(msg_info_t *msg)
{
    proc_context_t *session_rep_proc;

    session_rep_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    session_rep_proc->proc_type = msg->proc; 
    session_rep_proc->ue_context = (void *)msg->ue_context;
    session_rep_proc->pdn_context = (void *)msg->pdn_context;
    session_rep_proc->handler = session_report_event_handler;

    msg->proc_context = session_rep_proc;
    SET_PROC_MSG(session_rep_proc, msg);
    
    return session_rep_proc;
}

void
session_report_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case PFCP_SESS_RPT_REQ_RCVD_EVNT: {
            process_rpt_req_handler(proc_context, msg);
            break;
        } 
        case DDN_ACK_RESP_RCVD_EVNT: {
            process_ddn_ack_rsp(proc_context, msg);
            break;
        }
        default: {
            assert(0); // wrong event 
        }
    }
    return;

}

void
proc_session_report_failure(msg_info_t *msg, uint8_t cause)
{
    proc_context_t *proc_context = (proc_context_t *)msg->proc_context;

    // send PFCP session report response with cause 

    proc_session_report_complete(proc_context);
    LOG_MSG(LOG_NEVER, "session report failed cause = %d ", cause);
    return;
}

void
proc_session_report_success(msg_info_t *msg)
{
    proc_context_t *proc_context = (proc_context_t *)msg->proc_context;
    proc_session_report_complete(proc_context);
    return;
}

void
proc_session_report_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
    return;
}


int
process_rpt_req_handler(proc_context_t *proc_ctxt, msg_info_t *msg)
{
    int ret = 0;
    pfcp_sess_rpt_req_t *pfcp_sess_rep_req = &msg->pfcp_msg.pfcp_sess_rep_req;
	uint64_t sess_id = pfcp_sess_rep_req->header.seid_seqno.has_seid.seid;
	uint8_t ebi =  UE_BEAR_ID(sess_id);
    ue_context_t *context = (ue_context_t *)msg->ue_context;

	if (proc_ctxt->proc_type == PAGING_PROC) {
        ret = send_ddn_indication(proc_ctxt, ebi);
		if (ret) {
			LOG_MSG(LOG_ERROR, "DDN (%d) ", ret);
            increment_mme_peer_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP, context->s11_mme_gtpc_ipv4.s_addr);
			return -1;
		}
		/* Update the Session state */
		proc_ctxt->state = DDN_REQ_SNT_STATE;
	} else {
        send_session_report_response(proc_ctxt, msg);
    }
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

void
process_ddn_ack_rsp(proc_context_t *proc_ctxt, msg_info_t *msg)
{
    send_session_report_response(proc_ctxt, msg);
}

void
send_session_report_response(proc_context_t *proc_ctxt, msg_info_t *msg)
{
	uint8_t pfcp_msg[250] = {0};
	int encoded = 0;
	pfcp_sess_rpt_rsp_t pfcp_sess_rep_resp = {0};
    ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
    transData_t *pfcp_trans = proc_ctxt->pfcp_trans;
    pdn_connection_t *pdn = (pdn_connection_t *)proc_ctxt->pdn_context;

	/*Fill and send pfcp session report response. */
	fill_pfcp_sess_report_resp(&pfcp_sess_rep_resp, pfcp_trans->sequence);

	pfcp_sess_rep_resp.header.seid_seqno.has_seid.seid = pdn->dp_seid;

	encoded =  encode_pfcp_sess_rpt_rsp_t(&pfcp_sess_rep_resp, pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSREPORTRSP, GET_UPF_ADDR(context->upf_context));
    proc_session_report_success(msg);
    return;
}


