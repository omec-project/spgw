
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "pfcp.h"
#include "sm_structs_api.h"
#include "cp_config.h"
#include "sm_struct.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "gtpv2_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "vepc_cp_dp_api.h"
#include "pfcp_cp_util.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "cp_transactions.h"
#include "spgw_cpp_wrapper.h"
#include "proc_s1_release.h"
#include "gtpv2_error_rsp.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "assert.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_rab_proc(msg_info_t *msg)
{
    proc_context_t *rab_proc = calloc(1, sizeof(proc_context_t));
    rab_proc->proc_type = msg->proc; 
    rab_proc->handler = rab_event_handler;
    rab_proc->ue_context = msg->ue_context;
    rab_proc->pdn_context = msg->pdn_context;
    msg->proc_context = rab_proc;
    SET_PROC_MSG(rab_proc, msg);
    return rab_proc;
}

void 
rab_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case REL_ACC_BER_REQ_RCVD_EVNT: {
            process_rel_access_ber_req_handler(proc_context, msg);
            break;
        } 
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            process_rab_proc_pfcp_mod_sess_rsp(proc_context, msg);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

int
process_rel_access_ber_req_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
    gtpv2c_header_t *rab_header = &msg->gtpc_msg.rab.header;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:%s[%u], Teid:%u, "
			"Procedure:%s, State:%s, Event:%s",
			gtp_type_str(msg->msg_type), msg->msg_type,
			rab_header->teid.has_teid.teid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));


	/* TODO: Check return type and do further processing */
	ret = process_release_access_bearer_request(proc_context, msg); 
    if (ret) {
        LOG_MSG(LOG_ERROR, "Error: %d ", ret);
        return -1;
    }

	return 0;
}

void 
process_release_access_bearer_request_pfcp_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    msg_info_t *msg = calloc(1, sizeof(msg_info_t));
    msg->msg_type = PFCP_SESSION_MODIFICATION_RESPONSE;
    msg->proc_context = proc_context;
    SET_PROC_MSG(proc_context, msg);
    proc_rab_failed(msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_release_access_bearer_request(proc_context_t *rab_proc, msg_info_t *msg) 
{
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	uint32_t sequence = 0;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    ue_context_t *ue_context = msg->ue_context;
    rel_acc_bearer_req_t *rel_acc_ber_req_t = &msg->gtpc_msg.rab;

	for (int i = 0; i < MAX_BEARERS; ++i) {
		if (ue_context->eps_bearers[i] == NULL)
			continue;

		bearer = ue_context->eps_bearers[ebi_index];
		if (!bearer) {
			LOG_MSG(LOG_ERROR,
					"Retrive Context for release access bearer is non-existent EBI - "
					"Bitmap Inconsistency - Dropping packet");
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
		if (get_sess_entry_seid((rel_acc_ber_req_t->context)->pdns[ebi_index]->seid, &resp) != 0) {
			LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH");
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

		pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &ue_context->upf_context->upf_sockaddr);

        increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(ue_context->upf_context));

        transData_t *trans_entry;
        trans_entry = start_response_wait_timer(rab_proc, pfcp_msg, encoded, process_release_access_bearer_request_pfcp_timeout);

        trans_entry->self_initiated = 1;
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
process_rab_proc_pfcp_mod_sess_rsp(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	uint16_t payload_length = 0;
    transData_t *gtpc_trans = proc_context->gtpc_trans;
    ue_context_t *ue_context = proc_context->ue_context; 


	/*Validate the modification is accepted or not. */
	if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
			LOG_MSG(LOG_DEBUG, "Cause received PFCP Modify response is %d",
					msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value);
			proc_rab_failed(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
			return;
	}

	/* Retrive the session information based on session id. */
    ue_context_t *temp_context = NULL; 
	if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
				&temp_context) != 0) {
		LOG_MSG(LOG_ERROR, "Session entry not found Msg_Type:%u,"
				"Sess ID:%lu ",
				msg->msg_type,
				msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);

		proc_rab_failed(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
		return ;
	}
    assert(temp_context == ue_context);

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s",
			msg->msg_type,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_rab_pfcp_sess_mod_resp(proc_context,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
		    LOG_MSG(LOG_ERROR, "Error: %d ", ret);
			proc_rab_failed(msg, ret); 
		return;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
			(struct sockaddr *) &gtpc_trans->peer_sockaddr,
			sizeof(struct sockaddr_in));

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_RABRSP,gtpc_trans->peer_sockaddr.sin_addr.s_addr);

    increment_stat(PROCEDURES_SPGW_S1_RELEASE_SUCCESS);
    proc_rab_complete(proc_context);
	return;
}

uint8_t
process_rab_pfcp_sess_mod_resp(proc_context_t *proc_context, 
                               uint64_t sess_id, 
                               gtpv2c_header_t *gtpv2c_tx)
{
    int ret = 0;
    uint8_t ebi_index = 0;
    eps_bearer_t *bearer  = NULL;
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;
    uint32_t teid = UE_SESS_ID(sess_id);

    /* Retrive the session information based on session id. */
    if (get_sess_entry_seid(sess_id, &context) != 0){
        LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", sess_id);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    assert(proc_context->ue_context == context); // just verification 

    /* Retrieve the UE context */
    ret = get_ue_context(teid, &context);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
    }

    assert(proc_context->ue_context == context);
    ebi_index = UE_BEAR_ID(sess_id) - 5;
    bearer = context->eps_bearers[ebi_index];
    /* Update the UE state */
    pdn = GET_PDN(context, ebi_index);

    pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

    if (!bearer) {
        LOG_MSG(LOG_ERROR,
                "Retrive modify bearer context but EBI is non-existent- "
                "Bitmap Inconsistency - Dropping packet");
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    LOG_MSG(LOG_DEBUG2, "Send RAB response");
    transData_t *gtpc_trans = proc_context->gtpc_trans;
    /* Fill the release bearer response */
    set_release_access_bearer_response(gtpv2c_tx,
            gtpc_trans->sequence, context->s11_mme_gtpc_teid);

    /* Update the UE state */
    pdn->state = IDEL_STATE;

    LOG_MSG(LOG_DEBUG, "Sent RAB response to peer : %s",
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

    
    increment_stat(PROCEDURES_SPGW_S1_RELEASE_FAILURE);
    proc_rab_complete(proc_context);
}

/* should be called after sending RAB response */
void 
proc_rab_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
}
