// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "clogger.h"
#include "cp_io_poll.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"

extern udp_sock_t my_sock;



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

#ifdef FUTURE_NEED
int process_pfcp_sess_mod_resp_ubr_handler(void *data, void *unused_param)
{
	int ret = 0;
	ue_context_t *context = NULL;
	uint8_t ebi_index = 0;

	msg_info_t *msg = (msg_info_t *)data;
	uint32_t teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);


	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
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
int
handle_pfcp_session_mod_response_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in peer_addr = {0};
    peer_addr = msg->peer_addr;

    process_response(peer_addr.sin_addr.s_addr);
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_sess_mod_rsp_t((uint8_t *)pfcp_rx,
            &msg->pfcp_msg.pfcp_sess_mod_resp);

    if(decoded <= 0)
    {
        clLog(sxlogger, eCLSeverityDebug, "DECODED bytes in Sess Modify Resp is %d\n",
                decoded);
        update_cli_stats(peer_addr.sin_addr.s_addr,
                pfcp_rx->message_type, REJ,SX);

        return -1;
    }

    update_cli_stats(peer_addr.sin_addr.s_addr,
            pfcp_rx->message_type, ACC,SX);


    handle_pfcp_session_modification_response(msg);
    return 0;
}

