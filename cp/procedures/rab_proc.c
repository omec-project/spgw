
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "pfcp.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "sm_structs_api.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "sm_struct.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "gtpv2c_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "pfcp_cp_util.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "cp_transactions.h"
#include "spgw_cpp_wrapper.h"
#include "rab_proc.h"
#include "gtpv2c_error_rsp.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern udp_sock_t my_sock;

proc_context_t*
alloc_rab_proc(msg_info_t *msg)
{
    ue_context_t *context;
    proc_context_t *rab_proc;
    context = msg->ue_context;

    rab_proc = calloc(1, sizeof(proc_context_t));
    rab_proc->proc_type = msg->proc; 
    rab_proc->handler = rab_event_handler;
    rab_proc->ue_context = context;
    rab_proc->pdn_context = NULL;

    // set cross references in msg 
    msg->proc_context = rab_proc;
    msg->ue_context = msg->ue_context;
    msg->pdn_context = NULL;

    return rab_proc;
}

void 
rab_event_handler(void *proc, uint32_t event, void *data)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    RTE_SET_USED(proc_context);

    switch(event) {
        case REL_ACC_BER_REQ_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            process_rel_access_ber_req_handler(msg, NULL);
            break;
        } 
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            process_rab_proc_pfcp_mod_sess_rsp(msg);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

int
process_rel_access_ber_req_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
    gtpv2c_header_t *rab_header = &msg->gtpc_msg.rab.header;

	clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:%s[%u], Teid:%u, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, gtp_type_str(msg->msg_type), msg->msg_type,
			rab_header->teid.has_teid.teid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));


	/* TODO: Check return type and do further processing */
	ret = process_release_access_bearer_request(&msg->gtpc_msg.rab, msg->proc, msg);
    if (ret) {
        clLog(s11logger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
        return -1;
    }

	RTE_SET_USED(unused_param);
	return 0;
}

void 
process_release_access_bearer_request_pfcp_timeout(void *data)
{
    ue_context_t *ue_context = (ue_context_t *)data;
    proc_context_t *proc_context = ue_context->current_proc;

    msg_info_t msg = {0};
    msg.msg_type = PFCP_SESSION_MODIFICATION_RESPONSE;
    msg.proc_context = proc_context;

    proc_rab_failed(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_release_access_bearer_request(rel_acc_bearer_req_t *rel_acc_ber_req_t, uint8_t proc, msg_info_t *msg)
{
    RTE_SET_USED(proc);
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    proc_context_t *rab_proc = msg->proc_context;
	uint32_t sequence = 0;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    ue_context_t *ue_context = msg->ue_context;

	for (int i = 0; i < MAX_BEARERS; ++i) {
		if (ue_context->eps_bearers[i] == NULL)
			continue;

		bearer = ue_context->eps_bearers[ebi_index];
		if (!bearer) {
			clLog(clSystemLog, eCLSeverityCritical,
					"Retrive Context for release access bearer is non-existent EBI - "
					"Bitmap Inconsistency - Dropping packet\n");
			return -EPERM;
		}

		bearer->s1u_enb_gtpu_teid = 0;

		pdn = bearer->pdn;

#if 0
    // why we are doing it ??
		rel_acc_ber_req_t->context->pdns[ebi_index]->seid =
			SESS_ID((rel_acc_ber_req_t->context)->s11_sgw_gtpc_teid, bearer->eps_bearer_id);
#endif

		pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];
		pfcp_sess_mod_req.update_far_count = 1;
		for(int itr=0; itr < pfcp_sess_mod_req.update_far_count; itr++ ){
			update_far[itr].upd_frwdng_parms.outer_hdr_creation.teid =
				bearer->s1u_enb_gtpu_teid;
			update_far[itr].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
				bearer->s1u_enb_gtpu_ipv4.s_addr;
			update_far[itr].upd_frwdng_parms.dst_intfc.interface_value =
				GTPV2C_IFTYPE_S1U_ENODEB_GTPU;
			update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		}

		sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &rel_acc_ber_req_t->header,
				 bearer, pdn, update_far, 0);

		if(pfcp_sess_mod_req.update_far_count) {
			for(int itr=0; itr < pfcp_sess_mod_req.update_far_count; itr++ ){
				pfcp_sess_mod_req.update_far[itr].apply_action.forw = 0;
				pfcp_sess_mod_req.update_far[itr].apply_action.buff = PRESENT;
				if (pfcp_sess_mod_req.update_far[itr].apply_action.buff == PRESENT) {
					pfcp_sess_mod_req.update_far[itr].apply_action.nocp = PRESENT;
					pfcp_sess_mod_req.update_far[itr].upd_frwdng_parms.outer_hdr_creation.teid = 0;
				}
			}
		}

#if 0
		if (get_sess_entry((rel_acc_ber_req_t->context)->pdns[ebi_index]->seid, &resp) != 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s %s %d Failed to add response in entry in SM_HASH\n",__file__,
					__func__, __LINE__);
			return -1;
		}


		/* Store s11 struture data into sm_hash for sending response back to s11 */
		resp->msg_type = GTP_RELEASE_ACCESS_BEARERS_REQ;
		resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
		resp->proc = proc;
#endif

		uint8_t pfcp_msg[sizeof(pfcp_sess_mod_req_t)]={0};
		int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);

		pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
		header->message_len = htons(encoded - 4);

		if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &ue_context->upf_context->upf_sockaddr) < 0 )
        {
			clLog(sxlogger, eCLSeverityCritical,"Error sending: %i\n",errno);
            return -1;
        }
        update_cli_stats((uint32_t)ue_context->upf_context->upf_sockaddr.sin_addr.s_addr,
                pfcp_sess_mod_req.header.message_type,SENT,SX);


        transData_t *trans_entry;
        trans_entry = start_pfcp_session_timer(ue_context, pfcp_msg, encoded, process_release_access_bearer_request_pfcp_timeout);

        /* add transaction into transaction table */
        add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
        trans_entry->sequence = sequence;
        /* link proc & transaction */
        rab_proc->pfcp_trans = trans_entry;
        trans_entry->proc_context = (void *)rab_proc;

		/* Update UE State */
		pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	}
	return 0;
}

