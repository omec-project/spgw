// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <byteswap.h>
#include "pfcp_cp_util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "gw_adapter.h"
#include "clogger.h"
#include "pfcp_cp_interface.h"
#include "cp_peer.h"
#include "sm_structs_api.h"
#include "pfcp.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "cp_config.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config_defs.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"

extern udp_sock_t my_sock;
extern struct rte_hash *heartbeat_recovery_hash;
uint8_t pfcp_rx[1024]; /* TODO: Decide size */



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

// SAEGW - INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_REQ_SNT_STATE, PFCP_SESS_EST_RESP_RCVD_EVNT => process_sess_est_resp_handler
// saegw - SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
// pgw - INITIAL_PDN_ATTACH_PROC INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_handler
// pgw SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
// sgw - INITIAL_PDN_ATTACH_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_handler
// sgw - SGW_RELOCATION_PROC PFCP_SESS_EST_REQ_SNT_STATE PFCP_SESS_EST_RESP_RCVD_EVNT ==> process_sess_est_resp_sgw_reloc_handler
static
int handle_pfcp_session_est_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_ESTABLISHMENT_RESPONSE);
    uint32_t seq_num = msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

	/* Retrive the session information based on session id. */
    if(pfcp_trans == NULL) {
        clLog(sxlogger, eCLSeverityCritical, "Received PFCP response and transaction not found \n");
		return -1;
    }
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive teid from session id */
	/* stop and delete the timer session for pfcp  est. req. */
	stop_transaction_timer(pfcp_trans);

    proc_context_t *proc_context = (proc_context_t *)pfcp_trans->proc_context; 
    proc_context->pfcp_trans = NULL; 
    msg->proc_context = pfcp_trans->proc_context;
    free(pfcp_trans); /* EST Response */

    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; 
    assert(msg->ue_context != NULL);
    assert(msg->pdn_context != NULL);
    msg->event = PFCP_SESS_EST_RESP_RCVD_EVNT;
    proc_context->handler((void*)proc_context, msg->event, (void *)msg);
    return 0;
}

// saegw, INITIAL_PDN_ATTACH_PROC,PFCP_SESS_MOD_REQ_SNT_STATE,PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw SGW_RELOCATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw CONN_SUSPEND_PROC PFCP_SESS_MOD_REQ_SNT_STATE -PFCP_SESS_MOD_RESP_RCVD_EVNT => process_sess_mod_resp_handler
// saegw DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_cbr_handler
// saegw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_dbr_handler
// saegw MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => del_bearer_cmd_mbr_resp_handler
// saegw UPDATE_BEARER_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT => process_pfcp_sess_mod_resp_ubr_handler

// pgw - SGW_RELOCATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT ==> process_sess_mod_resp_sgw_reloc_handler
// pgw - DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT process_pfcp_sess_mod_resp_cbr_handler 
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT - process_pfcp_sess_mod_resp_dbr_handler 
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT ==> del_bearer_cmd_mbr_resp_handler
// pgw - UPDATE_BEARER_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT process_pfcp_sess_mod_resp_ubr_handler 
// sgw INITIAL_PDN_ATTACH_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_REQ_SNT_STATE ==> process_sess_mod_resp_handler
// sgw SGW_RELOCATION_PROC CS_RESP_RCVD_STATE CS_RESP_RCVD_STATE process_sess_mod_resp_handler 
// sgw CONN_SUSPEND_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT --> process_sess_mod_resp_handler
// sgw DETACH_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT --> process_mod_resp_delete_handler 
// sgw DED_BER_ACTIVATION_PROC PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT : process_pfcp_sess_mod_resp_cbr_handler 
// sgw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT : process_pfcp_sess_mod_resp_dbr_handler
// sgw MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC - PFCP_SESS_MOD_REQ_SNT_STATE PFCP_SESS_MOD_RESP_RCVD_EVNT - process_pfcp_sess_mod_resp_dbr_handler 
static
int handle_pfcp_session_modification_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_MODIFICATION_RESPONSE);
    uint32_t seq_num = msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

    if(pfcp_trans == NULL) {
        clLog(sxlogger, eCLSeverityCritical, "Received Modify response and transaction not found \n");
        return -1;
    }
    clLog(sxlogger, eCLSeverityDebug, "Received Modify response and transaction found \n");
	/* Retrive teid from session id */
	/* stop and delete timer entry for pfcp mod req */
	stop_transaction_timer(pfcp_trans);
    proc_context_t *proc_context = pfcp_trans->proc_context;
    free(pfcp_trans);
    proc_context->pfcp_trans = NULL;

    msg->proc_context = proc_context;
    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; /* can be null in case of rab release */ 
    assert(msg->ue_context != NULL);
    msg->event = PFCP_SESS_MOD_RESP_RCVD_EVNT;

    proc_context->handler((void*)proc_context, msg->event, (void *)msg);
    return 0;
}

// saegw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_sess_del_resp_handler
// saegw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_pfcp_sess_del_resp_dbr_handler
// pgw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_sess_del_resp_handler
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE  PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_pfcp_sess_del_resp_dbr_handler
// sgw DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : PFCP_SESS_DEL_RESP_RCVD_EVNT 
// sgw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : process_pfcp_sess_del_resp_dbr_handler 

