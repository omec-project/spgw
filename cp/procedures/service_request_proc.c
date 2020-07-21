
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_common.h"
#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "gtpv2_evt_handler.h"
#include "gtpv2c_error_rsp.h"
#include "assert.h"
#include "clogger.h"
#include "cp_peer.h"
#include "gw_adapter.h"
#include "gtpv2_interface.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
#include "cp_config_apis.h"
#include "ip_pool.h"
#include "gen_utils.h"
#include "pfcp_cp_session.h"
#include "pfcp_enum.h"
#include "pfcp.h"
#include "pfcp_cp_association.h"
#include "cp_log.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "service_request_proc.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_encoder.h"
#include "gtpv2c_set_ie.h"


extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern udp_sock_t my_sock;
#define size sizeof(pfcp_sess_mod_req_t)
extern struct sockaddr_in s11_mme_sockaddr;
extern socklen_t s11_mme_sockaddr_len;


proc_context_t*
alloc_service_req_proc(msg_info_t *msg)
{
    proc_context_t *service_req_proc;

    service_req_proc = calloc(1, sizeof(proc_context_t));
    service_req_proc->proc_type = msg->proc; 
    service_req_proc->ue_context = (void *)msg->ue_context;
    service_req_proc->pdn_context = (void *)msg->pdn_context; 

    service_req_proc->handler = service_req_event_handler;

    // set cross references in msg 
    msg->proc_context = service_req_proc;

    return service_req_proc;
}

void 
service_req_event_handler(void *proc, uint32_t event, void *data)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    RTE_SET_USED(proc_context);

    switch(event) {
        case MB_REQ_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            process_mb_req_handler(msg, NULL);
            break;
        } 
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            process_service_request_pfcp_mod_sess_rsp(msg);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

int
process_mb_req_handler(void *data, void *unused_param)
{

    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
    gtpv2c_header_t *mbr_header = &msg->gtpc_msg.mbr.header;

    clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
            "Msg_Type:%s[%u], Teid:%u, "
            "Procedure:%s, State:%s, Event:%s\n",
            __func__, gtp_type_str(msg->msg_type), msg->msg_type,
            mbr_header->teid.has_teid.teid,
            get_proc_string(msg->proc),
            get_state_string(msg->state), get_event_string(msg->event));

	ret = process_pfcp_sess_mod_request(&msg->gtpc_msg.mbr);
	if (ret != 0) {
		if(ret != -1)
			mbr_error_response(msg, ret,
					cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

void
process_pfcp_sess_mod_request_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_pfcp_sess_mod_request(mod_bearer_req_t *mb_req)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

	ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
			(const void *) &mb_req->header.teid.has_teid.teid,
			(void **) &context);

	if (ret < 0 || !context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	if (!mb_req->bearer_contexts_to_be_modified.eps_bearer_id.header.len
			|| !mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Dropping packet\n",
				__func__, __LINE__);
		return GTPV2C_CAUSE_INVALID_LENGTH;
	}

	ebi_index = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s:%d Received modify bearer on non-existent EBI - "
				"Dropping packet\n", __func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s:%d Received modify bearer on non-existent EBI - "
				"Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	pdn = bearer->pdn;
	if(mb_req->ue_time_zone.header.len)
	{
		if((mb_req->ue_time_zone.time_zone != pdn->ue_tz.tz) ||
				(mb_req->ue_time_zone.daylt_svng_time != pdn->ue_tz.dst))
		{
			pdn->old_ue_tz = pdn->ue_tz;
			pdn->old_ue_tz_valid = true;
			pdn->ue_tz.tz = mb_req->ue_time_zone.time_zone;
			pdn->ue_tz.dst = mb_req->ue_time_zone.daylt_svng_time;
		}
	}

	/* TODO something with modify_bearer_request.delay if set */

	if (mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.header.len &&
			(context->s11_mme_gtpc_teid != mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key))
		context->s11_mme_gtpc_teid = mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key;

	bearer->eps_bearer_id = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];
	pfcp_sess_mod_req.update_far_count = 0;
	uint8_t x2_handover = 0;

	if (mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len  != 0){

		if(bearer->s1u_enb_gtpu_ipv4.s_addr != 0) {
			if((mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.teid_gre_key)
					!= bearer->s1u_enb_gtpu_teid  ||
					(mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.ipv4_address) !=
					bearer->s1u_enb_gtpu_ipv4.s_addr) {

				x2_handover = 1;
			}
		}

		/* Bug 370. No need to send end marker packet in DDN */
		if (CONN_SUSPEND_PROC == pdn->proc) {
			x2_handover = 0;
		}

		bearer->s1u_enb_gtpu_ipv4.s_addr =
			mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.ipv4_address;
		bearer->s1u_enb_gtpu_teid =
			mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.teid_gre_key;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s1u_enb_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s1u_enb_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.interface_type);
		update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		pfcp_sess_mod_req.update_far_count++;

	}

	if (mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.header.len  != 0){
		bearer->s5s8_sgw_gtpu_ipv4.s_addr =
			mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.ipv4_address;
		bearer->s5s8_sgw_gtpu_teid =
			mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.teid_gre_key;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s5s8_sgw_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s5s8_sgw_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.interface_type);
		if ( cp_config->cp_type != PGWC) {
			update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		}
		pfcp_sess_mod_req.update_far_count++;
	}

    // MUST CLEAN - why we are updating seid on the fly ?
	context->pdns[ebi_index]->seid = SESS_ID(context->s11_sgw_gtpc_teid, bearer->eps_bearer_id);

	sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &mb_req->header, bearer, pdn, update_far, x2_handover);

	uint8_t pfcp_msg[size]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ){
		clLog(clSystemLog, eCLSeverityDebug,"Error sending: %i\n",errno);
        return GTPV2C_CAUSE_SYSTEM_FAILURE;
	} 
    update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
            pfcp_sess_mod_req.header.message_type,SENT,SX);
    transData_t *trans_entry;
    trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pfcp_sess_mod_request_timeout);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  

    proc_context_t *proc_context = context->current_proc;
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

