// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_common.h"
#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "gtpv2_evt_handler.h"
#include "gtpv2c_error_rsp.h"
#include "assert.h"
#include "clogger.h"
#include "cp_peer.h"
#include "gw_adapter.h"
#include "gtpv2_interface.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
#include "cp_config_apis.h"
#include "ip_pool.h"
#include "gen_utils.h"
#include "pfcp_cp_session.h"
#include "pfcp_enum.h"
#include "pfcp.h"
#include "pfcp_cp_association.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "initial_attach_proc.h"
#include "rab_proc.h"
#include "detach_proc.h"
#include "service_request_proc.h"

extern int s11logger;
extern int s5s8logger;
extern udp_sock_t my_sock;

extern const uint32_t s5s8_sgw_gtpc_base_teid; /* 0xE0FFEE */
int
check_interface_type(uint8_t iface) 
{
	switch(iface){
		case GTPV2C_IFTYPE_S1U_ENODEB_GTPU:
			if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
				return DESTINATION_INTERFACE_VALUE_ACCESS;
			}
			break;
		case GTPV2C_IFTYPE_S5S8_SGW_GTPU:
			if (cp_config->cp_type == PGWC){
				return DESTINATION_INTERFACE_VALUE_ACCESS;
			}
			break;
		case GTPV2C_IFTYPE_S5S8_PGW_GTPU:
			if (cp_config->cp_type == SGWC){
				return DESTINATION_INTERFACE_VALUE_CORE;
			}
			break;
		case GTPV2C_IFTYPE_S1U_SGW_GTPU:
		case GTPV2C_IFTYPE_S11_MME_GTPC:
		case GTPV2C_IFTYPE_S11S4_SGW_GTPC:
		case GTPV2C_IFTYPE_S11U_SGW_GTPU:
		case GTPV2C_IFTYPE_S5S8_SGW_GTPC:
		case GTPV2C_IFTYPE_S5S8_PGW_GTPC:
		case GTPV2C_IFTYPE_S5S8_SGW_PIMPv6:
		case GTPV2C_IFTYPE_S5S8_PGW_PIMPv6:
		default:
			return -1;
			break;
	}
	return -1;
}


// saegw, INITIAL_PDN_ATTACH_PROC, CS_REQ_SNT_STATE, CS_RESP_RCVD_EVNT, process_cs_resp_handler
// saegw, SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT -> process_cs_resp_handler 
// pgw - INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 
// pgw - SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw   INITIAL_PDN_ATTACH_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler
// sgw SGW_RELOCATION_PROC CS_REQ_SNT_STATE CS_RESP_RCVD_EVNT ==> process_cs_resp_handler 

#ifdef FUTURE_NEED_SGW
int handle_create_session_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpc_delete_timer_entry(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid);

	if(msg->gtpc_msg.cs_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		cs_error_response(msg, msg->gtpc_msg.cs_rsp.cause.cause_value,
				        cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

    // Requirement : teid can be 0 from PDN GW 
	if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid, &context) != 0)
	{
		cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
					cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	uint8_t ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
	pdn_connection_t *pdn = GET_PDN(context, ebi_index);
	msg->state = pdn->state;
	msg->proc = pdn->proc;

	/*Set the appropriate event type.*/
	msg->event = CS_RESP_RCVD_EVNT;

	update_sys_stat(number_of_users, INCREMENT);
	update_sys_stat(number_of_active_session, INCREMENT);

	clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					gtpv2c_rx->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
#if 0
			gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

			/* Retrive UE Context */
			if (get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
				cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
																		  S5S8_IFACE);
				return -1;
			}

			msg->state = context->pdns[ebi_index]->state;
			msg->proc = context->pdns[ebi_index]->proc;

			/*Set the appropriate event type.*/
			msg->event = CS_RESP_RCVD_EVNT;

			clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					gtpv2c_rx->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));

#endif

    return 0;
}
#endif

