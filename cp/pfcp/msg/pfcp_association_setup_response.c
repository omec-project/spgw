// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "pfcp_cp_interface.h"
#include "pfcp_messages_decoder.h"
#include "cp_log.h"
#include "proc_pfcp_assoc_setup.h"
#include "spgw_cpp_wrapper.h"

int 
handle_pfcp_association_setup_response_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_assn_setup_rsp_t((uint8_t *)pfcp_rx,
            &msg->pfcp_msg.pfcp_ass_resp);

    if(decoded <= 0) 
    {
        LOG_MSG(LOG_ERROR, "Failed to process pfcp precondition check");
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);
        return -1;
    }
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);

    pfcp_up_func_feat_ie_t *up_feat = &msg->pfcp_msg.pfcp_ass_resp.up_func_feat;
    LOG_MSG(LOG_DEBUG,"supported features = %x, Additional Supported Feature1 = %x, Additional Supported Feature2 %x",
             up_feat->sup_feat, up_feat->add_sup_feat1, up_feat->add_sup_feat2);
    //Dont hold on to any message reference inside this function call...
    handle_pfcp_association_setup_response(msg);
    // caller would free allocated message  
    return 0;
}

