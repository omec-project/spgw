// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifdef FUTURE_NEED
int process_pfcp_sess_mod_resp_ubr_handler(void *data, void *unused_param)
{
	ue_context_t *context = NULL;
	uint8_t ebi_index = 0;

	msg_info_t *msg = (msg_info_t *)data;
	uint32_t teid = UE_SESS_ID(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);


	resp = get_sess_entry_seid(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
	if (resp == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu",
				msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	if(resp->num_of_bearers)
		ebi_index = resp->list_bearer_ids[0] - 5;

	/* Retrieve the UE context */
	context = (ue_context_t *)get_ue_context(teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

    if(cp_config->gx_enabled) {
        gen_reauth_response(context, ebi_index);
    }

	context->eps_bearers[ebi_index]->pdn->state = CONNECTED_STATE;

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;

}
#endif
