// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "sm_structs_api.h"
#include "tables/tables.h"
#include "spgw_cpp_wrapper.h"

// saegw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
// saegw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
static
int handle_pfcp_session_report_req_msg(msg_info_t *msg)
{
    ue_context_t *context = NULL;
    assert(msg->msg_type == PFCP_SESSION_REPORT_REQUEST);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive the session information based on session id. */
	if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
				&context) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, Sess ID:%lu\n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid);
		return -1;
	}

    /* Report is handleed for various cases..
     * case 1: Connection idle - UE DDN packet indication  
     * case 2: Connection is active. UE usage report.
     * case 3: Connection is active. Error indication is received. 
     */
	msg->event = PFCP_SESS_RPT_REQ_RCVD_EVNT;
    /* For time being just getting rid of 3d FSM array */
	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_REPORT_REQUEST[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

    process_rpt_req_handler((void *)msg, NULL);
    return 0;
}

int 
handle_session_report_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct*/
    int decoded = decode_pfcp_sess_rpt_req_t((uint8_t *)pfcp_rx,

            &msg->pfcp_msg.pfcp_sess_rep_req);

    if(decoded <= 0)
    {

        clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Report Request is %d\n",
                decoded);
        // TODOSTATISTICS
        // increment_userplane_stats(MSG_RX_PFCP_REPORT_DECODE_ERR, peer_addr.sin_addr.s_addr);
        return -1;
    }

    increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ, peer_addr.sin_addr.s_addr);

    handle_pfcp_session_report_req_msg(msg);
    return 0;
}


