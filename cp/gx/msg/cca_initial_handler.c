// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "gx_interface.h"
#include "cp_log.h"
#include "spgw_config_struct.h"
#include "sm_enum.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "pfcp_cp_set_ie.h" // ajay - included for Gx context. need cleanup  
#include "pfcp.h"
#include "sm_structs_api.h"
#include "gen_utils.h"
#include "proc_pfcp_assoc_setup.h"
#include "gtpv2_error_rsp.h"
#include "spgw_cpp_wrapper.h"
#include "cp_proc.h"
#include "cp_events.h"
#include "cp_io_poll.h"
#include "cp_test.h"

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
	gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));

    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_I, saddr_in.sin_addr.s_addr);

    pdn_connection_t *pdn_cntxt = NULL;
    /* Retrive Gx_context based on Sess ID. */
    int ret;
    ue_context_t *temp_ue_context  = (ue_context_t *)get_gx_context((uint8_t*)msg->gx_msg.cca.session_id.val);
    if (temp_ue_context == NULL) {
        LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", msg->gx_msg.cca.session_id.val);
        return -1;
    }
    gx_context = (gx_context_t *)temp_ue_context->gx_context;
    proc_context_t *proc_context = (proc_context_t *)gx_context->proc_context;

    if(msg->gx_msg.cca.presence.result_code &&
            msg->gx_msg.cca.result_code != 2001){
        LOG_MSG(LOG_ERROR, "Received CCA with DIAMETER Failure [%d]", msg->gx_msg.cca.result_code);
        gx_msg_proc_failure(proc_context); 
        return -1;
    }

    /* Extract the call id from session id */
    uint32_t call_id;
    ret = retrieve_call_id((char *)msg->gx_msg.cca.session_id.val, &call_id);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s", msg->gx_msg.cca.session_id.val);
        gx_msg_proc_failure(proc_context); 
        return -1;
    }

    /* Retrieve PDN context based on call id */
    pdn_cntxt = (pdn_connection_t *)get_pdn_conn_entry(call_id);
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
            "State:%s, Event:%s",
            gx_type_str(msg->msg_type), msg->msg_type,
            msg->gx_msg.cca.session_id.val,
            get_state_string(msg->state), get_event_string(msg->event));
    SET_PROC_MSG(proc_context, msg);
    msg->proc_context = proc_context;
    proc_context->handler(proc_context, msg);
    // NOTE : this is important so that caller does not free msg pointer 
    *msg_p = NULL;
    // if we wish to generate new test events based on CCA-I then enable following code. 
    //queue_stack_unwind_event(TEST_EVENTS, (void *)pdn_cntxt, test_event_handler);
    return 0;
}
