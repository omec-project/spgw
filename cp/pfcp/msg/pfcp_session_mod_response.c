// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "cp_log.h"
#include "cp_io_poll.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "proc_struct.h"

// saegw, INITIAL_PDN_ATTACH_PROC,PFCP_SESS_MOD_REQ_SNT_STATE,PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw SGW_RELOCATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw CONN_SUSPEND_PROC PFCP_SESS_MOD_REQ_SNT_STATE -PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_cbr_handler process_pfcp_sess_mod_resp_pre_cbr_handler
// saegw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_dbr_handler
// saegw MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => del_bearer_cmd_mbr_resp_handler
// saegw UPDATE_BEARER_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_ubr_handler

// pgw - SGW_RELOCATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT ==> process_sess_mod_resp_sgw_reloc_handler
// pgw - DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT process_pfcp_sess_mod_resp_cbr_handler process_pfcp_sess_mod_resp_pre_cbr_handler 
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT - process_pfcp_sess_mod_resp_dbr_handler 
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT ==> del_bearer_cmd_mbr_resp_handler
// pgw - UPDATE_BEARER_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT process_pfcp_sess_mod_resp_ubr_handler 
// sgw INITIAL_PDN_ATTACH_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_REQ_SNT_STATE ==> process_sess_mod_resp_handler
// sgw SGW_RELOCATION_PROC CS_RESP_RCVD_STATE CS_RESP_RCVD_STATE process_sess_mod_resp_handler 
// sgw CONN_SUSPEND_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT --> process_sess_mod_resp_handler
// sgw DETACH_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT --> process_mod_resp_delete_handler 
// sgw DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT : process_pfcp_sess_mod_resp_cbr_handler process_pfcp_sess_mod_resp_pre_cbr_handler 
// sgw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT : process_pfcp_sess_mod_resp_dbr_handler
// sgw MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC - PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT - process_pfcp_sess_mod_resp_dbr_handler 
static
int handle_pfcp_session_modification_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_MODIFICATION_RESPONSE);
    uint32_t seq_num = msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = (transData_t *) delete_pfcp_transaction(local_addr, port_num, seq_num);

    if(pfcp_trans == NULL) {
        LOG_MSG(LOG_ERROR, "Received Modify response and transaction not found ");
        return -1;
    }
    LOG_MSG(LOG_DEBUG, "Received Modify response and transaction found ");
    proc_context_t *proc_context = (proc_context_t *)pfcp_trans->proc_context;
    proc_context->pfcp_trans = NULL;

	/* stop and delete timer entry for pfcp mod req */
	stop_transaction_timer(pfcp_trans);
    delayed_free(pfcp_trans);

    msg->proc_context = proc_context;
    msg->event = PFCP_SESS_MOD_RESP_RCVD_EVNT;
    SET_PROC_MSG(proc_context, msg);

    proc_context->handler((void*)proc_context, msg);
    return 0;
}


int
handle_pfcp_session_mod_response_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_sess_mod_rsp_t((uint8_t *)pfcp_rx,
            &msg->rx_msg.pfcp_sess_mod_resp);

    if(decoded <= 0)
    {
        LOG_MSG(LOG_DEBUG, "DECODED bytes in Sess Modify Resp is %d", decoded);
        // TODOSTATISTICS
        // increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSMODRSP_REJ, peer_addr.sin_addr.s_addr);
        return -1;
    }

    increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSMODRSP, peer_addr.sin_addr.s_addr);

    int err = handle_pfcp_session_modification_response(msg);

    if(!err) {
      // we would free the mesage as a part of proc cleanup 
      *msg_p = NULL;
    }

    return 0;
}

