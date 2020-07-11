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
#include "pfcp_timer.h"
#include "sm_structs_api.h"
#include "pfcp.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "cp_config.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config_defs.h"

extern udp_sock_t my_sock;
extern struct rte_hash *heartbeat_recovery_hash;
uint8_t pfcp_rx[1024]; /* TODO: Decide size */

static 
int handle_pfcp_association_setup_response(msg_info *msg)
{
	upf_context_t *upf_context = NULL;
    int ret=0;

    assert(msg->msg_type == PFCP_ASSOCIATION_SETUP_RESPONSE);
    // Requirement : 
    // 1. node_id should come as name or ip address
    // 2. Can UPF change its address and can it be different 
    memcpy(&msg->upf_ipv4.s_addr,
            &msg->pfcp_msg.pfcp_ass_resp.node_id.node_id_value,
            IPV4_SIZE);

	/*Retrive association state based on UPF IP. */
	ret = rte_hash_lookup_data(upf_context_by_ip_hash,
			(const void*) &(msg->upf_ipv4.s_addr), (void **) &(upf_context));

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: UPF entry not Found Msg_Type:%u, UPF IP:%u, Error_no:%d\n",
				__func__, msg->msg_type, msg->upf_ipv4.s_addr, ret);
		cs_error_response(msg,
						  GTPV2C_CAUSE_INVALID_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

		process_error_occured_handler(&msg, NULL);
		return -1;
	}
    msg->upf_context = upf_context;

	if(upf_context->timer_entry->rt.ti_id != 0) {
		stoptimer(&upf_context->timer_entry->rt.ti_id);
		deinittimer(&upf_context->timer_entry->rt.ti_id);
		/* free trans data when timer is deint */
		rte_free(upf_context->timer_entry);
        upf_context->timer_entry = NULL;
	}

	if(msg->pfcp_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED) {
		msg->state = ERROR_OCCURED_STATE;
		msg->event = ERROR_OCCURED_EVNT;
		msg->proc = INITIAL_PDN_ATTACH_PROC;
		clLog(sxlogger, eCLSeverityDebug,
				"Cause received  Association response is %d\n",
				msg->pfcp_msg.pfcp_ass_resp.cause.cause_value);

		/* TODO: Add handling to send association to next upf
		 * for each buffered CSR 
         * Cleanup should be done for each layer
         *   1. Cleanup GTPv2
         *   2. Cleanup IP-CAN if GX is enabled. 
         */
		cs_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        /* cleanup UE context */
		process_error_occured_handler(&msg, NULL);
		return -1;
	}

	msg->state = upf_context->state;
	/* Set Hard code value for temporary purpose as assoc is only in initial pdn */
	msg->proc = INITIAL_PDN_ATTACH_PROC;
	msg->event = PFCP_ASSOC_SETUP_RESP_RCVD_EVNT;

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_ASSOCIATION_SETUP_RESPONSE[%u], UPF_IP:%u, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type, msg->upf_ipv4.s_addr,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));
#if 0
    if(cp_config->cp_type != SGWC) {
        /* Init rule tables of user-plane */
        context->upf_ctxt->upf_sockaddr.sin_addr.s_addr = msg->upf_ipv4.s_addr;
        // init_dp_rule_tables();
    }
#endif
    process_assoc_resp_handler((void *)msg, (void *)msg->peer_addr);
    return 0;
}