#ifdef FUTURE_NEED_SGW
// sgw : SGW_RELOCATION_PROC DDN_ACK_RCVD_STATE MB_RESP_RCVD_EVNT => process_mbr_resp_handover_handler  
int handle_modify_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpc_delete_timer_entry(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid);

	if(msg->gtpc_msg.mb_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		cs_error_response(msg, msg->gtpc_msg.mb_rsp.cause.cause_value,
				cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0)
	{
		cs_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		process_error_occured_handler(&msg, NULL);

		return -1;
	}
	uint8_t ebi_index = msg->gtpc_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi - 5;
	pdn = GET_PDN(context, ebi_index);
	msg->state = pdn->state;
	msg->proc = pdn->proc;
	msg->event = MB_RESP_RCVD_EVNT;
    return 0;
}
#endif

#ifdef FUTURE_NEED_SGW
// saegw DETACH_PROC DS_REQ_SNT_STATE DS_RESP_RCVD_EVNT => process_ds_resp_handler
// sgw DETACH_PROC DS_REQ_SNT_STATE DS_RESP_RCVD_EVNT : process_ds_resp_handler 
int handle_delete_session_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    uint8_t ebi_index = 5; // ajay - todo use transaction 
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpc_delete_timer_entry(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid);

	if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid, &context) != 0)
	 {

		ds_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}

	if(msg->gtpc_msg.ds_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
		clLog(clSystemLog, eCLSeverityCritical, "Cause Req Error : (%s:%d)msg type :%u, cause ie : %u \n", __func__, __LINE__,
				msg->msg_type, msg->gtpc_msg.ds_rsp.cause.cause_value);

		 ds_error_response(msg, msg->gtpc_msg.ds_rsp.cause.cause_value,
						cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		return -1;
	}
	/*Set the appropriate procedure and state.*/
	msg->state = context->pdns[ebi_index]->state;
	msg->proc = context->pdns[ebi_index]->proc;
	/*Set the appropriate event type.*/
	msg->event = DS_RESP_RCVD_EVNT;

#if 0
	msg->state = DS_REQ_SNT_STATE;
	msg->proc =  DETACH_PROC;
	/*Set the appropriate event type.*/
	msg->event = DS_RESP_RCVD_EVNT;
#endif

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
		"Msg_Type:%s[%u], Teid:%u, "
		"Procedure:%s, State:%s, Event:%s\n",
		__func__, gtp_type_str(msg->msg_type), msg->msg_type,
		msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid,
		get_proc_string(msg->proc),
		get_state_string(msg->state), get_event_string(msg->event));
#if 0
// SGW egress ???
			msg->state = context->pdns[ebi_index]->state;
			msg->proc = context->pdns[ebi_index]->proc;

			/*Set the appropriate event type.*/
			msg->event = DS_RESP_RCVD_EVNT;


#endif
    return 0;
}
#endif

#ifdef FUTURE_NEEDS
// sgw - UPDATE_BEARER_PROC CONNECTED_STATE UPDATE_BEARER_REQ_RCVD_EVNT - process_update_bearer_request_handler
// sgw - UPDATE_BEARER_PROC IDEL_STATE UPDATE_BEARER_REQ_RCVD_EVNT - process_update_bearer_request_handler 
int handle_update_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	uint8_t ebi_index = msg->gtpc_msg.ub_req.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	//Vikrant Which ebi to be selected as multiple bearer in request
	if(get_ue_context_by_sgw_s5s8_teid(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
			fprintf(stderr , "%s:%d UE Context not found... 0x%x\n",__func__,
						__LINE__, gtpv2c_rx->teid.has_teid.teid);
			ubr_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
															S5S8_IFACE);
			return -1;
	}
	msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->proc = UPDATE_BEARER_PROC;
	msg->event = UPDATE_BEARER_REQ_RCVD_EVNT;
    return 0;
}

