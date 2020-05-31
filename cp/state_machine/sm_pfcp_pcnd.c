/*
 * Copyright (c) 2019 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cp.h"
#include "gtpv2c.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "debug_str.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_decoder.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config_new.h"
#include "cp_timer.h"
#include "cp_config.h"

extern pfcp_config_t pfcp_config;
extern struct cp_stats_t cp_stats;
extern struct sockaddr_in upf_pfcp_sockaddr;

/**
 * @brief  : Validate pfcp messages
 * @param  : pfcp_header, message data
 * @param  : bytes_rx, number of bytes in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
static uint8_t
pcnd_check(pfcp_header_t *pfcp_header, int bytes_rx)
{
	RTE_SET_USED(pfcp_header);
	RTE_SET_USED(bytes_rx);
	/* int ret = 0; */
	/* TODO: Precondition of PFCP message need to handle later on. ]*/

	return 0;

}

uint8_t
pfcp_pcnd_check(uint8_t *pfcp_rx, msg_info *msg, int bytes_rx)
{
	int ret = 0;
	int decoded = 0;
	struct resp_info *resp = NULL;

	pfcp_header_t *pfcp_header = (pfcp_header_t *) pfcp_rx;

	if ((ret = pcnd_check(pfcp_header, bytes_rx)) != 0)
		return ret;

	msg->msg_type = pfcp_header->message_type;

	switch(msg->msg_type) {
	case PFCP_ASSOCIATION_SETUP_RESPONSE: {
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_assn_setup_rsp_t(pfcp_rx,
						&msg->pfcp_msg.pfcp_ass_resp);

			clLog(sxlogger, eCLSeverityDebug, "Decoded bytes [%d]\n", decoded);

			memcpy(&msg->upf_ipv4.s_addr,
					&msg->pfcp_msg.pfcp_ass_resp.node_id.node_id_value,
					IPV4_SIZE);
			if(cp_config->cp_type != SGWC) {
				/* Init rule tables of user-plane */
				upf_pfcp_sockaddr.sin_addr.s_addr = msg->upf_ipv4.s_addr;
				init_dp_rule_tables();
			}

			upf_context_t *upf_context = NULL;

			/*Retrive association state based on UPF IP. */
			ret = rte_hash_lookup_data(upf_context_by_ip_hash,
					(const void*) &(msg->upf_ipv4.s_addr), (void **) &(upf_context));
			if(upf_context->timer_entry->pt.ti_id != 0) {
				stoptimer(&upf_context->timer_entry->pt.ti_id);
				deinittimer(&upf_context->timer_entry->pt.ti_id);
				/* free peer data when timer is de int */
				rte_free(upf_context->timer_entry);
			}
			if(msg->pfcp_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED){

				msg->state = ERROR_OCCURED_STATE;
				msg->event = ERROR_OCCURED_EVNT;
				msg->proc = INITIAL_PDN_ATTACH_PROC;
				clLog(sxlogger, eCLSeverityDebug,
						"Cause received  Association response is %d\n",
						msg->pfcp_msg.pfcp_ass_resp.cause.cause_value);

				/* TODO: Add handling to send association to next upf
				 * for each buffered CSR */
				cs_error_response(msg,
								  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
								  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
				process_error_occured_handler(&msg, NULL);
				return -1;
			}


			if (ret >= 0) {
				msg->state = upf_context->state;

				/* Set Hard code value for temporary purpose as assoc is only in initial pdn */
				msg->proc = INITIAL_PDN_ATTACH_PROC;
			} else {
				cs_error_response(msg,
								  GTPV2C_CAUSE_INVALID_PEER,
								  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

				process_error_occured_handler(&msg, NULL);
				clLog(clSystemLog, eCLSeverityCritical, "%s: Entry not Found Msg_Type:%u, UPF IP:%u, Error_no:%d\n",
						__func__, msg->msg_type, msg->upf_ipv4.s_addr, ret);
				return -1;
			}

			/*Set the appropriate event type.*/
			msg->event = PFCP_ASSOC_SETUP_RESP_RCVD_EVNT;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:PFCP_ASSOCIATION_SETUP_RESPONSE[%u], UPF_IP:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type, msg->upf_ipv4.s_addr,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;
	}

	case PFCP_PFD_MANAGEMENT_RESPONSE: {
			/* Decode pfd mgmt response */
			decoded = decode_pfcp_pfd_mgmt_rsp_t(pfcp_rx, &msg->pfcp_msg.pfcp_pfd_resp);
			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Pfd Mgmt Resp is %d\n",
					decoded);
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
		break;
	}

	case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_estab_rsp_t(pfcp_rx,
											&msg->pfcp_msg.pfcp_sess_est_resp);
			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Estab Resp is %d\n",
					 decoded);
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
				clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						__func__, msg->msg_type,
						msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid, ret);
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

			msg->state = resp->state;
			msg->proc = resp->proc;

			/*Set the appropriate event type.*/
			msg->event = PFCP_SESS_EST_RESP_RCVD_EVNT;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:PFCP_SESSION_ESTABLISHMENT_RESPONSE[%u], Seid:%lu, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type,
					msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;
	}

	case PFCP_SESSION_MODIFICATION_RESPONSE: {
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_mod_rsp_t(pfcp_rx,
					&msg->pfcp_msg.pfcp_sess_mod_resp);

			clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Modify Resp is %d\n",
					decoded);

			/* Retrive teid from session id */
			/* stop and delete timer entry for pfcp mod req */
			delete_pfcp_if_timer_entry(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
						UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5);
			/*Validate the modification is accepted or not. */
			if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value !=
					REQUESTACCEPTED){
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
						"Sess ID:%lu, Error_no:%d\n",
						__func__, msg->msg_type,
						msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid, ret);
				return -1;
			}

			msg->state = resp->state;
			msg->proc = resp->proc;

			/*Set the appropriate event type.*/
			msg->event = PFCP_SESS_MOD_RESP_RCVD_EVNT;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type,
					msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;
	}

	case PFCP_SESSION_DELETION_RESPONSE: {
			/* Decode pfcp session delete response*/
			decoded = decode_pfcp_sess_del_rsp_t(pfcp_rx, &msg->pfcp_msg.pfcp_sess_del_resp);

					clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Del Resp is %d\n",
					decoded);

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
						"Sess ID:%lu, Error_no:%d\n",
						__func__, msg->msg_type,
						msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid, ret);
				ds_error_response(msg,
								  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
								  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
				return -1;
			}

			msg->state = resp->state;
			msg->proc = resp->proc;

			/*Set the appropriate event type.*/
			msg->event = PFCP_SESS_DEL_RESP_RCVD_EVNT ;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:PFCP_SESSION_DELETION_RESPONSE[%u], Seid:%lu, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type,
					msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;
	}

	case PFCP_SESSION_REPORT_REQUEST: {
			/*Decode the received msg and stored into the struct*/
			decoded = decode_pfcp_sess_rpt_req_t(pfcp_rx,

							&msg->pfcp_msg.pfcp_sess_rep_req);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Report Request is %d\n",
					decoded);

			/* Retrive the session information based on session id. */
			if (get_sess_entry(msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
						&resp) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						__func__, msg->msg_type,
						msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid, ret);
				return -1;
			}

			msg->state = resp->state;
			msg->proc = resp->proc;

			/*Set the appropriate event type.*/
			msg->event = PFCP_SESS_RPT_REQ_RCVD_EVNT;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:PFCP_SESSION_REPORT_REQUEST[%u], Seid:%lu, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type,
					msg->pfcp_msg.pfcp_sess_rep_req.header.seid_seqno.has_seid.seid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;
	}

	case PFCP_SESSION_SET_DELETION_REQUEST:
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_set_del_req_t(pfcp_rx,
							&msg->pfcp_msg.pfcp_sess_set_del_req);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Set Deletion Request is %d\n",
					decoded);

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
		break;
	case PFCP_SESSION_SET_DELETION_RESPONSE:
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_set_del_rsp_t(pfcp_rx,
							&msg->pfcp_msg.pfcp_sess_set_del_rsp);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Set Deletion Resp is %d\n",
					decoded);

			msg->state = PFCP_SESS_SET_DEL_REQ_SNT_STATE;
			msg->proc = RESTORATION_RECOVERY_PROC;

			/*Set the appropriate event type.*/
			msg->event = PFCP_SESS_SET_DEL_RESP_RCVD_EVNT;

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					" Msg_Type: PFCP_SESSION_SET_DELETION_RESPONSE[%u], "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, msg->msg_type,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));
		break;

	default:
			/* Retrive the session information based on session id */
			if ((get_sess_entry(pfcp_header->seid_seqno.has_seid.seid, &resp)) != 0 ) {
				msg->proc = NONE_PROC;
				if( SGWC == cp_config->cp_type )
					msg->state = SGWC_NONE_STATE;
				else
					msg->state = PGWC_NONE_STATE;
			} else {
				msg->state = resp->state;
				msg->proc = resp->proc;
			}

			msg->event = NONE_EVNT;

			clLog(clSystemLog, eCLSeverityCritical, "%s::process_msgs-"
					"\n\tcase: spgw_cfg= %d;"
					"\n\tReceived unprocessed PFCP Message_Type:%u"
					"... Discarding\n", __func__, cp_config->cp_type, msg->msg_type);
			return -1;
	}

	return 0;
}
