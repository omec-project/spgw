// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "tables/tables.h"
#ifdef FUTURE_NEED
// sgw - UPDATE_BEARER_PROC CONNECTED_STATE UPDATE_BEARER_REQ_RCVD_EVNT - process_update_bearer_request_handler
// sgw - UPDATE_BEARER_PROC IDEL_STATE UPDATE_BEARER_REQ_RCVD_EVNT - process_update_bearer_request_handler 
int handle_update_bearer_request_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);

    if((ret = decode_upd_bearer_req((uint8_t *) gtpv2c_rx,
                    &msg->gtpc_msg.ub_req) == 0))
        return -1;


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

int
process_update_bearer_request(upd_bearer_req_t *ubr)
{
	int ret = 0;
	upd_bearer_req_t ubr_req = {0};
	uint8_t bearer_id = 0;
	uint8_t ebi_index = 0;
	pdn_connection_t *pdn_cntxt = NULL;
	uint16_t payload_length = 0;

	ue_context_t *context = NULL;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	/* for now taking 0th element bearer id bcz
	 * a request will come from commom PGW for which PDN is same
	 */
	ebi_index = ubr->bearer_contexts[0].eps_bearer_id.ebi_ebi - 5;

	ret = get_ue_context_by_sgw_s5s8_teid(ubr->header.teid.has_teid.teid, &context);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	pdn_cntxt = context->eps_bearers[ebi_index]->pdn;

	if (get_sess_entry_seid(pdn_cntxt->seid, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, pdn_cntxt->seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	set_gtpv2c_teid_header((gtpv2c_header_t *) &ubr_req, GTP_UPDATE_BEARER_REQ,
 							 context->s11_mme_gtpc_teid, ubr->header.teid.has_teid.seq);

	if(ubr->apn_ambr.header.len){
		ubr_req.apn_ambr.apn_ambr_uplnk = ubr->apn_ambr.apn_ambr_uplnk;
		ubr_req.apn_ambr.apn_ambr_dnlnk = ubr->apn_ambr.apn_ambr_dnlnk;
		set_ie_header(&ubr_req.apn_ambr.header, GTP_IE_AGG_MAX_BIT_RATE, IE_INSTANCE_ZERO,
																			sizeof(uint64_t));
	}else{

		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;

	}

	if(!ubr->bearer_context_count)
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;

	if(ubr->indctn_flgs.header.len){
		set_ie_header(&ubr_req.indctn_flgs.header, GTP_IE_INDICATION,
								IE_INSTANCE_ZERO,
    		                    sizeof(gtp_indication_ie_t)- sizeof(ie_header_t));
		ubr_req.indctn_flgs.indication_retloc = 1;
	}

	ubr_req.bearer_context_count = ubr->bearer_context_count;
	for(uint32_t i = 0; i < ubr->bearer_context_count; i++){

		bearer_id = ubr->bearer_contexts[i].eps_bearer_id.ebi_ebi - 5;
		resp->list_bearer_ids[resp->num_of_bearers++] = bearer_id + 5;
		int len = 0;
		set_ie_header(&ubr_req.bearer_contexts[i].header,
									GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO, 0);
		//If QoS update is required
		/*if(ubr_req->bearer_contexts[i].bearer_lvl_qos.header.len){
			ubr_req->bearer_contexts[i].bearer_lvl_qos = ubr.bearer_contexts[i].bearer_lvl_qos;
			ubr_req.bearer_contexts[i].header.len += sizeof(gtp_bearer_qlty_of_svc_ie_t);
		}*/

		memset(ubr_req.bearer_contexts[i].tft.eps_bearer_lvl_tft, 0, 257);
		memcpy(ubr_req.bearer_contexts[i].tft.eps_bearer_lvl_tft,
					ubr->bearer_contexts[i].tft.eps_bearer_lvl_tft, 257);

		uint8_t tft_len = ubr->bearer_contexts[i].tft.header.len;
		set_ie_header(&ubr_req.bearer_contexts[i].tft.header,
			GTP_IE_EPS_BEARER_LVL_TRAFFIC_FLOW_TMPL, IE_INSTANCE_ZERO, tft_len);
		len = tft_len + IE_HEADER_SIZE;

		ubr_req.bearer_contexts[i].header.len += len;

		set_ebi(&ubr_req.bearer_contexts[i].eps_bearer_id,
					IE_INSTANCE_ZERO, context->eps_bearers[bearer_id]->eps_bearer_id);
		ubr_req.bearer_contexts[i].header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

	}

	pdn_cntxt->proc = UPDATE_BEARER_PROC;
	pdn_cntxt->state = UPDATE_BEARER_REQ_SNT_STATE;

	resp->msg_type = GTP_UPDATE_BEARER_REQ;
	resp->state =  UPDATE_BEARER_REQ_SNT_STATE;
	resp->proc =  UPDATE_BEARER_PROC;
	//Send ub_request to MME
	uint16_t msg_len = 0;
	msg_len = encode_upd_bearer_req(&ubr_req, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);


	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr, sizeof(struct sockaddr_in));

	return 0;
}

int process_update_bearer_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_update_bearer_request(&msg->gtpc_msg.ub_req);
	if (ret) {
		if(ret != -1)
			ubr_error_response(msg, ret, S5S8_IFACE);
		clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
		return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;

}

#endif
