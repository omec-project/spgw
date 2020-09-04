// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"

int 
handle_pfcp_set_deletion_response_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_sess_set_del_rsp_t((uint8_t*)pfcp_rx,
            &msg->pfcp_msg.pfcp_sess_set_del_rsp);


    if(decoded <= 0)
    {
        clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Set Deletion Resp is %d\n",
                decoded);
//        increment_userplane_stats(MSG_RX_PFCP_SXASXB_SET_DELETE_RSP_DECODE_ERR, peer_addr.sin_addr.s_addr);
        return -1;
    }
//        increment_userplane_stats(MSG_RX_PFCP_SXASXB_SET_DELETE_RSP, peer_addr.sin_addr.s_addr);

    return 0;
}
