// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "clogger.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "cp_io_poll.h"
extern udp_sock_t my_sock;

// SAEGW - INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_REQ_SNT_STATE, PFCP_SESS_EST_RESP_RCVD_EVNT => process_sess_est_resp_handler
// saegw - SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
// pgw - INITIAL_PDN_ATTACH_PROC INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_handler
// pgw SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
// sgw - INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_handler
// sgw - SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
static
int handle_pfcp_session_est_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_ESTABLISHMENT_RESPONSE);
    uint32_t seq_num = msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

	/* Retrive the session information based on session id. */
    if(pfcp_trans == NULL) {
        clLog(sxlogger, eCLSeverityCritical, "Received PFCP response and transaction not found \n");
		return -1;
    }
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive teid from session id */
	/* stop and delete the timer session for pfcp  est. req. */
	stop_transaction_timer(pfcp_trans);

    proc_context_t *proc_context = (proc_context_t *)pfcp_trans->proc_context; 
    proc_context->pfcp_trans = NULL; 
    msg->proc_context = pfcp_trans->proc_context;
    free(pfcp_trans); /* EST Response */

    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; 
    assert(msg->ue_context != NULL);
    assert(msg->pdn_context != NULL);
    msg->event = PFCP_SESS_EST_RESP_RCVD_EVNT;
    proc_context->handler((void*)proc_context, msg->event, (void *)msg);
    return 0;
}

int 
handle_pfcp_session_est_response_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_sess_estab_rsp_t((uint8_t*)pfcp_rx,
            &msg->pfcp_msg.pfcp_sess_est_resp);
    clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Estab Resp is %d\n",
            decoded);

    if(decoded <= 0) 
    {
        clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

        update_cli_stats(peer_addr.sin_addr.s_addr,
                pfcp_rx->message_type, REJ,SX);

        return -1;
    }
    update_cli_stats(peer_addr.sin_addr.s_addr,
            pfcp_rx->message_type, ACC,SX);

    handle_pfcp_session_est_response(msg);
    return 0;
}


