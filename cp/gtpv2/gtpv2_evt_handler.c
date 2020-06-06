/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config_new.h"
#include "sm_enum.h"
#include "gtpv2_evt_handler.h"
#include "assert.h"

int handle_create_session_request_msg(msg_info *msg)
{
    assert(msg->msg_type == GTP_CREATE_SESSION_REQ);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
    msg->proc = INITIAL_PDN_ATTACH_PROC;
    /* For time being just getting rid of 3d FSM array */
    association_setup_handler((void *)msg, NULL);
    return 0;
}

int handle_modify_bearer_request_msg(msg_info *msg)
{
    assert(msg->msg_type == GTP_MODIFY_BEARER_REQ);
    msg->proc = INITIAL_PDN_ATTACH_PROC;
    process_mb_req_handler((void *)msg, NULL);
    return 0;
}

int handle_delete_session_request_msg(msg_info *msg)
{
    assert(msg->msg_type == GTP_DELETE_SESSION_REQ);
    return 0;
}