// saegw - INITIAL_PDN_ATTACH_PROC,PFCP_PFD_MGMT_RESP_RCVD_STATE, PFCP_PFD_MGMT_RESP_RCVD_EVNT, => pfd_management_handler
// pgw - INITIAL_PDN_ATTACH_PROC PFCP_PFD_MGMT_RESP_RCVD_STATE PFCP_PFD_MGMT_RESP_RCVD_EVNT ==> pfd_management_handler
// sgw - INITIAL_PDN_ATTACH_PROC PFCP_PFD_MGMT_RESP_RCVD_STATE PFCP_PFD_MGMT_RESP_RCVD_EVNT - pfd_management_handler 
static
int handle_pfcp_pfd_management_response(msg_info *msg)
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
int handle_pfcp_session_est_response(msg_info *msg)
{
	struct resp_info *resp = NULL;
    assert(msg->msg_type == PFCP_SESSION_ESTABLISHMENT_RESPONSE);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive teid from session id */
	/* stop and delete the timer session for pfcp  est. req. */
	delete_pfcp_if_timer_entry(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid),
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid) - 5);

	/* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid, &resp) != 0) {
		cs_error_response(msg,
				  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
				  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		process_error_occured_handler(&msg, NULL);
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, Sess ID:%lu, \n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid);
		return -1;
	}

	if(msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value !=
			REQUESTACCEPTED){
		msg->state = ERROR_OCCURED_STATE;
		msg->event = ERROR_OCCURED_EVNT;
		msg->proc = INITIAL_PDN_ATTACH_PROC;
		cs_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		process_error_occured_handler(&msg, NULL);
		clLog(sxlogger, eCLSeverityDebug, "Cause received Est response is %d\n",
				msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value);
		return -1;
	}

    msg->event = PFCP_SESS_EST_RESP_RCVD_EVNT;
	msg->state = resp->state;
	msg->proc = resp->proc;
	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_ESTABLISHMENT_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

    /* For time being just getting rid of 3d FSM array */
    process_sess_est_resp_handler((void *)msg, NULL);
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
int handle_pfcp_session_modification_response(msg_info *msg)
{
	struct resp_info *resp = NULL;
    assert(msg->msg_type == PFCP_SESSION_MODIFICATION_RESPONSE);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */

	/* Retrive teid from session id */
	/* stop and delete timer entry for pfcp mod req */
	delete_pfcp_if_timer_entry(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5);

	/*Validate the modification is accepted or not. */
	if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
			clLog(sxlogger, eCLSeverityDebug, "Cause received Modify response is %d\n",
					msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value);
			mbr_error_response(msg,
							  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
							  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			return -1;
	}

	/* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
				&resp) != 0) {
		mbr_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u,"
				"Sess ID:%lu, n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return -1;
	}

	msg->state = resp->state;
	msg->proc = resp->proc;
    msg->event = PFCP_SESS_MOD_RESP_RCVD_EVNT;
	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

    /* For time being just getting rid of 3d FSM array */
    process_sess_mod_resp_handler((void *)msg, NULL);
    return 0;
}

// saegw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_sess_del_resp_handler
// saegw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT => process_pfcp_sess_del_resp_dbr_handler
// pgw - DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_sess_del_resp_handler
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE  PFCP_SESS_DEL_RESP_RCVD_EVNT ==> process_pfcp_sess_del_resp_dbr_handler
// sgw DETACH_PROC PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : PFCP_SESS_DEL_RESP_RCVD_EVNT 
// sgw PDN_GW_INIT_BEARER_DEACTIVATION PFCP_SESS_DEL_REQ_SNT_STATE PFCP_SESS_DEL_RESP_RCVD_EVNT : process_pfcp_sess_del_resp_dbr_handler 

static
int handle_pfcp_session_delete_response(msg_info *msg)
{
	struct resp_info *resp = NULL;
    assert(msg->msg_type == PFCP_SESSION_DELETION_RESPONSE);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */

	/* Retrive teid from session id */
	/* stop and delete timer entry for pfcp sess del req */
	delete_pfcp_if_timer_entry(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid),
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid) - 5);

	if(msg->pfcp_msg.pfcp_sess_del_resp.cause.cause_value != REQUESTACCEPTED){

		clLog(sxlogger, eCLSeverityCritical, "Cause received Del response is %d\n",
				msg->pfcp_msg.pfcp_sess_del_resp.cause.cause_value);
		ds_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}


    /* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
				&resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, "
				"Sess ID:%lu\n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
		ds_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	msg->state = resp->state;
	msg->proc = resp->proc;
    msg->event = PFCP_SESS_DEL_RESP_RCVD_EVNT;

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_DELETION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

    /* TODO - For time being just getting rid of 3d FSM array */
    process_sess_del_resp_handler((void *)msg, NULL);
    return 0;
}