// saegw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler  
// pgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT process_update_bearer_response_handler 
// sgw - UPDATE_BEARER_PROC UPDATE_BEARER_REQ_SNT_STATE UPDATE_BEARER_RSP_RCVD_EVNT - process_update_bearer_response_handler
int handle_update_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    int ret = 0;

    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
    
	if(msg->gtpc_msg.ub_rsp.cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED){
			ubr_error_response(msg, msg->gtpc_msg.ub_rsp.cause.cause_value,
									cp_config->cp_type == SGWC ? S5S8_IFACE : GX_IFACE);
			return -1;
	}

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	uint8_t ebi_index = msg->gtpc_msg.ub_rsp.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;
	//Vikrant Which ebi to be selected as multiple bearer in request
	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid ,ebi_index)) > 0){
			msg->state = ret;
	}else{
		return -1;
	}
	msg->proc = UPDATE_BEARER_PROC;
	msg->event = UPDATE_BEARER_RSP_RCVD_EVNT;

    return 0;
}
// sgw - DED_BER_ACTIVATION_PROC CONNECTED_STATE CREATE_BER_REQ_RCVD_EVNT : process_create_bearer_request_handler 
// sgw : DED_BER_ACTIVATION_PROC IDEL_STATE CREATE_BER_REQ_RCVD_EVNT - process_create_bearer_request_handler 
int handle_create_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
	uint8_t ebi_index = msg->gtpc_msg.cb_req.lbi.ebi_ebi - 5;

	if(get_ue_context_by_sgw_s5s8_teid(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		fprintf(stderr , "%s:%d UE Context not found... 0x%x\n",__func__,
					__LINE__, gtpv2c_rx->teid.has_teid.teid);
		return -1;
	}
	msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->proc =  DED_BER_ACTIVATION_PROC;
	msg->event = CREATE_BER_REQ_RCVD_EVNT;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

// saegw DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_create_bearer_resp_handler
// pgw - DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT => process_cbresp_handler
// sgw  DED_BER_ACTIVATION_PROC CREATE_BER_REQ_SNT_STATE CREATE_BER_RESP_RCVD_EVNT ==> process_create_bearer_resp_handler 
int handle_create_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    int ret = 0;

    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(context);
    RTE_SET_USED(msg);

    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
	uint8_t ebi_index = msg->gtpc_msg.cb_rsp.bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid ,ebi_index)) > 0){
		msg->state = ret;
	}else{
		return -1;
	}

	msg->proc = DED_BER_ACTIVATION_PROC;
	msg->event = CREATE_BER_RESP_RCVD_EVNT;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

// sgw :  PDN_GW_INIT_BEARER_DEACTIVATION CONNECTED_STATE DELETE_BER_REQ_RCVD_EVNT - process_delete_bearer_request_handler 
// sgw : PDN_GW_INIT_BEARER_DEACTIVATION IDEL_STATE DELETE_BER_REQ_RCVD_EVNT - process_delete_bearer_request_handler
// sgw : MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_REQ_RCVD_EVNT : process_delete_bearer_req_handler 
int handle_delete_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    uint8_t ebi_index;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	if (msg->gtpc_msg.db_req.lbi.header.len) {
		ebi_index = msg->gtpc_msg.db_req.lbi.ebi_ebi - 5;
	} else {
		ebi_index = msg->gtpc_msg.db_req.eps_bearer_ids[0].ebi_ebi - 5;
	}

	if(get_ue_context_by_sgw_s5s8_teid(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		clLog(sxlogger, eCLSeverityCritical,
			"%s:%d UE Context not found... 0x%x\n",__func__,
			__LINE__, gtpv2c_rx->teid.has_teid.teid);
		return -1;
	}

	if(context->eps_bearers[ebi_index]->pdn->proc == MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC){
		msg->proc = context->eps_bearers[ebi_index]->pdn->proc;
	}else{
		msg->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		context->eps_bearers[ebi_index]->pdn->proc = msg->proc;
	}
	msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->event = DELETE_BER_REQ_RCVD_EVNT;

	context->eps_bearers[ebi_index]->pdn->proc = msg->proc;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));


    return 0;
}

