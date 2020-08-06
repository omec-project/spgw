// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "clogger.h"
#include "cp_io_poll.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"

extern udp_sock_t my_sock;


// saegw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_sess_del_resp_handler
// saegw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_pfcp_sess_del_resp_dbr_handler
// pgw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_sess_del_resp_handler
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE  PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_pfcp_sess_del_resp_dbr_handler
// sgw DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : PFCP_SESS_DEL_RESP_RCVD_EVNT 
// sgw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : process_pfcp_sess_del_resp_dbr_handler 

static
int handle_pfcp_session_delete_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_DELETION_RESPONSE);
    uint32_t seq_num = msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

	/* Retrive the session information based on session id. */
    if(pfcp_trans == NULL) {
        clLog(sxlogger, eCLSeverityCritical, "Received PFCP response and transaction not found \n");
		return -1;
    }

	/* Retrive teid from session id */
	/* stop and delete timer entry for pfcp sess del req */
	stop_transaction_timer(pfcp_trans);
    proc_context_t *proc_context = (proc_context_t *)pfcp_trans->proc_context; 
    proc_context->pfcp_trans = NULL; 
    msg->proc_context = pfcp_trans->proc_context;
    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; 

    msg->event = PFCP_SESS_DEL_RESP_RCVD_EVNT;
    proc_context->handler((void *)proc_context, msg->event, (void *)msg);

    return 0;
}


int
handle_pfcp_session_delete_response_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);

    /* Decode pfcp session delete response*/
    int decoded = decode_pfcp_sess_del_rsp_t((uint8_t*)pfcp_rx, &msg->pfcp_msg.pfcp_sess_del_resp);


    if(decoded <=0 ) {
        clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Del Resp is %d\n",
                decoded);

        update_cli_stats(peer_addr.sin_addr.s_addr,
                pfcp_rx->message_type, REJ,SX);
        return -1;
    }
    update_cli_stats(peer_addr.sin_addr.s_addr,
            pfcp_rx->message_type, ACC,SX);

    handle_pfcp_session_delete_response(msg);
    return 0;
}
