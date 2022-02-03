// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include "pfcp_cp_interface.h"
#include "cp_peer.h"
#include "cp_log.h"
#include "pfcp_messages_decoder.h"
#include "spgw_cpp_wrapper.h"
#include "proc.h"



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
	if(msg->rx_msg.pfcp_pfd_resp.cause.cause_value !=  REQUESTACCEPTED){
		LOG_MSG(LOG_ERROR, "Msg_Type:%u, Cause value:%d, offending ie:%u",
					msg->msg_type, msg->rx_msg.pfcp_pfd_resp.cause.cause_value,
			    msg->rx_msg.pfcp_pfd_resp.offending_ie.type_of_the_offending_ie);
		return -1;
	}

	//msg->state = PFCP_PFD_MGMT_RESP_RCVD_STATE;
	msg->event = PFCP_PFD_MGMT_RESP_RCVD_EVNT;
	msg->proc = INITIAL_PDN_ATTACH_PROC;

    /* For time being just getting rid of 3d FSM array */
    pfd_management_handler((void *)msg, NULL);
    return 0;
}

int 
handle_pfcp_pfd_management_response_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;
 
    process_response(peer_addr.sin_addr.s_addr);
    /* Decode pfd mgmt response */
    int decoded = decode_pfcp_pfd_mgmt_rsp_t((uint8_t *)pfcp_rx, &msg->rx_msg.pfcp_pfd_resp);
    if(decoded <= 0) 
    {
        LOG_MSG(LOG_DEBUG, "DEOCED bytes in Pfd Mgmt Resp is %d", decoded);
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_PFDMGMTRSP_DROP, peer_addr.sin_addr.s_addr); 
        return -1;
    }
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_PFDMGMTRSP, peer_addr.sin_addr.s_addr); 
    int err = handle_pfcp_pfd_management_response(msg);

    if(!err) {
      // we would free the mesage as a part of proc cleanup 
      *msg_p = NULL;
    }

    return 0;
}

