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
#include "spgw_cpp_wrapper.h"
#include "proc_session_report.h"

proc_context_t*
alloc_session_report_proc(msg_info_t **msg)
{
    /* Now onwards only refer msg_info coming from */
    msg_info_t *new_msg = calloc(1, sizeof(msg_info_t));
    memcpy(new_msg, *msg, sizeof(msg_info_t));

    proc_context_t *session_rep_proc;

    session_rep_proc = calloc(1, sizeof(proc_context_t));
    session_rep_proc->proc_type = new_msg->proc; 
    session_rep_proc->ue_context = (void *)new_msg->ue_context;
    session_rep_proc->pdn_context = (void *)new_msg->pdn_context;
    session_rep_proc->handler = session_report_event_handler;

    new_msg->proc_context = session_rep_proc;
    session_rep_proc->msg_info = (void *) new_msg;
    
    *msg = new_msg;

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
            rte_panic("wrong event");
        }
    }
    return;

}

void
proc_session_report_failure(msg_info_t *msg, uint8_t cause)
{
    proc_context_t *proc_context = msg->proc_context;
    RTE_SET_USED(cause);

    // send PFCP session report response with cause 

    proc_session_report_complete(proc_context);
    return;
}

void
proc_session_report_success(msg_info_t *msg)
{
    proc_context_t *proc_context = msg->proc_context;
    proc_session_report_complete(proc_context);
    return;
}

void
proc_session_report_complete(proc_context_t *proc_context)
{
    assert(proc_context->gtpc_trans != NULL);
    transData_t *gtpc_trans = proc_context->gtpc_trans;

    if(gtpc_trans != NULL) {
        uint16_t port_num = gtpc_trans->peer_sockaddr.sin_port; 
        uint32_t sender_addr = gtpc_trans->peer_sockaddr.sin_addr.s_addr; 
        uint32_t seq_num = gtpc_trans->sequence; 

        transData_t *temp= delete_gtp_transaction(sender_addr, port_num, seq_num);
        if(temp != NULL) {
            /* Let's cross check if transaction from the table is matchig with the one we have 
            * in subscriber 
            */
            assert(gtpc_trans == temp);
        }
        free(gtpc_trans);
    }
    proc_context->gtpc_trans =  NULL;

    transData_t *pfcp_trans = proc_context->pfcp_trans;
    if(pfcp_trans != NULL) {
        uint16_t port_num = pfcp_trans->peer_sockaddr.sin_port; 
        uint32_t sender_addr = pfcp_trans->peer_sockaddr.sin_addr.s_addr; 
        uint32_t seq_num = pfcp_trans->sequence; 

        transData_t *temp = delete_gtp_transaction(sender_addr, port_num, seq_num);
        if(temp != NULL) {
            /* Let's cross check if transaction from the table is matchig with the one we have 
            * in subscriber 
            */
            assert(pfcp_trans == temp);
        }
        free(pfcp_trans);
    }
    proc_context->pfcp_trans = NULL;
    /* PFCP transaction is already complete. */
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
    ue_context_t *context = msg->ue_context;

	if (proc_ctxt->proc_type == PAGING_PROC) {
        ret = send_ddn_indication(proc_ctxt, ebi);
		if (ret) {
			clLog(clSystemLog, eCLSeverityCritical, "DDN %s: (%d) \n", __func__, ret);
            increment_mme_peer_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP, context->s11_mme_gtpc_ipv4.s_addr);
			return -1;
		}
		/* Update the Session state */
		proc_ctxt->state = DDN_REQ_SNT_STATE;
	} else {
        // various other report triggers 
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
	uint8_t pfcp_msg[250] = {0};
	int encoded = 0;
	pfcp_sess_rpt_rsp_t pfcp_sess_rep_resp = {0};
    ue_context_t *context = proc_ctxt->ue_context;
    transData_t *pfcp_trans = proc_ctxt->pfcp_trans;
    pdn_connection_t *pdn = proc_ctxt->pdn_context;

	/*Fill and send pfcp session report response. */
	fill_pfcp_sess_report_resp(&pfcp_sess_rep_resp, pfcp_trans->sequence);

	pfcp_sess_rep_resp.header.seid_seqno.has_seid.seid = pdn->dp_seid;

	encoded =  encode_pfcp_sess_rpt_rsp_t(&pfcp_sess_rep_resp, pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ) {
		clLog(sxlogger, eCLSeverityCritical, "Error REPORT REPONSE message: %i\n", errno);
		return;
	}
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSREPORTRSP, GET_UPF_ADDR(context->upf_context));
    proc_session_report_success(msg);
    return;
}


