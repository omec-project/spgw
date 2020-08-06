// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "cp_peer.h"
#include "clogger.h"
#include "pfcp_messages_decoder.h"



// saegw - INITIAL_PDN_ATTACH_PROC,PFCP_PFD_MGMT_RESP_RCVD_STATE, PFCP_PFD_MGMT_RESP_RCVD_EVNT, => pfd_management_handler
// pgw - INITIAL_PDN_ATTACH_PROC PFCP_PFD_MGMT_RESP_RCVD_STATE PFCP_PFD_MGMT_RESP_RCVD_EVNT ==> pfd_management_handler
// sgw - INITIAL_PDN_ATTACH_PROC PFCP_PFD_MGMT_RESP_RCVD_STATE PFCP_PFD_MGMT_RESP_RCVD_EVNT - pfd_management_handler 
static
int handle_pfcp_pfd_management_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_PFD_MANAGEMENT_RESPONSE);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* check cause ie */
	if(msg->pfcp_msg.pfcp_pfd_resp.cause.cause_value !=  REQUESTACCEPTED){
		clLog(clSystemLog, eCLSeverityCritical, "%s:  Msg_Type:%u, Cause value:%d, offending ie:%u\n",
					__func__, msg->msg_type, msg->pfcp_msg.pfcp_pfd_resp.cause.cause_value,
			    msg->pfcp_msg.pfcp_pfd_resp.offending_ie.type_of_the_offending_ie);
		return -1;
	}

	msg->state = PFCP_PFD_MGMT_RESP_RCVD_STATE;
	msg->event = PFCP_PFD_MGMT_RESP_RCVD_EVNT;
	msg->proc = INITIAL_PDN_ATTACH_PROC;

    /* For time being just getting rid of 3d FSM array */
    pfd_management_handler((void *)msg, NULL);
    return 0;
}

int 
handle_pfcp_pfd_management_response_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    process_response(peer_addr.sin_addr.s_addr);
    /* Decode pfd mgmt response */
    int decoded = decode_pfcp_pfd_mgmt_rsp_t((uint8_t *)pfcp_rx, &msg->pfcp_msg.pfcp_pfd_resp);
    if(decoded <= 0) 
    {
        clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Pfd Mgmt Resp is %d\n",
                decoded);
        update_cli_stats(peer_addr.sin_addr.s_addr,
                pfcp_rx->message_type, REJ,SX);
        return -1;
    }
    update_cli_stats(peer_addr.sin_addr.s_addr,
            pfcp_rx->message_type, ACC,SX);

    handle_pfcp_pfd_management_response(msg);
    return 0;
}