#ifdef FUTURE_NEEDS
// saegw - PDN_GW_INIT_BEARER_DEACTIVATION  DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_resp_handler  
// saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_response_handler 
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_resp_handler
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_response_handler
// sgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT : process_delete_bearer_resp_handler 
// sgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT - process_delete_bearer_response_handler 
int handle_delete_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    uint8_t ebi_index;
    ue_context_t *context = NULL;
    int ret = 0;
    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.sin_port;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);
    assert(gtpc_trans);
	stop_transaction_timer(gtpc_trans);


    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	if (msg->gtpc_msg.db_rsp.lbi.header.len) {
		ebi_index = msg->gtpc_msg.db_rsp.lbi.ebi_ebi - 5;
	} else {
		ebi_index = msg->gtpc_msg.db_rsp.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;
	}

	if(get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		return -1;
	}
	if((ret = get_ue_state(gtpv2c_rx->teid.has_teid.teid, ebi_index)) > 0){
		msg->state = ret;
	}else{
		return -1;
	}

	if(context->eps_bearers[ebi_index]->pdn->proc == MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC){
		msg->proc = context->eps_bearers[ebi_index]->pdn->proc;
	}else{
		msg->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		context->eps_bearers[ebi_index]->pdn->proc = msg->proc;
	}
	msg->event = DELETE_BER_RESP_RCVD_EVNT;
	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			gtpv2c_rx->teid.has_teid.teid,
			get_state_string(msg->state), get_event_string(msg->event));


    process_delete_bearer_response_handler(msg, NULL);
    return 0;
}
#endif

// saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT ==> process_delete_bearer_command_handler
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT - process_delete_bearer_command_handler
// sgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT - process_delete_bearer_command_handler 
int handle_delete_bearer_cmd_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    uint8_t ebi_index;
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	ebi_index = msg->gtpc_msg.del_ber_cmd.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;

	if(get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		return -1;
	}
	msg->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
	if (update_ue_proc(context->s11_sgw_gtpc_teid,
					msg->proc ,ebi_index) != 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s failed\n", __func__);
			return -1;
	}
	context->eps_bearers[ebi_index]->pdn->proc =  msg->proc;

//		msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->state = CONNECTED_STATE;
	msg->event = DELETE_BER_CMD_RCVD_EVNT;
    return 0;
}

// saegw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT => process_del_pdn_conn_set_rsp  
// saegw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT => process_del_pdn_conn_set_req  
// pgw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT ==> process_del_pdn_conn_set_req
// sgw RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_RCVD_STATE DEL_PDN_CONN_SET_REQ_RCVD_EVNT process_del_pdn_conn_set_req

int handle_delete_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    RTE_SET_USED(gtpv2c_rx);
	msg->state = DEL_PDN_CONN_SET_REQ_RCVD_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->event = DEL_PDN_CONN_SET_REQ_RCVD_EVNT;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			" Msg_Type:%s[%u],"
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

// pgw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT : process_del_pdn_conn_set_rsp 
// sgw - RESTORATION_RECOVERY_PROC DEL_PDN_CONN_SET_REQ_SNT_STATE DEL_PDN_CONN_SET_RESP_RCVD_EVNT process_del_pdn_conn_set_rsp 
int handle_delete_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    RTE_SET_USED(gtpv2c_rx);
	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->state = DEL_PDN_CONN_SET_REQ_SNT_STATE;
	msg->event = DEL_PDN_CONN_SET_RESP_RCVD_EVNT;
    return 0;
}

int handle_update_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    RTE_SET_USED(gtpv2c_rx);
	//TODO: TEID based lookup
	//msg->state = ;
	msg->proc = RESTORATION_RECOVERY_PROC; 
	//msg->event = UPD_PDN_CONN_SET_REQ_RCVD_EVNT;

	//clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
	//		" Msg_Type:%s[%u],"
	//		"State:%s, Event:%s\n",
	//		__func__, gtp_type_str(msg->msg_type), msg->msg_type,
	//		get_state_string(msg->state), get_event_string(msg->event));
    return 0;
}

// saegw - RESTORATION_RECOVERY_PROC PGW_RSTRT_NOTIF_REQ_SNT_STATE PGW_RSTRT_NOTIF_ACK_RCVD_EVNT : process_pgw_rstrt_notif_ack  
int handle_update_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
	//TODO:TEID based lookup
    RTE_SET_USED(gtpv2c_rx);
	//msg->state = ;
	msg->proc = RESTORATION_RECOVERY_PROC; 
	//msg->proc = get_procedure(msg);
	//msg->event = UPD_PDN_CONN_SET_RESP_RCVD_EVNT;

	//clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
	//		" Msg_Type:%s[%u],"
	//		"State:%s, Event:%s\n",
	//		__func__, gtp_type_str(msg->msg_type), msg->msg_type,
	//		get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}

