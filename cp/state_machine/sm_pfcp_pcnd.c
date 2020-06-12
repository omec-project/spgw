// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "cp_global_defs.h"
#include "cp.h"
#include "gtpv2c.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_decoder.h"
#include "gtpv2c_error_rsp.h"
#include "cp_config_new.h"
#include "pfcp_timer.h"
#include "cp_config.h"
#include "pfcp_cp_set_ie.h"
#include "upf_struct.h"

extern struct cp_stats_t cp_stats;

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

		break;
	}

	case PFCP_PFD_MANAGEMENT_RESPONSE: {
			/* Decode pfd mgmt response */
			decoded = decode_pfcp_pfd_mgmt_rsp_t(pfcp_rx, &msg->pfcp_msg.pfcp_pfd_resp);
			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Pfd Mgmt Resp is %d\n",
					decoded);
		break;
	}

	case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_estab_rsp_t(pfcp_rx,
											&msg->pfcp_msg.pfcp_sess_est_resp);
			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Estab Resp is %d\n",
					 decoded);

		break;
	}

	case PFCP_SESSION_MODIFICATION_RESPONSE: {
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_mod_rsp_t(pfcp_rx,
					&msg->pfcp_msg.pfcp_sess_mod_resp);

			clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Modify Resp is %d\n",
					decoded);

		break;
	}

	case PFCP_SESSION_DELETION_RESPONSE: {
			/* Decode pfcp session delete response*/
			decoded = decode_pfcp_sess_del_rsp_t(pfcp_rx, &msg->pfcp_msg.pfcp_sess_del_resp);

					clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Del Resp is %d\n",
					decoded);

		break;
	}

	case PFCP_SESSION_REPORT_REQUEST: {
			/*Decode the received msg and stored into the struct*/
			decoded = decode_pfcp_sess_rpt_req_t(pfcp_rx,

							&msg->pfcp_msg.pfcp_sess_rep_req);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Report Request is %d\n",
					decoded);

		break;
	}

	case PFCP_SESSION_SET_DELETION_REQUEST:
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_set_del_req_t(pfcp_rx,
							&msg->pfcp_msg.pfcp_sess_set_del_req);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Set Deletion Request is %d\n",
					decoded);

		break;
	case PFCP_SESSION_SET_DELETION_RESPONSE:
			/*Decode the received msg and stored into the struct. */
			decoded = decode_pfcp_sess_set_del_rsp_t(pfcp_rx,
							&msg->pfcp_msg.pfcp_sess_set_del_rsp);

			clLog(sxlogger, eCLSeverityDebug, "DEOCED bytes in Sess Set Deletion Resp is %d\n",
					decoded);

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
