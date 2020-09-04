// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "pfcp_messages_decoder.h"
#include "clogger.h"
#include "proc_pfcp_assoc_setup.h"
#include "spgw_cpp_wrapper.h"

int 
handle_pfcp_association_setup_response_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_assn_setup_rsp_t((uint8_t *)pfcp_rx,
            &msg->pfcp_msg.pfcp_ass_resp);

    clLog(sxlogger, eCLSeverityDebug, "Decoded bytes [%d]\n", decoded);
    if(decoded <= 0) 
    {
        clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

        increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);

        return -1;
    }
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);

    handle_pfcp_association_setup_response(msg);
    return 0;
}

