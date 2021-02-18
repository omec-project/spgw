// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtpv2_interface.h"
#include "ue.h"
#include "trans_struct.h"
#include "sm_struct.h"
#include "cp_io_poll.h"
#include "gtp_messages_decoder.h"
#include "spgw_cpp_wrapper.h"
#include "cp_log.h"
#include "gen_utils.h"
#include "util.h"
#include "sm_enum.h"
#include "cp_transactions.h"
#include "sm_structs_api.h"
#include "gtpv2_session.h"
#include "byteswap.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "cp_peer.h"

// saegw DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_create_bearer_resp_handler
// pgw - DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_cbresp_handler
// sgw  DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT ==> process_create_bearer_resp_handler 

int handle_create_bearer_response_msg(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    ue_context_t *context = NULL;
    int ret = 0;
    struct sockaddr_in *peer_addr = &msg->peer_addr;

    create_bearer_rsp_t *cbrsp = &msg->gtpc_msg.cb_rsp;

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    if((ret = decode_create_bearer_rsp((uint8_t *) gtpv2c_rx,
                    cbrsp) == 0)) {
        LOG_MSG(LOG_DEBUG0, "Received CBRsp - message decode failed ");
        return -1;
    }

    if (gtpv2c_rx->teid.has_teid.teid) { 
        context = (ue_context_t*)get_ue_context(cbrsp->header.teid.has_teid.teid);
        if(context != NULL) { 
            LOG_MSG(LOG_DEBUG0, "No matching user session found. Dropping CBRsp message for teid = %u", gtpv2c_rx->teid.has_teid.teid);
            increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK_DROP, peer_addr->sin_addr.s_addr);
            return -1;
        }
    }
    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);

    proc_context_t *proc_context = gtpc_trans->proc_context;

    if(context == NULL) {
        context = proc_context->ue_context;
        LOG_MSG(LOG_DEBUG, "Context not found from teid. Found context using procedure. IMSI %lu ", context->imsi64);
    } else {
        assert(proc_context->ue_context == context);
    }
    LOG_MSG(LOG_DEBUG2, "Received CBRsp for user - %lu", context->imsi64);
    msg->proc_context = gtpc_trans->proc_context;
	msg->event = CREATE_BER_RESP_RCVD_EVNT;
    msg->proc  = proc_context->state;
    SET_PROC_MSG(proc_context, msg);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

    proc_context->handler(proc_context, msg);
    return 0;
}
