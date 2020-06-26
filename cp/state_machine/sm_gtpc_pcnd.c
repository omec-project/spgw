// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "sm_pcnd.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "gtp_messages_decoder.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config.h"
#include "gtpc_timer.h"
#include "pfcp_timer.h"
#include "gtpv2_interface.h"
#include "cp_timers.h"
#include "clogger.h"
#include "sm_structs_api.h"

#ifdef USE_REST
#include "cp_main.h"
#endif

#include "cp_config.h"
#include "gw_adapter.h"

extern struct cp_stats_t cp_stats;

uint8_t
gtpc_pcnd_check(gtpv2c_header_t *gtpv2c_rx, msg_info *msg, int bytes_rx)
{
	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	msg->msg_type = gtpv2c_rx->gtpc.message_type;
	int ebi_index = 0;

	if ((unsigned)bytes_rx != (ntohs(gtpv2c_rx->gtpc.message_len) + sizeof(gtpv2c_rx->gtpc))) {
		ret = GTPV2C_CAUSE_INVALID_LENGTH;
		/* According to 29.274 7.7.7, if message is request,
		 * reply with cause = GTPV2C_CAUSE_INVALID_LENGTH
		 *  should be sent - ignoring packet for now
		 */
		clLog(clSystemLog, eCLSeverityCritical, "GTPv2C Received UDP Payload:"
				"\n\t(%d bytes) with gtpv2c + "
				"header (%u + %lu) = %lu bytes\n",
				bytes_rx, ntohs(gtpv2c_rx->gtpc.message_len),
				sizeof(gtpv2c_rx->gtpc),
				ntohs(gtpv2c_rx->gtpc.message_len)
				+ sizeof(gtpv2c_rx->gtpc));
		return ret;
	}

	if(bytes_rx > 0) {
		if (gtpv2c_rx->gtpc.version < GTP_VERSION_GTPV2C) {
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}else if (gtpv2c_rx->gtpc.version > GTP_VERSION_GTPV2C) {
			send_version_not_supported(cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
					gtpv2c_rx->teid.has_teid.seq);
			fprintf(stderr, "Discarding packet due to gtp version is not supported..");
			return GTPV2C_CAUSE_VERSION_NOT_SUPPORTED;
		}
	}

	switch(msg->msg_type) {

		case GTP_CREATE_SESSION_REQ: {

			if ((ret = decode_check_csr(gtpv2c_rx, &msg->gtpc_msg.csr)) != 0){
				if(ret != -1)
					cs_error_response(msg, ret,
							cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
				return -1;
			}
		break;
	}

	case GTP_CREATE_SESSION_RSP: {
			ret = decode_create_sess_rsp((uint8_t *)gtpv2c_rx, &msg->gtpc_msg.cs_rsp);
			if(!ret)
				return -1;

		break;
	}

	case GTP_MODIFY_BEARER_REQ: {
			/*Decode the received msg and stored into the struct. */
			if((ret = decode_mod_bearer_req((uint8_t *) gtpv2c_rx,
							&msg->gtpc_msg.mbr) == 0)) {
				return -1;
			}
			break;
	}

	case GTP_MODIFY_BEARER_RSP: {
		if((ret = decode_mod_bearer_rsp((uint8_t *) gtpv2c_rx,
					&msg->gtpc_msg.mb_rsp) == 0)) {
			return -1;
		}

		break;
	}

	case GTP_DELETE_SESSION_REQ: {
			/* Decode delete session request */

			ret = decode_del_sess_req((uint8_t *) gtpv2c_rx,
					&msg->gtpc_msg.dsr);
			if (ret == 0)
				return -1;
		break;
	}

	case GTP_DELETE_SESSION_RSP: {

		ret = decode_del_sess_rsp((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.ds_rsp);
		if(ret == 0)
			return -1;

		break;
		}
	case GTP_RELEASE_ACCESS_BEARERS_REQ: {
			/* Parse the Relaese access bearer request message and update State and Event */
			/* TODO: Revisit after libgtpv2c support */
			ret = parse_release_access_bearer_request(gtpv2c_rx,
					&msg->gtpc_msg.rel_acc_ber_req_t);
			if (ret)
				return ret;

		break;
	}

	case GTP_DOWNLINK_DATA_NOTIFICATION_ACK: {
			/* TODO: Revisit after libgtpv2c support */
			ret = parse_downlink_data_notification_ack(gtpv2c_rx,
				&msg->gtpc_msg.ddn_ack);
			if (ret)
				return ret;

		break;
	}

	case GTP_CREATE_BEARER_REQ:{

			if((ret = decode_create_bearer_req((uint8_t *) gtpv2c_rx,
							&msg->gtpc_msg.cb_req) == 0))
					return -1;

	   break;
	}

	case GTP_CREATE_BEARER_RSP:{
			if((ret = decode_create_bearer_rsp((uint8_t *) gtpv2c_rx,
						&msg->gtpc_msg.cb_rsp) == 0))
				return -1;

	   break;
	}

	case GTP_DELETE_BEARER_REQ:{

			if((ret = decode_del_bearer_req((uint8_t *) gtpv2c_rx,
							&msg->gtpc_msg.db_req) == 0))
					return -1;

	   break;
	}

	case GTP_DELETE_BEARER_RSP:{
			if((ret = decode_del_bearer_rsp((uint8_t *) gtpv2c_rx,
						&msg->gtpc_msg.db_rsp) == 0))
				return -1;

	 	  break;
	}
	case GTP_UPDATE_BEARER_REQ:{

		if((ret = decode_upd_bearer_req((uint8_t *) gtpv2c_rx,
						&msg->gtpc_msg.ub_req) == 0))
			return -1;
		break;

	}

	case GTP_UPDATE_BEARER_RSP:{

		if((ret = decode_upd_bearer_rsp((uint8_t *) gtpv2c_rx,
						&msg->gtpc_msg.ub_rsp) == 0))
				return -1;

		break;
	}

	case GTP_DELETE_PDN_CONNECTION_SET_REQ: {
			if ((ret = decode_del_pdn_conn_set_req((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.del_pdn_req) == 0))
			    return -1;

		break;
	}

	case GTP_DELETE_PDN_CONNECTION_SET_RSP: {
			if ((ret = decode_del_pdn_conn_set_rsp((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.del_pdn_rsp) == 0))
			    return -1;
		break;
	}
	case GTP_UPDATE_PDN_CONNECTION_SET_REQ: {
			//if ((ret = decode_upd_pdn_conn_set_req((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.upd_pdn_req) == 0))
			//    return -1;

	   break;
	 }
	case GTP_DELETE_BEARER_CMD: {

			if((ret = decode_del_bearer_cmd((uint8_t *) gtpv2c_rx,
					&msg->gtpc_msg.del_ber_cmd) == 0)) {
					return -1;
			}


		break;
	}
	case GTP_UPDATE_PDN_CONNECTION_SET_RSP: {
			//if ((ret = decode_upd_pdn_conn_set_rsp((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.upd_pdn_rsp) == 0)))
			//    return -1;

		break;
	}
	case GTP_PGW_RESTART_NOTIFICATION_ACK: {
			if ((ret = decode_pgw_rstrt_notif_ack((uint8_t *) gtpv2c_rx, &msg->gtpc_msg.pgw_rstrt_notif_ack) == 0))
			    return -1;

		break;
	}
	default:
			/*If Event is not supported then we will called default handler. */
			/* Retrive UE state. */
			if (get_ue_context(ntohl(gtpv2c_rx->teid.has_teid.teid), &context) != 0) {
				msg->proc =  NONE_PROC;
                if (SGWC == cp_config->cp_type)
                    msg->state = SGWC_NONE_STATE;
                else {
                    if(cp_config->gx_enabled)
                        msg->state = PGWC_NONE_STATE;
                    else
                        msg->state = SGWC_NONE_STATE;
                }
			} else {
					pdn = GET_PDN(context, ebi_index);
					msg->state = pdn->state;
					msg->proc = pdn->proc;
			}

			msg->event = NONE_EVNT;

			clLog(clSystemLog, eCLSeverityCritical, "%s::process_msgs-"
					"\n\tcase: SAEGWC::spgw_cfg= %d;"
					"\n\tReceived GTPv2c Message Type: "
					"%s (%u) not supported... Discarding\n", __func__,
					cp_config->cp_type, gtp_type_str(gtpv2c_rx->gtpc.message_type),
					gtpv2c_rx->gtpc.message_type);
			return -1;
	}
	return 0;
}