// saegw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
// saegw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC CONNECTED_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler
// sgw - CONN_SUSPEND_PROC IDEL_STATE PFCP_SESS_RPT_REQ_RCVD_EVNT ==> process_rpt_req_handler 
static
int handle_pfcp_session_report_req_msg(msg_info *msg)
{
	struct resp_info *resp = NULL;
    assert(msg->msg_type == PFCP_SESSION_REPORT_REQUEST);
    /*
     * if session found then detect retransmission
     * if no retransmission then delete the existing session
     * handler new event  
     */
	/* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
				&resp) != 0) {
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
	msg->state = resp->state;
	msg->proc = resp->proc;
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
int handle_pfcp_session_set_delete_request(msg_info *msg)
{
	struct resp_info *resp = NULL;
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
    RTE_SET_USED(resp);
    return 0;
}
 
static 
void handle_pfcp_message(msg_info *msg)
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
msg_handler_sx_n4(struct sockaddr_in *peer_addr)
{
	int ret = 0, bytes_rx = 0;

	/* TODO: Move this rx */
	if ((bytes_rx = pfcp_recv(pfcp_rx, 512,
					peer_addr)) < 0) {
		perror("msgrecv");
		return -1;
	}

	pfcp_header_t *pfcp_header = (pfcp_header_t *) pfcp_rx;
	msg_info msg = {0};
	if(pfcp_header->message_type == PFCP_HEARTBEAT_REQUEST){

		printf("Heartbit request received from UP %s \n",inet_ntoa(peer_addr->sin_addr));
		update_cli_stats(peer_addr->sin_addr.s_addr,
				pfcp_header->message_type,RCVD,SX);

		ret = process_heartbeat_request(pfcp_rx, peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat request\n", __func__);
			return -1;
		}
		return 0;
	}else if(pfcp_header->message_type == PFCP_HEARTBEAT_RESPONSE){
		printf("Heartbit response received from UP %s \n",inet_ntoa(peer_addr->sin_addr));
		ret = process_heartbeat_response(pfcp_rx, peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat response\n", __func__);
			return -1;
		} else {

			update_cli_stats(peer_addr->sin_addr.s_addr,
					PFCP_HEARTBEAT_RESPONSE,RCVD,SX);

		}
		return 0;
	} else {
		// Requirement - cleanup - why this is called as response ? it could be request mesage as well 
		printf("PFCP message %d  received from UP %s \n",pfcp_header->message_type, inet_ntoa(peer_addr->sin_addr));
		/*Reset periodic timers*/
        if(pfcp_header->message_type != PFCP_ASSOCIATION_SETUP_RESPONSE) 
		    process_response(peer_addr->sin_addr.s_addr);
        msg.peer_addr = peer_addr;

        // TODO - PORT peer address should be copied to msg 
		if ((ret = pfcp_pcnd_check(pfcp_rx, &msg, bytes_rx)) != 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, REJ,SX);
			return -1;
		}

        // validate message content - validate the presence of IEs
        ret = validate_pfcp_message_content(&msg);
        if(ret != 0) 
        {
            // validatation failed;
            printf("PFCP message validation failed \n");
            return ret;
        }

        /* Event figured out from msg. 
         * Proc  found from user context and message content, message type 
         * State is on the pdn connection (for now)..Need some more attention here later   
         */
		if(pfcp_header->message_type == PFCP_SESSION_REPORT_REQUEST)
			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, RCVD,SX);
		else
			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, ACC,SX);

        msg.rx_interface = PGW_SXB; 
        handle_pfcp_message(&msg);
	}
	return 0;
}
