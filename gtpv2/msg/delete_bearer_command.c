// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#ifdef FUTURE_NEED
// saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT ==> process_delete_bearer_command_handler
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT - process_delete_bearer_command_handler
// sgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CONNECTED_STATE DELETE_BER_CMD_RCVD_EVNT - process_delete_bearer_command_handler 
int handle_delete_bearer_cmd_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    uint8_t ebi_index;
    ue_context_t *context = NULL;
    int ret;

    if((ret = decode_del_bearer_cmd((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.del_ber_cmd) == 0)) {
        return -1;
    }

	gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	ebi_index = msg->gtpc_msg.del_ber_cmd.bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;

	if(get_ue_context(gtpv2c_rx->teid.has_teid.teid, &context) != 0) {
		return -1;
	}
	msg->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
#if 0
	if (update_ue_proc(context->s11_sgw_gtpc_teid,
					msg->proc ,ebi_index) != 0) {
			LOG_MSG(LOG_ERROR, "%s failed\n", __func__);
			return -1;
	}
#endif
	context->eps_bearers[ebi_index]->pdn->proc =  msg->proc;

//		msg->state = context->eps_bearers[ebi_index]->pdn->state;
	msg->state = CONNECTED_STATE;
	msg->event = DELETE_BER_CMD_RCVD_EVNT;
    return 0;
}

void
set_delete_bearer_command(del_bearer_cmd_t *del_bearer_cmd, pdn_connection_t *pdn, gtpv2c_header_t *gtpv2c_tx)
{
	del_bearer_cmd_t del_cmd = {0};
	ue_context_t *context = NULL;
	del_cmd.header.gtpc.message_len = 0;
	get_ue_context(del_bearer_cmd->header.teid.has_teid.teid, &context);
	pdn->context->sequence = del_bearer_cmd->header.teid.has_teid.seq;
	set_gtpv2c_teid_header((gtpv2c_header_t *) &del_cmd, GTP_DELETE_BEARER_CMD,
			pdn->s5s8_pgw_gtpc_teid, del_bearer_cmd->header.teid.has_teid.seq);

	/*Below IE are Condition IE's*/

	set_ipv4_fteid(&del_cmd.sender_fteid_ctl_plane, GTPV2C_IFTYPE_S5S8_SGW_GTPC,
			IE_INSTANCE_ZERO, pdn->s5s8_sgw_gtpc_ipv4,
			pdn->s5s8_sgw_gtpc_teid);

	del_cmd.header.gtpc.message_len += del_bearer_cmd->sender_fteid_ctl_plane.header.len + sizeof(ie_header_t);

	if(del_bearer_cmd->uli.header.len != 0) {
		/*set uli*/
		memcpy(&del_cmd.uli, &(del_bearer_cmd->uli), sizeof(gtp_user_loc_info_ie_t));
		set_ie_header(&del_cmd.uli.header, GTP_IE_USER_LOC_INFO, IE_INSTANCE_ZERO, del_bearer_cmd->uli.header.len);
		del_cmd.header.gtpc.message_len += del_bearer_cmd->uli.header.len + sizeof(ie_header_t);

	}

	if(del_bearer_cmd->uli_timestamp.header.len != 0) {
		/*set uli timestamp*/
		memcpy(&del_cmd.uli_timestamp, &(del_bearer_cmd->uli_timestamp), sizeof(gtp_uli_timestamp_ie_t));
		set_ie_header(&del_cmd.uli_timestamp.header, GTP_IE_ULI_TIMESTAMP, IE_INSTANCE_ZERO,
				del_bearer_cmd->uli_timestamp.header.len);
		del_cmd.header.gtpc.message_len += del_bearer_cmd->uli_timestamp.header.len + sizeof(ie_header_t);
	}

	if(del_bearer_cmd->ue_time_zone.header.len != 0) {

		memcpy(&del_cmd.ue_time_zone, &(del_bearer_cmd->ue_time_zone), sizeof(gtp_ue_time_zone_ie_t));
		set_ie_header(&del_cmd.ue_time_zone.header, GTP_IE_UE_TIME_ZONE, IE_INSTANCE_ZERO, del_bearer_cmd->ue_time_zone.header.len);
		del_cmd.header.gtpc.message_len += del_bearer_cmd->ue_time_zone.header.len + sizeof(ie_header_t);
	}
	if(del_bearer_cmd->mmes4_sgsns_ovrld_ctl_info.header.len != 0) {

		memcpy(&del_cmd.mmes4_sgsns_ovrld_ctl_info, &(del_bearer_cmd->mmes4_sgsns_ovrld_ctl_info), sizeof(gtp_ovrld_ctl_info_ie_t));
		set_ie_header(&del_cmd.mmes4_sgsns_ovrld_ctl_info.header, GTP_IE_OVRLD_CTL_INFO, IE_INSTANCE_ZERO,
				del_bearer_cmd->mmes4_sgsns_ovrld_ctl_info.header.len);
		del_cmd.header.gtpc.message_len += del_bearer_cmd->mmes4_sgsns_ovrld_ctl_info.header.len + sizeof(ie_header_t);
	}

	if(del_bearer_cmd->sgws_ovrld_ctl_info.header.len != 0) {

		memcpy(&del_cmd.sgws_ovrld_ctl_info, &(del_bearer_cmd->sgws_ovrld_ctl_info), sizeof(gtp_ovrld_ctl_info_ie_t));
		set_ie_header(&del_cmd.sgws_ovrld_ctl_info.header, GTP_IE_OVRLD_CTL_INFO, IE_INSTANCE_ZERO,
				del_bearer_cmd->sgws_ovrld_ctl_info.header.len);
		del_cmd.header.gtpc.message_len += del_bearer_cmd->sgws_ovrld_ctl_info.header.len + sizeof(ie_header_t);
	}

	if(del_bearer_cmd->secdry_rat_usage_data_rpt.header.len != 0) {

		memcpy(&del_cmd.secdry_rat_usage_data_rpt, &(del_bearer_cmd->secdry_rat_usage_data_rpt), sizeof(gtp_secdry_rat_usage_data_rpt_ie_t));
		set_ie_header(&del_cmd.secdry_rat_usage_data_rpt.header, GTP_IE_SECDRY_RAT_USAGE_DATA_RPT, IE_INSTANCE_ZERO,
				del_bearer_cmd->secdry_rat_usage_data_rpt.header.len);

		del_cmd.header.gtpc.message_len += del_bearer_cmd->secdry_rat_usage_data_rpt.header.len + sizeof(ie_header_t);
	}

	del_cmd.bearer_count = del_bearer_cmd->bearer_count;

	for(uint8_t i= 0; i< del_bearer_cmd->bearer_count; i++) {

		set_ie_header(&del_cmd.bearer_contexts[i].header, GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO,
				0);

		set_ebi(&del_cmd.bearer_contexts[i].eps_bearer_id,
					IE_INSTANCE_ZERO,del_bearer_cmd->bearer_contexts[i].eps_bearer_id.ebi_ebi);

		del_cmd.bearer_contexts[i].header.len +=
	          sizeof(uint8_t) + IE_HEADER_SIZE;

		del_cmd.header.gtpc.message_len += del_bearer_cmd->bearer_contexts[i].header.len
					+ sizeof(ie_header_t);
	}


	uint16_t msg_len = 0;
	msg_len = encode_del_bearer_cmd(&del_cmd, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

}

/*
 * The Function handles when MME sends Delete Bearer CMD to SGWC and
 * also when SGWC sends the same to PGWC
*/

int
process_delete_bearer_command_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;
	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_delete_bearer_cmd_request(&msg->gtpc_msg.del_ber_cmd, gtpv2c_tx);

	if(ret != 0) {
	/* TODO:set error response*/
	LOG_MSG(LOG_ERROR, "%s : Error: %d \n", __func__, ret);
	}

	if (SGWC == cp_config->cp_type ) {
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);


	gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
			(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
            sizeof(struct sockaddr_in));
	}

	RTE_SET_USED(unused_param);

	return 0;
}

