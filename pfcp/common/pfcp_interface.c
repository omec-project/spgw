// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "pfcp_messages_decoder.h"
#include "cp_config.h"


extern cp_config_t *cp_config;
pfcp_handler pfcp_msg_handler[256];

void init_pfcp_interface(void)
{
    for(int i=0;i<256;i++)
        pfcp_msg_handler[i] = handle_unknown_pfcp_msg;

    pfcp_msg_handler[PFCP_HEARTBEAT_REQUEST] =  handle_pfcp_heartbit_req_msg;
    pfcp_msg_handler[PFCP_HEARTBEAT_RESPONSE] =  handle_pfcp_heartbit_rsp_msg;
    pfcp_msg_handler[PFCP_ASSOCIATION_SETUP_REQUEST] =  handle_pfcp_association_setup_request_msg;
    pfcp_msg_handler[PFCP_ASSOCIATION_SETUP_RESPONSE] =  handle_pfcp_association_setup_response_msg;
    pfcp_msg_handler[PFCP_SESSION_ESTABLISHMENT_RESPONSE] =  handle_pfcp_session_est_response_msg;
    pfcp_msg_handler[PFCP_SESSION_MODIFICATION_RESPONSE] =  handle_pfcp_session_mod_response_msg;
    pfcp_msg_handler[PFCP_SESSION_DELETION_RESPONSE] = handle_pfcp_session_delete_response_msg; 
    pfcp_msg_handler[PFCP_SESSION_REPORT_REQUEST] = handle_session_report_msg;
    pfcp_msg_handler[PFCP_PFD_MANAGEMENT_RESPONSE] = handle_pfcp_pfd_management_response_msg;
    pfcp_msg_handler[PFCP_SESSION_SET_DELETION_REQUEST] = handle_pfcp_session_delete_request_msg; 
    pfcp_msg_handler[PFCP_SESSION_SET_DELETION_RESPONSE] = handle_pfcp_set_deletion_response_msg; 
    return;
}

int 
handle_unknown_pfcp_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    RTE_SET_USED(msg);
    clLog(clSystemLog, eCLSeverityCritical, "%s::process_msgs-"
            "\n\tcase: spgw_cfg= %d;"
            "\n\tReceived unprocessed PFCP Message_Type:%u"
            "... Discarding\n", __func__, cp_config->cp_type, pfcp_rx->message_type);

    return -1;
}

