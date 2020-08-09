#ifdef FUTURE_NEED
int process_pfcp_sess_mod_resp_ubr_handler(void *data, void *unused_param)
{
	int ret = 0;
	ue_context_t *context = NULL;
	uint8_t ebi_index = 0;

	msg_info_t *msg = (msg_info_t *)data;
	uint32_t teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);


	if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
																			&resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	if(resp->num_of_bearers)
		ebi_index = resp->list_bearer_ids[0] - 5;

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					teid);
	}

    if(cp_config->gx_enabled) {
        gen_reauth_response(context, ebi_index);
    }

	context->eps_bearers[ebi_index]->pdn->state = CONNECTED_STATE;

	RTE_SET_USED(unused_param);
	return 0;

}
#endif
