
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "rte_errno.h"
#include "pfcp.h"
#include "gx_interface.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_session.h"
#include "cp_config.h"
#include "cp_log.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "proc_detach.h"
#include "pfcp_messages_encoder.h"
#include "cp_transactions.h"
#include "gtp_ies.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_detach_proc(msg_info_t *msg)
{
    proc_context_t *detach_proc = calloc(1, sizeof(proc_context_t));
    detach_proc->proc_type = msg->proc; 
    detach_proc->handler = detach_event_handler;
    detach_proc->ue_context = msg->ue_context;
    detach_proc->pdn_context = msg->pdn_context;
    msg->proc_context = detach_proc;
    SET_PROC_MSG(detach_proc, msg);
    return detach_proc;
}

void 
detach_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case DS_REQ_RCVD_EVNT: {
            process_ds_req_handler(proc_context, msg);
            break;
        } 
        case CCA_RCVD_EVNT: {
        }
        case PFCP_SESS_DEL_RESP_RCVD_EVNT: {
            process_sess_del_resp_handler(proc_context, msg);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

// saegw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT => process_ds_req_handler
// saegw - DETACH_PROC IDEL_STATE  DS_REQ_RCVD_EVNT => process_ds_req_handler
// pgw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler
// pgw - DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler 
// sgw DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 
// sgw DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 
int
process_ds_req_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
    gtpv2c_header_t *dsr_header = &msg->gtpc_msg.dsr.header;
	LOG_MSG(LOG_DEBUG, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					dsr_header->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));


	if (cp_config->cp_type == SGWC && msg->gtpc_msg.dsr.indctn_flgs.indication_oi == 1) {
		/* Indication flag 1 mean dsr needs to be sent to PGW otherwise dont send it to PGW */
		ret = process_sgwc_delete_session_request(proc_context, msg);
	} else {
		ret = process_pfcp_sess_del_request(proc_context, msg);
	}

	if (ret) {
		if(ret != -1) {
            proc_detach_failure(proc_context, msg, ret);
        }
		return ret;
	}

	return 0;
}

int
process_sess_del_resp_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	uint16_t payload_length = 0;
	uint16_t msglen = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
    pfcp_sess_del_rsp_t *pfcp_sess_del_resp = &msg->pfcp_msg.pfcp_sess_del_resp;
    

	LOG_MSG(LOG_DEBUG, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_DELETION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			pfcp_sess_del_resp->header.seid_seqno.has_seid.seid,
			get_proc_string(proc_context->proc_type),
			get_state_string(msg->state), get_event_string(msg->event));


	if(pfcp_sess_del_resp->cause.cause_value != REQUESTACCEPTED) {
		LOG_MSG(LOG_ERROR, "Cause received in pfcp delete response is %d\n",
				pfcp_sess_del_resp->cause.cause_value);

		proc_detach_failure(proc_context, msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
		return -1;
	}

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

    ret = process_pfcp_sess_del_resp(
            pfcp_sess_del_resp->header.seid_seqno.has_seid.seid,
            gtpv2c_tx, &ccr_request, &msglen, proc_context);

	if (ret) {
		LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n", __func__, __LINE__, ret);
		proc_detach_failure(proc_context, msg, ret); 
		return -1;
	}


    /* Lookup value in hash using session id and fill pfcp response and delete entry from hash*/
    if((cp_config->cp_type != SGWC ) &&  (cp_config->gx_enabled)) {
        buffer = rte_zmalloc_socket(NULL, msglen + sizeof(ccr_request.msg_type),
                RTE_CACHE_LINE_SIZE, rte_socket_id());
        if (buffer == NULL) {
            LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory"
                    "structure: %s (%s:%d)\n", rte_strerror(rte_errno),
                    __FILE__, __LINE__);
            return -1;
        }

        memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

        if (gx_ccr_pack(&(ccr_request.data.ccr),
                    (unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msglen) == 0) {
            LOG_MSG(LOG_ERROR, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
            return -1;
        }
		gx_send(my_sock.gx_app_sock, buffer,
				msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
        struct sockaddr_in saddr_in;
        saddr_in.sin_family = AF_INET;
        inet_aton("127.0.0.1", &(saddr_in.sin_addr));
        increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T, saddr_in.sin_addr.s_addr);
    }


	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

	if ((cp_config->cp_type == PGWC) ) {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
        struct sockaddr_in peer_addr = gtpc_trans->peer_sockaddr; 
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		        sizeof(struct sockaddr_in));

        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_DSRSP, peer_addr.sin_addr.s_addr);
        decrement_stat(NUM_UE_PGW_ACTIVE_SUBSCRIBERS);
        increment_stat(PROCEDURES_PGW_MME_INIT_DETACH_SUCCESS);
	} else {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
        struct sockaddr_in peer_addr = gtpc_trans->peer_sockaddr; 
		/* Send response on s11 interface */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &peer_addr,
				sizeof(struct sockaddr_in));

		/*CLI:CSResp sent cnt*/
        increment_mme_peer_stats(MSG_TX_GTPV2_S11_DSRSP, peer_addr.sin_addr.s_addr);
        decrement_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
        increment_stat(PROCEDURES_SPGW_MME_INIT_DETACH_SUCCESS);

	}
    proc_detach_complete(proc_context, msg);
	return 0;
}

