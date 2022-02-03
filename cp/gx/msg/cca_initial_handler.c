// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0


#include "gx_interface.h"
#include "cp_log.h"
#include "spgw_config_struct.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "pfcp_cp_set_ie.h" // ajay - included for Gx context. need cleanup  
#include "pfcp.h"
#include "sm_structs_api.h"
#include "gen_utils.h"
#include "proc_pfcp_assoc_setup.h"
#include "gtpv2_error_rsp.h"
#include "spgw_cpp_wrapper.h"
#include "proc_struct.h"
#include "cp_events.h"
#include "cp_io_poll.h"
#include "cp_test.h"
#include "proc.h"
#include "cp_transactions.h"

void gx_msg_proc_failure(proc_context_t *proc_ctxt)
{
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    msg->msg_type = GX_CCAI_FAILED;
    msg->event = GX_CCAI_FAILED;
    msg->proc_context = proc_ctxt; 
    SET_PROC_MSG(proc_ctxt, msg);
    proc_ctxt->handler(proc_ctxt, msg);
    return;
}

int handle_cca_initial_msg(msg_info_t **msg_p)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));

    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_I, saddr_in.sin_addr.s_addr);

    /* Retrive Gx_context based on Sess ID. */
    int ret;
    ue_context_t *ue_context  = (ue_context_t *)get_ue_context_from_gxsessid((uint8_t*)msg->rx_msg.cca.session_id.val);
    if (ue_context == NULL) {
        LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", msg->rx_msg.cca.session_id.val);
        return -1;
    }

    proc_context_t *proc_context = get_first_procedure(ue_context);
    transData_t *gx_trans = proc_context->gx_trans;

    /* Retrive the session information based on session id. */
    if(gx_trans == NULL) {
        LOG_MSG(LOG_ERROR, "Received GX response and transaction not found ");
        return -1;
    }
    proc_context->gx_trans = NULL;
    stop_transaction_timer(gx_trans);
    delayed_free(gx_trans);

    if(msg->rx_msg.cca.presence.result_code &&
            msg->rx_msg.cca.result_code != 2001){
        LOG_MSG(LOG_ERROR, "Received CCA with DIAMETER Failure [%d]", msg->rx_msg.cca.result_code);
        gx_msg_proc_failure(proc_context); 
        return -1;
    }

    /* Extract the call id from session id */
    uint32_t call_id;
    ret = retrieve_call_id((char *)msg->rx_msg.cca.session_id.val, &call_id);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s", msg->rx_msg.cca.session_id.val);
        gx_msg_proc_failure(proc_context); 
        return -1;
    }

    /* Retrieve PDN context based on call id */
    pdn_connection_t *pdn_cntxt = (pdn_connection_t *)get_pdn_conn_entry(call_id);
    if (pdn_cntxt == NULL) {
        LOG_MSG(LOG_ERROR, "No valid pdn cntxt found for CALL_ID:%u", call_id);
        gx_msg_proc_failure(proc_context); 
        return -1;
    }

    msg->pdn_context = pdn_cntxt;
    msg->ue_context = pdn_cntxt->context;
    msg->event = CCA_RCVD_EVNT;
    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:%s[%u], Session Id:%s, "
            "Event:%s",
            gx_type_str(msg->msg_type), msg->msg_type,
            msg->rx_msg.cca.session_id.val,
            get_event_string(msg->event));
    SET_PROC_MSG(proc_context, msg);
    msg->proc_context = proc_context;
    proc_context->handler(proc_context, msg);

    // NOTE : this is important so that caller does not free msg pointer 
    *msg_p = NULL;
    // if we wish to generate new test events based on CCA-I then enable following code. 
    //queue_stack_unwind_event(TEST_EVENTS, (void *)pdn_cntxt, test_event_handler);
    return 0;
}
