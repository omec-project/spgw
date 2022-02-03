// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
#include "proc_session_report.h"
#include "proc.h"

// Triggers to receive session report are
// 1. Downlink packet received when user is in idle state
// 2. Usage Report is sent by userplane
// 3. Error indication is received by userplane
// 4. User plane inactivity report.
// Single Session Report can come for 1 or more reason

// saegw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
// saegw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
static
int handle_pfcp_session_report_req_msg(msg_info_t *msg)
{
    struct sockaddr_in *peer_addr = &msg->peer_addr;
    ue_context_t *context = (ue_context_t *)msg->ue_context;
    assert(msg->msg_type == PFCP_SESSION_REPORT_REQUEST);
    pfcp_sess_rpt_req_t *pfcp_sess_rep_req = &msg->rx_msg.pfcp_sess_rep_req;


    /* Find old transaction */
    uint32_t source_addr = peer_addr->sin_addr.s_addr;
    uint16_t source_port = peer_addr->sin_port;
    uint32_t seq_num = pfcp_sess_rep_req->header.seid_seqno.has_seid.seq_no;  
    transData_t *old_trans = (transData_t*)find_pfcp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL) {
        // drop packet 
        LOG_MSG(LOG_ERROR,"Retransmitted session report received");
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }

    /* Report is handleed for various cases..
     * case 1: Connection idle - UE DDN packet indication  
     * case 2: Connection is active. UE usage report.
     * case 3: Connection is active. Error indication is received. 
     */
    uint64_t sess_id = pfcp_sess_rep_req->header.seid_seqno.has_seid.seid;
    uint8_t ebi_index = UE_BEAR_ID(sess_id) - 5; 
    pdn_connection_t *pdn = GET_PDN(context, ebi_index);
	msg->event = PFCP_SESS_RPT_REQ_RCVD_EVNT;
    msg->ue_context = context;
    msg->pdn_context = pdn;
	if (pfcp_sess_rep_req->report_type.dldr == 1) {
        msg->proc = PAGING_PROC;
    } else {
        // usage report
        // error indication
        // UE inactivity indication 
        msg->proc = USAGE_REPORT_PROC;
        LOG_MSG(LOG_DEBUG2, "Setting data usage for subscriber %lu and data usage = %lu ", 
                context->imsi64,pfcp_sess_rep_req->usage_report[0].vol_meas.total_volume);
        set_data_stats(DATA_USAGE_SPGW_PDN, context->imsi64, pfcp_sess_rep_req->usage_report[0].vol_meas.total_volume);
    }

    /* Create new transaction */
    transData_t *trans = (transData_t *) calloc(1, sizeof(transData_t));  
    RESET_TRANS_SELF_INITIATED(trans);
    trans->sequence = seq_num;
    trans->peer_sockaddr = msg->peer_addr;
    add_pfcp_transaction(source_addr, source_port, seq_num, trans);

    proc_context_t *sess_report_proc = alloc_session_report_proc(msg);
    sess_report_proc->pfcp_trans = trans;
    trans->proc_context = (void *)sess_report_proc;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:PFCP_SESSION_REPORT_REQUEST[%u], Seid:%lu, "
			"Procedure:%s, Event:%s",
			msg->msg_type,
			msg->rx_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_event_string(msg->event));

    start_procedure(sess_report_proc);

    return 0;
}


int 
handle_session_report_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in *peer_addr = &msg->peer_addr;
 
    LOG_MSG(LOG_DEBUG2, "Received Session Report Msg from %s ", inet_ntoa(peer_addr->sin_addr));
    process_response(peer_addr->sin_addr.s_addr);

    increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ, peer_addr->sin_addr.s_addr);

    /*Decode the received msg and stored into the struct*/
    int decoded = decode_pfcp_sess_rpt_req_t((uint8_t *)pfcp_rx,
                        &msg->rx_msg.pfcp_sess_rep_req);

    if(decoded <= 0)
    {
        LOG_MSG(LOG_ERROR, "DECODED bytes in Sess Report Request is %d",
                decoded);
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }

	/* Retrive the session information based on session id. */
    ue_context_t *context = NULL;
    pfcp_sess_rpt_req_t *pfcp_sess_rep_req = &msg->rx_msg.pfcp_sess_rep_req;
	context = (ue_context_t *)get_sess_entry_seid(pfcp_sess_rep_req->header.seid_seqno.has_seid.seid);
    if(context == NULL) {
		LOG_MSG(LOG_ERROR, "Session entry not found Msg_Type:%u, Sess ID:%lu",
				msg->msg_type,
				pfcp_sess_rep_req->header.seid_seqno.has_seid.seid);
        // TODO: robustness. Generate report response with context not found 
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSREPORTREQ_REJ, peer_addr->sin_addr.s_addr);
		return -1;
	}

    msg->ue_context = context;

    int err = handle_pfcp_session_report_req_msg(msg);

    if(!err) {
      // we would free the mesage as a part of proc cleanup 
      *msg_p = NULL;
    }

    return 0;
}