static
int handle_pfcp_session_delete_response(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_DELETION_RESPONSE);
    uint32_t seq_num = msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seq_no; 
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);

	/* Retrive the session information based on session id. */
    if(pfcp_trans == NULL) {
        clLog(sxlogger, eCLSeverityCritical, "Received PFCP response and transaction not found \n");
		return -1;
    }

	/* Retrive teid from session id */
	/* stop and delete timer entry for pfcp sess del req */
	stop_transaction_timer(pfcp_trans);
    proc_context_t *proc_context = (proc_context_t *)pfcp_trans->proc_context; 
    proc_context->pfcp_trans = NULL; 
    msg->proc_context = pfcp_trans->proc_context;
    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; 

    msg->event = PFCP_SESS_DEL_RESP_RCVD_EVNT;
    proc_context->handler((void *)proc_context, msg->event, (void *)msg);

    return 0;
}

// saegw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
// saegw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
static
int handle_pfcp_session_report_req_msg(msg_info_t *msg)
{
    ue_context_t *context = NULL;
    assert(msg->msg_type == PFCP_SESSION_REPORT_REQUEST);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
				&context) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, Sess ID:%lu\n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid);
		return -1;
	}

    /* Report is handleed for various cases..
     * case 1: Connection idle - UE DDN packet indication  
     * case 2: Connection is active. UE usage report.
     * case 3: Connection is active. Error indication is received. 
     */
	msg->event = PFCP_SESS_RPT_REQ_RCVD_EVNT;
    /* For time being just getting rid of 3d FSM array */
	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_REPORT_REQUEST[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

    process_rpt_req_handler((void *)msg, NULL);
    return 0;
}

// saegw - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_RCVD_STATE PFCP_SESS_SET_DEL_REQ_RCVD_EVNT => process_pfcp_sess_set_del_req 
static
int handle_pfcp_session_set_delete_request(msg_info_t *msg)
{
    assert(msg->msg_type == PFCP_SESSION_SET_DELETION_REQUEST);
	msg->state = PFCP_SESS_SET_DEL_REQ_RCVD_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;

	/*Set the appropriate event type.*/
	msg->event = PFCP_SESS_SET_DEL_REQ_RCVD_EVNT;

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			" Msg_Type: PFCP_SESSION_SET_DELETION_RESPONSE[%u], "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));
    return 0;
}
 
static 
void handle_pfcp_message(msg_info_t *msg)
{
    /* fetch user context */
    // find session context  
    //      non-zero teid - find context from teid 
    //      zero teid
    //          Request     : IMSI from the message
    //          Response    : transactions
    // Once we have found/not-found context then get procedure...based on UE context and message content  

    //get_user_context_gtp_msg(msg);

    switch(msg->msg_type)
    {
        case PFCP_ASSOCIATION_SETUP_RESPONSE:
        {
            handle_pfcp_association_setup_response(msg);
            break;
        }
        case PFCP_PFD_MANAGEMENT_RESPONSE:
        {
            handle_pfcp_pfd_management_response(msg);
            break;
        }
        case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
        {
            handle_pfcp_session_est_response(msg);
            break;
        }
        case PFCP_SESSION_MODIFICATION_RESPONSE:
        {
            handle_pfcp_session_modification_response(msg);
            break;
        }
        case PFCP_SESSION_DELETION_RESPONSE:
        {
            handle_pfcp_session_delete_response(msg);
            break;
        }
        case PFCP_SESSION_REPORT_REQUEST:
        {
            handle_pfcp_session_report_req_msg(msg);
            break;
        }
        case PFCP_SESSION_SET_DELETION_REQUEST:
        {
            handle_pfcp_session_set_delete_request(msg);
            break;
        }
        default:
        {
            printf("Unhandled message %d \n", msg->msg_type);
            break;
        }
    }
    return;
}

/**
 * @brief  : Process incoming heartbeat request and send response
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
process_heartbeat_request(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{
	int encoded = 0;
	int decoded = 0;
	uint8_t pfcp_msg[1024]= {0};

	RTE_SET_USED(decoded);

	memset(pfcp_msg, 0, 1024);
	pfcp_hrtbeat_req_t *pfcp_heartbeat_req = malloc(sizeof(pfcp_hrtbeat_req_t));
	pfcp_hrtbeat_rsp_t  pfcp_heartbeat_resp = {0};
	decoded = decode_pfcp_hrtbeat_req_t(buf_rx, pfcp_heartbeat_req);
	fill_pfcp_heartbeat_resp(&pfcp_heartbeat_resp);
	pfcp_heartbeat_resp.header.seid_seqno.no_seid.seq_no = pfcp_heartbeat_req->header.seid_seqno.no_seid.seq_no;

	encoded = encode_pfcp_hrtbeat_rsp_t(&pfcp_heartbeat_resp,  pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	/* Reset the periodic timers */
	process_response((uint32_t)peer_addr->sin_addr.s_addr);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, peer_addr) < 0 ) {
		clLog(clSystemLog, eCLSeverityDebug, "Error sending in heartbeat request: %i\n",errno);
	} else {
		update_cli_stats(peer_addr->sin_addr.s_addr,
						PFCP_HEARTBEAT_RESPONSE,SENT,SX);
	}
	free(pfcp_heartbeat_req);
	return 0;
}