// pgw - RESTORATION_RECOVERY_PROC PGW_RSTRT_NOTIF_REQ_SNT_STATE PGW_RSTRT_NOTIF_ACK_RCVD_EVNT ==> process_pgw_rstrt_notif_ack 
// sgw - RESTORATION_RECOVERY_PROC PGW_RSTRT_NOTIF_REQ_SNT_STATE PGW_RSTRT_NOTIF_ACK_RCVD_EVNT ==> process_pgw_rstrt_notif_ack  
int handle_pgw_restart_notf_ack(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    RTE_SET_USED(gtpv2c_rx);
	msg->state = PGW_RSTRT_NOTIF_REQ_SNT_STATE;
	msg->proc = RESTORATION_RECOVERY_PROC;
	msg->event = PGW_RSTRT_NOTIF_ACK_RCVD_EVNT;

	clLog(s5s8logger, eCLSeverityDebug, "%s: Callback called for"
			" Msg_Type:%s[%u],"
			"State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			get_state_string(msg->state), get_event_string(msg->event));

    return 0;
}
#endif


int 
process_gtp_message(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg)
{
    int ret = 0;
    RTE_SET_USED(gtpv2c_rx);
    /* fetch user context */
    // find session context  
    //      non-zero teid - find context from teid 
    //      zero teid
    //          Request     : IMSI from the message
    //          Response    : transactions
    // Once we have found/not-found context then get procedure...based on UE context and message content  

    switch(msg->msg_type)
    {
#ifdef FUTURE_NEEDS
        case GTP_CREATE_SESSION_RSP:
        {
            handle_create_session_response_msg(gtpv2c_rx, msg);
            break;
        }
        case GTP_MODIFY_BEARER_RSP:
        {
            handle_modify_bearer_response_msg(gtpv2c_rx, msg);
            break;
        }
        case GTP_DELETE_SESSION_RSP:
        {
            handle_delete_session_response_msg(gtpv2c_rx,msg);
            break;
        }
        case GTP_DELETE_BEARER_REQ:
        {
            handle_delete_bearer_request_msg(gtpv2c_rx,msg);
            break;
        } 

        case GTP_DELETE_BEARER_RSP:
        {
            handle_delete_bearer_response_msg(gtpv2c_rx,msg);
            break;
        }

        case GTP_UPDATE_BEARER_REQ:
        {
            handle_update_bearer_request_msg(gtpv2c_rx,msg);
            break;
        } 

        case GTP_UPDATE_BEARER_RSP:
        {
            handle_update_bearer_response_msg(gtpv2c_rx,msg);
            break;
        }

        case GTP_CREATE_BEARER_REQ:
        {
            handle_create_bearer_request_msg(gtpv2c_rx,msg);
            break;
        } 

        case GTP_CREATE_BEARER_RSP:
        {
            handle_create_bearer_response_msg(gtpv2c_rx,msg);
            break;
        }

	    case GTP_DELETE_BEARER_CMD: 
        {
            handle_delete_bearer_cmd_msg(gtpv2c_rx,msg);
            break;
        }
	    case GTP_DELETE_PDN_CONNECTION_SET_REQ: 
        {
            handle_delete_pdn_conn_set_req(gtpv2c_rx,msg);
            break;
        }

	    case GTP_DELETE_PDN_CONNECTION_SET_RSP: 
        {
            handle_delete_pdn_conn_set_rsp(gtpv2c_rx,msg);
            break;
        }

	    case GTP_UPDATE_PDN_CONNECTION_SET_REQ: 
        {
            handle_update_pdn_conn_set_req(gtpv2c_rx,msg);
            break;
        }

	    case GTP_UPDATE_PDN_CONNECTION_SET_RSP: 
        {
            handle_update_pdn_conn_set_rsp(gtpv2c_rx,msg);
            break;
        }

	    case GTP_PGW_RESTART_NOTIFICATION_ACK: 
        {
            handle_pgw_restart_notf_ack(gtpv2c_rx,msg);
            break;
        }
#endif
        default:
        {
            assert(0); // unhandled gtp message reception
            break;
        }
    }
    return ret;
}