void
process_pfcp_sess_del_request_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    assert(proc_context->gtpc_trans != NULL);
    assert(proc_context->pfcp_trans != NULL);

    LOG_MSG(LOG_ERROR, "PFCP Session delete timeout ");
    msg_info_t *msg = calloc(1, sizeof(msg_info_t));
    msg->msg_type = PFCP_SESSION_DELETION_RESPONSE;
    msg->proc_context = proc_context;

    proc_detach_failure(proc_context, msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_pfcp_sess_del_request(proc_context_t *proc_context, msg_info_t *msg) 
{

	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t s5s8_pgw_gtpc_teid = 0;
	uint32_t s5s8_pgw_gtpc_ipv4 = 0;
	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
    del_sess_req_t *ds_req = &msg->gtpc_msg.dsr;
	uint64_t ebi_index = ds_req->lbi.ebi_ebi - 5;
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
	uint8_t pfcp_msg[512]={0};

    pdn = proc_context->pdn_context;

	/* Lookup and get context of delete request */
	ret = delete_context(ds_req->lbi, ds_req->header.teid.has_teid.teid,
		&context, &s5s8_pgw_gtpc_teid, &s5s8_pgw_gtpc_ipv4);
	if (ret) {
		return ret;
    }

    assert(proc_context->ue_context == context);

	pdn = GET_PDN(context, ebi_index);

	/* Fill pfcp structure for pfcp delete request and send it */
	sequence = fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = pdn->dp_seid;


	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSDELREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, process_pfcp_sess_del_request_timeout);
    trans_entry->self_initiated = 1;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;

    // link new transaction and proc context 
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;
    assert(proc_context->gtpc_trans != NULL);
    assert(proc_context->pfcp_trans != NULL);

	return 0;
}

// case 1: failed to delete context locally for whatever reason or failed to encode pfcp message  
// case 2: Received pfcp del response with -ve casue 
// case 3: failed to process pfcp del response 
// case 3: PFCP delete message timedout 
void
proc_detach_failure(proc_context_t *proc_context, msg_info_t *msg, uint8_t cause)
{
    increment_stat(PROCEDURES_SPGW_MME_INIT_DETACH_FAILURE);

    ds_error_response(proc_context, msg,
                      cause,
                      cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    proc_detach_complete(proc_context, msg);
}

/* free transactions. delink from procedure. Free procedure and de-link from subscriber */
void 
proc_detach_complete(proc_context_t *proc_context, msg_info_t *msg)
{
    RTE_SET_USED(msg);
    end_procedure(proc_context);
    return;
}

void process_spgwc_delete_session_request_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_sgwc_delete_session_request(proc_context_t *proc_context, msg_info_t *msg)
{
    RTE_SET_USED(proc_context);
	ue_context_t *context = msg->ue_context;
	pdn_connection_t *pdn =  msg->pdn_context;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    del_sess_req_t *del_req = &msg->gtpc_msg.dsr;

	fill_pfcp_sess_mod_req_delete(&pfcp_sess_mod_req, &del_req->header, context, pdn);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_spgwc_delete_session_request_timeout);
    pdn->trans_entry = trans_entry;

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

#ifdef TRANS_SUPPORT
	uint8_t ebi_index = 0;
	/* Update the sequence number */
	 context->sequence = del_req->header.teid.has_teid.seq;


	/*Retrive the session information based on session id. */
	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu\n", context->pdns[ebi_index]->seid);
		return -1;
	}

	resp->gtpc_msg.dsr = *del_req;
	resp->eps_bearer_id = del_req->lbi.ebi_ebi;
	resp->s5s8_pgw_gtpc_ipv4 = htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);
	resp->msg_type = GTP_DELETE_SESSION_REQ;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = pdn->proc;