/**
 * @brief  : Process hearbeat response message
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
process_heartbeat_response(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{

	process_response((uint32_t)peer_addr->sin_addr.s_addr);

	pfcp_hrtbeat_rsp_t pfcp_hearbeat_resp = {0};
	decode_pfcp_hrtbeat_rsp_t(buf_rx, &pfcp_hearbeat_resp);
	uint32_t *recov_time ;

	int ret = rte_hash_lookup_data(heartbeat_recovery_hash , &peer_addr->sin_addr.s_addr ,
			(void **) &(recov_time));

	if (ret == -ENOENT) {
		clLog(clSystemLog, eCLSeverityDebug, "No entry found for the heartbeat!!\n");

	} else {
		/*TODO: Restoration part to be added if recovery time is found greater*/
		uint32_t update_recov_time = 0;
		update_recov_time =  (pfcp_hearbeat_resp.rcvry_time_stmp.rcvry_time_stmp_val);

		if(update_recov_time > *recov_time) {

			ret = rte_hash_add_key_data (heartbeat_recovery_hash,
					&peer_addr->sin_addr.s_addr, &update_recov_time);

			ret = rte_hash_lookup_data(heartbeat_recovery_hash , &peer_addr->sin_addr.s_addr,
					(void **) &(recov_time));
		}
	}

	return 0;
}


/* TODO: Parse byte_rx to msg_handler_sx_n4 */
int
msg_handler_sx_n4(void)
{
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int ret = 0, bytes_pfcp_rx = 0;
	msg_info_t msg = {0};
	struct sockaddr_in peer_addr = {0};

	bytes_pfcp_rx = recvfrom(my_sock.sock_fd_pfcp, pfcp_rx, 512, MSG_DONTWAIT,
			(struct sockaddr *)(&peer_addr), &addr_len);

    if ((bytes_pfcp_rx < 0) &&
            (errno == EAGAIN  || errno == EWOULDBLOCK)) {
        return -1; // Read complete data 
    }

    if (bytes_pfcp_rx == 0) {
        clLog(clSystemLog, eCLSeverityCritical, "SGWC|SAEGWC_s11 recvfrom error: %s",
                strerror(errno));
        return -1;
    }

	pfcp_header_t *pfcp_header = (pfcp_header_t *) pfcp_rx;
	if(pfcp_header->message_type == PFCP_HEARTBEAT_REQUEST) {

		printf("Heartbit request received from UP %s \n",inet_ntoa(peer_addr.sin_addr));
		update_cli_stats(peer_addr.sin_addr.s_addr,
				pfcp_header->message_type,RCVD,SX);

		ret = process_heartbeat_request(pfcp_rx, &peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat request\n", __func__);
		}
		return 0;
	}else if(pfcp_header->message_type == PFCP_HEARTBEAT_RESPONSE) {
		printf("Heartbit response received from UP %s \n",inet_ntoa(peer_addr.sin_addr));
		ret = process_heartbeat_response(pfcp_rx, &peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat response\n", __func__);
		} else {
			update_cli_stats(peer_addr.sin_addr.s_addr,
					PFCP_HEARTBEAT_RESPONSE,RCVD,SX);
		}
		return 0;
	} else {
		// Requirement - cleanup - why this is called as response ? it could be request mesage as well 
		printf("PFCP message %d  received from UP %s \n",pfcp_header->message_type, inet_ntoa(peer_addr.sin_addr));
		/*Reset periodic timers*/
        if(pfcp_header->message_type != PFCP_ASSOCIATION_SETUP_RESPONSE) 
		    process_response(peer_addr.sin_addr.s_addr);

        msg.peer_addr = peer_addr;

        // TODO - PORT peer address should be copied to msg 
		if ((ret = pfcp_pcnd_check(pfcp_rx, &msg, bytes_pfcp_rx)) != 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

			update_cli_stats(peer_addr.sin_addr.s_addr,
							pfcp_header->message_type, REJ,SX);
			return 0;
		}

        // validate message content - validate the presence of IEs
        ret = validate_pfcp_message_content(&msg);
        if(ret != 0) 
        {
            // validatation failed;
            printf("PFCP message validation failed \n");
            return 0;
        }

        /* Event figured out from msg. 
         * Proc  found from user context and message content, message type 
         * State is on the pdn connection (for now)..Need some more attention here later   
         */
		if(pfcp_header->message_type == PFCP_SESSION_REPORT_REQUEST)
			update_cli_stats(peer_addr.sin_addr.s_addr,
							pfcp_header->message_type, RCVD,SX);
		else
			update_cli_stats(peer_addr.sin_addr.s_addr,
							pfcp_header->message_type, ACC,SX);

        msg.rx_interface = PGW_SXB; 
        handle_pfcp_message(&msg);
	}
	return 0;
}