void 
process_rab_proc_pfcp_mod_sess_rsp(msg_info_t *msg)
{
    int ret = 0;
	uint16_t payload_length = 0;
    ue_context_t *ue_context = msg->ue_context; 
    proc_context_t *proc_context = msg->proc_context;
    transData_t *gtpc_trans = proc_context->gtpc_trans;


	/*Validate the modification is accepted or not. */
	if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
			clLog(sxlogger, eCLSeverityDebug, "Cause received PFCP Modify response is %d\n",
					msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value);
			proc_rab_failed(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
			return;
	}

	/* Retrive the session information based on session id. */
    ue_context_t *temp_context = NULL; 
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
				&temp_context) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u,"
				"Sess ID:%lu, n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);

		proc_rab_failed(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
		return ;
	}
    assert(temp_context == ue_context);

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_rab_pfcp_sess_mod_resp(msg->proc_context,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
		    clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
			proc_rab_failed(msg, ret); 
		return;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
			(struct sockaddr *) &gtpc_trans->peer_sockaddr,
			sizeof(struct sockaddr_in));

	update_cli_stats(gtpc_trans->peer_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type,ACC,S11);

    proc_rab_complete(proc_context);
	return;
}

uint8_t
process_rab_pfcp_sess_mod_resp(proc_context_t *proc_context, 
                               uint64_t sess_id, 
                               gtpv2c_header_t *gtpv2c_tx)
{
    RTE_SET_USED(gtpv2c_tx);
    int ret = 0;
    uint8_t ebi_index = 0;
    eps_bearer_t *bearer  = NULL;
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;
    uint32_t teid = UE_SESS_ID(sess_id);

    /* Retrive the session information based on session id. */
    if (get_sess_entry(sess_id, &context) != 0){
        clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
                __func__, __LINE__, sess_id);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    assert(proc_context->ue_context == context); // just verification 

    /* Retrieve the UE context */
    ret = get_ue_context(teid, &context);
    if (ret < 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
                __func__, __LINE__,
                teid);
    }

    assert(proc_context->ue_context == context);
    ebi_index = UE_BEAR_ID(sess_id) - 5;
    bearer = context->eps_bearers[ebi_index];
    /* Update the UE state */
    pdn = GET_PDN(context, ebi_index);

    pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

    if (!bearer) {
        clLog(clSystemLog, eCLSeverityCritical,
                "%s:%d Retrive modify bearer context but EBI is non-existent- "
                "Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    printf("Send RAB response\n");
    transData_t *gtpc_trans = proc_context->gtpc_trans;
    /* Fill the release bearer response */
    set_release_access_bearer_response(gtpv2c_tx,
            gtpc_trans->sequence, context->s11_mme_gtpc_teid);

    /* Update the UE state */
    pdn->state = IDEL_STATE;

    clLog(sxlogger, eCLSeverityDebug, "%s:%d Sent RAB response to peer : %s\n",
            __func__, __LINE__,
            inet_ntoa(*((struct in_addr *)&gtpc_trans->peer_sockaddr.sin_addr.s_addr)));

    return 0;
}

void 
proc_rab_failed(msg_info_t *msg, uint8_t cause )
{
    proc_context_t *proc_context = msg->proc_context;

    rab_error_response(msg,
                       cause,
                       cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    proc_rab_complete(proc_context);
}

/* should be called after sending RAB response */
void 
proc_rab_complete(proc_context_t *proc_context)
{
    ue_context_t *ue_context = proc_context->ue_context; 
    transData_t *trans_rec = proc_context->gtpc_trans;

    uint16_t port_num = trans_rec->peer_sockaddr.sin_port; 
    uint32_t sender_addr = trans_rec->peer_sockaddr.sin_addr.s_addr; 
    uint32_t seq_num = trans_rec->sequence; 
    transData_t *temp_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
    assert(temp_trans != NULL);

    /* Let's cross check if transaction from the table is matchig with the one we have 
     * in subscriber 
     */
    assert(proc_context->gtpc_trans == temp_trans);
    proc_context->gtpc_trans =  NULL;

    /* PFCP transaction is already complete. */
    assert(proc_context->pfcp_trans == NULL);
    free(temp_trans);
    proc_context->gtpc_trans = NULL;
    free(proc_context);
    ue_context->current_proc = NULL;
}