#endif

	return 0;
}

int8_t
process_pfcp_sess_del_resp(uint64_t sess_id, 
                           gtpv2c_header_t *gtpv2c_tx,
		                   gx_msg *ccr_request, 
                           uint16_t *msglen,
                           proc_context_t *proc_context )
{
    RTE_SET_USED(sess_id);
	int ret = 0;
	uint8_t ebi_index = 0;
	uint16_t msg_len = 0;
	ue_context_t *context = proc_context->ue_context;
	del_sess_rsp_t del_resp = {0};
	pdn_connection_t *pdn =  NULL;

	pdn = proc_context->pdn_context; 
    ebi_index = pdn->default_bearer_id - 5;

	/* Update the UE state */
	pdn->state = PFCP_SESS_DEL_RESP_RCVD_STATE;
	if ((cp_config->gx_enabled) && (cp_config->cp_type != SGWC)) {

		gx_context_t *gx_context = NULL;

		/* Retrive Gx_context based on Sess ID. */
		ret = get_gx_context((uint8_t *)pdn->gx_sess_id,&gx_context);
		if (ret < 0) {
			LOG_MSG(LOG_ERROR, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
					pdn->gx_sess_id);
			return -1;
		}

		/* VS: Set the Msg header type for CCR-T */
		ccr_request->msg_type = GX_CCR_MSG ;

		/* VS: Set Credit Control Request type */
		ccr_request->data.ccr.presence.cc_request_type = PRESENT;
		ccr_request->data.ccr.cc_request_type = TERMINATION_REQUEST ;

		/* VG: Set Credit Control Bearer opertaion type */
		ccr_request->data.ccr.presence.bearer_operation = PRESENT;
		ccr_request->data.ccr.bearer_operation = TERMINATION ;

		/* VS: Fill the Credit Crontrol Request to send PCRF */
		if(fill_ccr_request(&ccr_request->data.ccr, context, ebi_index, pdn->gx_sess_id) != 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
			return -1;
		}
		/* Update UE State */
		pdn->state = CCR_SNT_STATE;

		/* VS: Set the Gx State for events */
		gx_context->state = CCR_SNT_STATE;
		gx_context->proc = proc_context->proc_type;

		/* VS: Calculate the max size of CCR msg to allocate the buffer */
		*msglen = gx_ccr_calc_length(&ccr_request->data.ccr);

	}

    /* Block to create DSRSP message */
    {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 

        if(cp_config->cp_type == PGWC) {
            /* Fill gtpv2c structure for sending on s11 interface */
            set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
                    pdn->s5s8_sgw_gtpc_teid, gtpc_trans->sequence);
        } else {
            /* Fill gtpv2c structure for sending on s11 interface */
            set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
                    context->s11_mme_gtpc_teid, gtpc_trans->sequence);
        }

        set_cause_accepted(&del_resp.cause, IE_INSTANCE_ZERO);

        /*Encode the S11 delete session response message. */
        msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);

        gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
    }
	return 0;
}

void
fill_pfcp_sess_mod_req_delete( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, ue_context_t *context, pdn_connection_t *pdn)
{
    RTE_SET_USED(header);
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer;

	RTE_SET_USED(context);  /* NK:to be checked */

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		LOG_MSG(LOG_ERROR, "%s : Error: %d \n", __func__, ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	/*SP: Adding FAR IE*/
	pfcp_sess_mod_req->update_far_count = 0;
	for (int index = 0; index < pdn->num_bearer; index++){
		bearer = pdn->eps_bearers[index];
		if(bearer){
			for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
	}
		switch (cp_config->cp_type)
		{
			case SGWC :
				if(pfcp_sess_mod_req->update_far_count){
					for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count; itr1++) {
						pfcp_sess_mod_req->update_far[itr1].apply_action.drop = PRESENT;
					}
				}
				break;

			default :
				LOG_MSG(LOG_DEBUG,"default pfcp sess mod req\n");
				break;
		}
	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}
}