int
process_delete_bearer_cmd_request(del_bearer_cmd_t *del_bearer_cmd, gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	pdn_connection_t *pdn = NULL;
	int ebi_index = 0;

	ret = get_ue_context(del_bearer_cmd->header.teid.has_teid.teid, &context);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:%d Failed to update UE State for teid: %u\n",
	                     __func__, __LINE__,
	                  del_bearer_cmd->header.teid.has_teid.teid);
	}
	ebi_index = del_bearer_cmd->bearer_contexts[ebi_index].eps_bearer_id.ebi_ebi -5;

	bearer = context->eps_bearers[ebi_index];
	pdn = bearer->pdn;


    if (SAEGWC == cp_config->cp_type || PGWC == cp_config->cp_type) {
        if(cp_config->gx_enabled) {
            if (ccru_req_for_bear_termination(pdn , bearer)) {
                LOG_MSG(LOG_ERROR, "%s:%d Error: %s \n", __func__, __LINE__,
                        strerror(errno));
                return -1;
            }
        }
    } else if(SGWC == cp_config->cp_type) {

		set_delete_bearer_command(del_bearer_cmd, pdn, gtpv2c_tx);
		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
			               htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);

	}
    pdn->state = CONNECTED_STATE;
    if (get_sess_entry_seid(pdn->seid, &resp) != 0){
        LOG_MSG(LOG_ERROR, "%s:%d NO Session Entry Found for sess ID:%lu\n",
                __func__, __LINE__, pdn->seid);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    resp->eps_bearer_id = ebi_index;
    resp->msg_type = GTP_DELETE_BEARER_CMD;
    resp->state = CONNECTED_STATE;//need to see this
    resp->gtpc_msg.del_bearer_cmd = *del_bearer_cmd;
    resp->gtpc_msg.del_bearer_cmd.header.teid.has_teid.seq = del_bearer_cmd->header.teid.has_teid.seq;
    resp->proc = pdn->proc;
    resp->bearer_count = del_bearer_cmd->bearer_count;
    for (uint8_t iCnt = 0; iCnt < del_bearer_cmd->bearer_count; ++iCnt) {
        resp->eps_bearer_ids[iCnt] = del_bearer_cmd->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
    }

	return 0;
}

#endif
