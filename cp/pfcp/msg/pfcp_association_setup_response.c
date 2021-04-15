// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "pfcp_cp_interface.h"
#include "pfcp_messages_decoder.h"
#include "cp_log.h"
#include "proc_pfcp_assoc_setup.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "cp_io_poll.h"
#include "upf_struct.h"

int 
handle_pfcp_association_setup_response_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_assn_setup_rsp_t((uint8_t *)pfcp_rx,
            &msg->rx_msg.pfcp_ass_resp);

    if(decoded <= 0) 
    {
        LOG_MSG(LOG_ERROR, "Failed to process pfcp precondition check");
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);
        return -1;
    }
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP, peer_addr.sin_addr.s_addr);

    pfcp_up_func_feat_ie_t *up_feat = &msg->rx_msg.pfcp_ass_resp.up_func_feat;
    LOG_MSG(LOG_DEBUG,"supported features = %x, Additional Supported Feature1 = %x, Additional Supported Feature2 %x",
             up_feat->sup_feat, up_feat->add_sup_feat1, up_feat->add_sup_feat2);

    uint32_t seq_num = msg->rx_msg.pfcp_ass_resp.header.seid_seqno.no_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    assert(msg->msg_type == PFCP_ASSOCIATION_SETUP_RESPONSE);

    transData_t *pfcp_trans = (transData_t *)delete_pfcp_transaction(local_addr, port_num, seq_num);

    if (pfcp_trans == NULL ) {
        LOG_MSG(LOG_ERROR, "transaction not found. Dropping association response message. from UPF IP:%s",
                inet_ntoa(peer_addr.sin_addr));
        return -1;
    }
    proc_context_t *proc_context = (proc_context_t*)pfcp_trans->proc_context;
    assert(proc_context != NULL);
    proc_context->pfcp_trans = NULL;

    stop_transaction_timer(pfcp_trans);
    delayed_free(pfcp_trans);

    msg->proc_context = proc_context;
    msg->event = PFCP_ASSOCIATION_SETUP_RSP; 
    msg->proc  = proc_context->state;
    SET_PROC_MSG(proc_context, msg);


    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

    proc_context->handler(proc_context, msg);

    return 0;
}

