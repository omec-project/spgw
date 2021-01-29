// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "sm_structs_api.h"


// saegw - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_RCVD_STATE PFCP_SESS_SET_DEL_REQ_RCVD_EVNT => process_pfcp_sess_set_del_req 
static
int handle_pfcp_session_set_delete_request(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_SET_DELETION_REQUEST);
	msg->state = PFCP_SESS_SET_DEL_REQ_RCVD_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;

	/*Set the appropriate event type.*/
	msg->event = PFCP_SESS_SET_DEL_REQ_RCVD_EVNT;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			" Msg_Type: PFCP_SESSION_SET_DELETION_RESPONSE[%u], "
			"Procedure:%s, State:%s, Event:%s\n",
			msg->msg_type,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));
    return 0;
}

int 
handle_pfcp_session_delete_request_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_sess_set_del_req_t((uint8_t *)pfcp_rx,
            &msg->pfcp_msg.pfcp_sess_set_del_req);

    if(decoded <=0 ) 
    {
        LOG_MSG(LOG_DEBUG, "DEOCED bytes in Sess Set Deletion Request is %d", decoded);
        return -1;
    }

    int err = handle_pfcp_session_set_delete_request(msg);

    if(!err) {
      // we would free the mesage as a part of proc cleanup 
      *msg_p = NULL;
    }

    return 0;
}
