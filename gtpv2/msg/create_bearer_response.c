// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#include "gtpv2_interface.h"
#include "ue.h"
#include "rte_common.h"
#include "trans_struct.h"
#include "sm_struct.h"
#include "cp_io_poll.h"
#include "gtp_messages_decoder.h"
#include "spgw_cpp_wrapper.h"
#include "clogger.h"
#include "gen_utils.h"
#include "util.h"
#include "sm_enum.h"
#include "cp_transactions.h"
#include "gw_adapter.h"
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


    printf("\n Received CBRsp\n ");
    if((ret = decode_create_bearer_rsp((uint8_t *) gtpv2c_rx,
                    cbrsp) == 0)) {
        printf("\n Received CBRsp - decode failed \n ");
        return -1;
    }

    if (gtpv2c_rx->teid.has_teid.teid && get_ue_context(cbrsp->header.teid.has_teid.teid, &context) != 0) {
        printf("No matching user session found. Dropping CBRsp message \n");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }
 
    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);

    proc_context_t *proc_context = gtpc_trans->proc_context;

    if(context == NULL) {
        printf("Context not found...\n");
        context = proc_context->ue_context;
    } else {
        assert(proc_context->ue_context == context);
    }

    msg->proc_context = gtpc_trans->proc_context;
	msg->event = CREATE_BER_RESP_RCVD_EVNT;
    msg->proc  = proc_context->state;
    SET_PROC_MSG(proc_context, msg);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));

    proc_context->handler(proc_context, msg);
    return 0;
}