void 
process_service_request_pfcp_mod_sess_rsp(msg_info_t *msg)
{
    ue_context_t *context = NULL;
    int ret = 0;
	uint16_t payload_length = 0;

	/*Validate the modification is accepted or not. */
	if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
			clLog(sxlogger, eCLSeverityDebug, "Cause received Modify response is %d\n",
					msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value);
			mbr_error_response(msg,
							  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
							  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			return ;
	}
	/* Retrive the session information based on session id. */
	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
				&context) != 0) {
		mbr_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u,"
				"Sess ID:%lu, n",
				__func__, msg->msg_type,
				msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return;
	}
    assert(context == msg->proc_context->ue_context);

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));


	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_srreq_pfcp_sess_mod_resp(msg->proc_context, 
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
			mbr_error_response(msg, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
			(struct sockaddr *) &s11_mme_sockaddr,
			s11_mme_sockaddr_len);

	update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type,ACC,S11);

    ue_context_t *ue_context = msg->ue_context; 
    proc_context_t *proc_context = ue_context->current_proc;
    assert(proc_context->gtpc_trans != NULL);

    uint16_t port_num = s11_mme_sockaddr.sin_port; 
    uint32_t sender_addr = s11_mme_sockaddr.sin_addr.s_addr; 
    uint32_t seq_num = proc_context->gtpc_trans->sequence; 

    transData_t *gtpc_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
    assert(gtpc_trans != NULL);

    /* Let's cross check if transaction from the table is matchig with the one we have 
     * in subscriber 
     */
    assert(proc_context->gtpc_trans == gtpc_trans);
    proc_context->gtpc_trans =  NULL;

    /* PFCP transaction is already complete. */
    assert(proc_context->pfcp_trans == NULL);

    free(gtpc_trans);
    free(proc_context);
    ue_context->current_proc = NULL;

	return;
}

uint8_t
process_srreq_pfcp_sess_mod_resp(proc_context_t *proc_context, 
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

    assert(proc_context->ue_context == context);

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

	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s:%d Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Update the UE state */
	pdn = GET_PDN(context, ebi_index);
    assert(proc_context->pdn_context == pdn);

	pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

    transData_t *gtpc_trans = proc_context->gtpc_trans;
    uint32_t sequence = gtpc_trans->sequence;
    /* Fill the modify bearer response */
    set_modify_bearer_response(gtpv2c_tx,
            sequence, context, bearer);

    /* Update the UE state */
    pdn->state = CONNECTED_STATE;

    /* Update the next hop IP address */
    if (PGWC != cp_config->cp_type) {
        s11_mme_sockaddr.sin_addr.s_addr =
            htonl(context->s11_mme_gtpc_ipv4.s_addr);
    }
    return 0;
}

