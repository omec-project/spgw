
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
#include "clogger.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "detach_proc.h"
#include "pfcp_messages_encoder.h"
#include "cp_transactions.h"
#include "gtp_ies.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"

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
    return detach_proc;
}

void 
detach_event_handler(void *proc, uint32_t event, void *data)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    RTE_SET_USED(proc_context);

    switch(event) {
        case DS_REQ_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            process_ds_req_handler(msg, NULL);
            break;
        } 
        case PFCP_SESS_DEL_RESP_RCVD_EVNT: {
            process_sess_del_resp_handler(data, NULL);
            break;
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

int
process_ds_req_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
    gtpv2c_header_t *dsr_header = &msg->gtpc_msg.dsr.header;
	clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Teid:%u, "
					"Procedure:%s, State:%s, Event:%s\n",
					__func__, gtp_type_str(msg->msg_type), msg->msg_type,
					dsr_header->teid.has_teid.teid,
					get_proc_string(msg->proc),
					get_state_string(msg->state), get_event_string(msg->event));


#ifdef FUTURE_NEED
	if (cp_config->cp_type == SGWC && msg->gtpc_msg.dsr.indctn_flgs.indication_oi == 1) {
		/* Indication flag 1 mean dsr needs to be sent to PGW otherwise dont send it to PGW */
		ret = process_sgwc_delete_session_request(msg, &msg->gtpc_msg.dsr);
	} else 
#endif
    {
		ret = process_pfcp_sess_del_request(msg, &msg->gtpc_msg.dsr);
	}

	if (ret) {
		if(ret != -1) {
            proc_detach_failure(msg, ret);
        }
		return ret;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_sess_del_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	uint16_t msglen = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
    proc_context_t *proc_context = msg->proc_context;
    

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_DELETION_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(proc_context->proc_type),
			get_state_string(msg->state), get_event_string(msg->event));


	if(msg->pfcp_msg.pfcp_sess_del_resp.cause.cause_value != REQUESTACCEPTED) {

		clLog(sxlogger, eCLSeverityCritical, "Cause received Del response is %d\n",
				msg->pfcp_msg.pfcp_sess_del_resp.cause.cause_value);

		proc_detach_failure(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
		return -1;
	}

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

    if( cp_config->cp_type != SGWC ) {
        /* Lookup value in hash using session id and fill pfcp response and delete entry from hash*/
        if(cp_config->gx_enabled) {
            ret = process_pfcp_sess_del_resp(
                    msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
                    gtpv2c_tx, &ccr_request, &msglen, proc_context);

            buffer = rte_zmalloc_socket(NULL, msglen + sizeof(ccr_request.msg_type),
                    RTE_CACHE_LINE_SIZE, rte_socket_id());
            if (buffer == NULL) {
                clLog(sxlogger, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
                        "structure: %s (%s:%d)\n",
                        rte_strerror(rte_errno),
                        __FILE__,
                        __LINE__);
                return -1;
            }

            memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

            if (gx_ccr_pack(&(ccr_request.data.ccr),
                        (unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msglen) == 0) {
                clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
                return -1;
            }
        } else {
            ret = process_pfcp_sess_del_resp(
                    msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
                    gtpv2c_tx, NULL, NULL, proc_context);

        }
    }  else {
		/**/
		ret = process_pfcp_sess_del_resp(
				msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
				gtpv2c_tx, NULL, NULL, proc_context);
	}

	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);

		proc_detach_failure(msg, ret); 
		return -1;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

#ifdef FUTURE_NEED
	if ((cp_config->cp_type == PGWC) ) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
		        sizeof(struct sockaddr_in));

        increment_sgw_peer_stat(MSG_TX_GTPV2_S5S8_DSRSP, peer_addr.sin_addr.s_addr);
        decrement_stat(NUM_UE_PGW_ACTIVE_SUBSCRIBERS);
		//s5s8_sgwc_msgcnt++;
	} 
    else 
#endif
    {

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
        proc_detach_complete(msg);

	}
	/* VS: Write or Send CCR -T msg to Gx_App */
	if ((cp_config->gx_enabled) && 
       (cp_config->cp_type != SGWC)) {
		send_to_ipc_channel(my_sock.gx_app_sock, buffer,
				msglen + sizeof(ccr_request.msg_type));
	}

#ifdef FUTURE_NEED
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T, saddr_in.sin_addr.s_addr);
#endif

	RTE_SET_USED(unused_param);
	return 0;
}

void
process_pfcp_sess_del_request_timeout(void *data)
{
    RTE_SET_USED(data);
    ue_context_t *ue_context = (ue_context_t *)data;
    proc_context_t *proc_context = ue_context->current_proc;

    msg_info_t msg = {0};
    msg.msg_type = PFCP_SESSION_DELETION_RESPONSE;
    msg.proc_context = proc_context;

    proc_detach_failure(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_pfcp_sess_del_request(msg_info_t *msg, del_sess_req_t *ds_req)
{

	RTE_SET_USED(msg);  
	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t s5s8_pgw_gtpc_teid = 0;
	uint32_t s5s8_pgw_gtpc_ipv4 = 0;
	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
	uint64_t ebi_index = ds_req->lbi.ebi_ebi - 5;
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
	uint8_t pfcp_msg[512]={0};

    proc_context_t *proc_context = msg->proc_context;

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

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ){
		clLog(clSystemLog, eCLSeverityDebug,"%s:%d Error sending: %i\n", __func__, __LINE__, errno);
        // Just logging is good enough, let timeout & retry take care further 
    } 
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSDELREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);
    transData_t *trans_entry;
    trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pfcp_sess_del_request_timeout);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;

    // link new transaction and proc context 
    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;

#ifdef DELETE_THIS
	/* Lookup entry in hash table on the basis of session id*/
	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, context->pdns[ebi_index]->seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
#endif

	return 0;
}

// case 1: failed to delete context locally for whatever reason or failed to encode pfcp message  
// case 2: Received pfcp del response with -ve casue 
// case 3: failed to process pfcp del response 
// case 3: PFCP delete message timedout 
void
proc_detach_failure(msg_info_t *msg, uint8_t cause)
{
    increment_stat(PROCEDURES_SPGW_MME_INIT_DETACH_FAILURE);
    ds_error_response(msg,
                      cause,
                      cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    proc_detach_complete(msg);
}

/* free transactions. delink from procedure. Free procedure and de-link from subscriber */
void 
proc_detach_complete(msg_info_t *msg)
{
    proc_context_t *proc_context = msg->proc_context;
    transData_t *gtpc_trans = proc_context->gtpc_trans;
 
    /* Cleanup transaction and cross references  */
    transData_t *trans = (transData_t *)delete_gtp_transaction(gtpc_trans->peer_sockaddr.sin_addr.s_addr,         
                                                               gtpc_trans->peer_sockaddr.sin_port,
                                                               gtpc_trans->sequence);
    assert(trans != NULL);
    assert(trans == gtpc_trans);
    gtpc_trans->cb_data = NULL;
    proc_context->gtpc_trans = NULL;
    free(gtpc_trans);
    free(proc_context);
    return;
}

#ifdef FUTURE_NEED
void process_spgwc_delete_session_request_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_sgwc_delete_session_request(msg_info_t *msg, del_sess_req_t *del_req)
{
	uint8_t ebi_index = 0;
	ue_context_t *context = msg->ue_context;
	eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ebi_index = del_req->lbi.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received delete session on non-existent EBI - "
				"Dropping packet\n");
		return -EPERM;
	}

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received delete session on non-existent EBI - "
				"Bitmap Inconsistency - Dropping packet\n");
		return -EPERM;
	}

	pdn = bearer->pdn;

	bearer->eps_bearer_id = del_req->lbi.ebi_ebi;

	fill_pfcp_sess_mod_req_delete(&pfcp_sess_mod_req, &del_req->header, context, pdn);

	uint8_t pfcp_msg[size]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ){
		clLog(clSystemLog, eCLSeverityDebug,"Error sending: %i\n",errno);
	} else {
        increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, context->upf_context->upf_sockaddr.sin_addr.s_addr);
        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_spgwc_delete_session_request_timeout);
        pdn->trans_entry = trans_entry;
	}

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	/* Update the sequence number */
	context->sequence =
		del_req->header.teid.has_teid.seq;

	/*Retrive the session information based on session id. */
	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n", context->pdns[ebi_index]->seid);
		return -1;
	}

	resp->gtpc_msg.dsr = *del_req;
	resp->eps_bearer_id = del_req->lbi.ebi_ebi;
	resp->s5s8_pgw_gtpc_ipv4 = htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);
	resp->msg_type = GTP_DELETE_SESSION_REQ;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = pdn->proc;

	return 0;
}
#endif

int8_t
process_pfcp_sess_del_resp(uint64_t sess_id, 
                           gtpv2c_header_t *gtpv2c_tx,
		                   gx_msg *ccr_request, 
                           uint16_t *msglen,
                           proc_context_t *proc_context )
{
	int ret = 0;
	uint8_t ebi_index = 0;
	uint16_t msg_len = 0;
	ue_context_t *context = NULL;
	del_sess_rsp_t del_resp = {0};
	uint32_t teid = UE_SESS_ID(sess_id);

	//eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	/* Lookup entry in hash table on the basis of session id*/
	if (get_sess_entry_seid(sess_id, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, sess_id);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					teid);
	}


	pdn = proc_context->pdn_context; 
    ebi_index = pdn->default_bearer_id - 5;

	/* Update the UE state */
	pdn->state = PFCP_SESS_DEL_RESP_RCVD_STATE;
	if ((cp_config->gx_enabled) && 
	    (cp_config->cp_type != SGWC)) {

		gx_context_t *gx_context = NULL;

		/* Retrive Gx_context based on Sess ID. */
		ret = get_gx_context((uint8_t *)pdn->gx_sess_id,&gx_context);
		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
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
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
			return -1;
		}
		/* Update UE State */
		pdn->state = CCR_SNT_STATE;

		/* VS: Set the Gx State for events */
		gx_context->state = CCR_SNT_STATE;
		gx_context->proc = pdn->proc;

		/* VS: Calculate the max size of CCR msg to allocate the buffer */
		*msglen = gx_ccr_calc_length(&ccr_request->data.ccr);

	}
#ifdef FUTURE_NEED
    if ( cp_config->cp_type == PGWC) {

        fill_pgwc_ds_sess_rsp(&del_resp, context->sequence,
                pdn->s5s8_sgw_gtpc_teid);

        uint16_t msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);

        gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;
        header->gtpc.message_len = htons(msg_len -4);

        my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
            htonl(context->pdns[ebi_index]->s5s8_sgw_gtpc_ipv4.s_addr);

        /* Delete entry from session entry */
        if (del_sess_entry_seid(sess_id) != 0){
            clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for Key sess ID:%lu\n",
                    __func__, __LINE__, sess_id);
            return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
        }

        clLog(sxlogger, eCLSeverityDebug, "PGWC:%s:%d "
                "s5s8_recv_sockaddr.sin_addr.s_addr :%s\n", __func__, __LINE__,
                inet_ntoa(*((struct in_addr *)&my_sock.s5s8_recv_sockaddr.sin_addr.s_addr)));

        if ( del_rule_entries(context, ebi_index) != 0 ){
            clLog(clSystemLog, eCLSeverityCritical,
                    "%s %s - Error on delete rule entries\n",__file__,
                    strerror(ret));
        }
        ret = delete_sgwc_context(teid, &context, &sess_id);
        if (ret)
            return ret;
        if(context->num_pdns == 0){
            /* Delete UE context entry from UE Hash */
            if (ue_context_delete_entry_imsiKey(context->imsi) < 0){
                clLog(clSystemLog, eCLSeverityCritical,
                        "%s %s - Error on ue_context_by_fteid_hash deletion\n",__file__,
                        strerror(ret));
            }

            /* Delete UPFList entry from UPF Hash */
            if ((context->dns_enable && upflist_by_ue_hash_entry_delete(&context->imsi, sizeof(context->imsi))) < 0){
                clLog(clSystemLog, eCLSeverityCritical,
                        "%s %s - Error on upflist_by_ue_hash deletion of IMSI \n",__file__,
                        strerror(ret));
            }

#ifdef USE_CSID
            fqcsid_t *csids = context->pgw_fqcsid;

            /* Get the session ID by csid */
            for (uint16_t itr = 0; itr < csids->num_csid; itr++) {
                sess_csid *tmp = NULL;

                tmp = get_sess_csid_entry(csids->local_csid[itr]);
                if (tmp == NULL)
                    continue;

                /* VS: Delete sess id from csid table */
                for(uint16_t cnt = 0; cnt < tmp->seid_cnt; cnt++) {
                    if (sess_id == tmp->cp_seid[cnt]) {
                        for(uint16_t pos = cnt; pos < (tmp->seid_cnt - 1); pos++ )
                            tmp->cp_seid[pos] = tmp->cp_seid[pos + 1];

                        tmp->seid_cnt--;
                        clLog(clSystemLog, eCLSeverityDebug, "Session Deleted from csid table sid:%lu\n",
                                sess_id);
                    }
                }

                if (tmp->seid_cnt == 0) {
                    /* Cleanup Internal data structures */
                    ret = del_peer_csid_entry(&csids->local_csid[itr], S5S8_PGWC_PORT_ID);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                strerror(errno));
                        return -1;
                    }

                    /* Clean MME FQ-CSID */
                    if (context->mme_fqcsid != 0) {
                        ret = del_peer_csid_entry(&(context->mme_fqcsid)->local_csid[itr], S5S8_PGWC_PORT_ID);
                        if (ret) {
                            clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                    strerror(errno));
                            return -1;
                        }
                        if (!(context->mme_fqcsid)->num_csid)
                            rte_free(context->mme_fqcsid);
                    }

                    /* Clean UP FQ-CSID */
                    if (context->up_fqcsid != 0) {
                        ret = del_peer_csid_entry(&(context->up_fqcsid)->local_csid[itr],
                                SX_PORT_ID);
                        if (ret) {
                            clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                    strerror(errno));
                            return -1;
                        }
                        if (!(context->up_fqcsid)->num_csid)
                            rte_free(context->up_fqcsid);
                    }
                }

            }

#endif /* USE_CSID */
            rte_free(context);
        }

        return 0;
    }
#endif


    transData_t *gtpc_trans = proc_context->gtpc_trans; 
	/* Fill gtpv2c structure for sending on s11 interface */
	set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
			context->s11_mme_gtpc_teid, gtpc_trans->sequence);
	set_cause_accepted_ie((gtpv2c_header_t *) &del_resp, IE_INSTANCE_ZERO);

	del_resp.cause.header.len = ntohs(del_resp.cause.header.len);

	/*Encode the S11 delete session response message. */
	msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);

	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	/* Delete entry from session entry */
    printf("Delete session from the pfcp seid table \n");
	if (del_sess_entry_seid(sess_id) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for Key sess ID:%lu\n",
				__func__, __LINE__, sess_id);
		return -1;
	}

	if (del_rule_entries(context, ebi_index) != 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s %s - Error on delete rule entries\n",__file__,
				strerror(ret));
	}
	ret = delete_sgwc_context(teid, &context, &sess_id);
	if (ret)
		return ret;
    printf("%s %d : number of PDNS = %d  \n",__FUNCTION__, __LINE__,context->num_pdns);
    if(context->num_pdns == 0) {
        /* Delete UE context entry from UE Hash */
        if (ue_context_delete_entry_imsiKey(context->imsi) < 0){
            clLog(clSystemLog, eCLSeverityCritical,
                    "%s %s - Error on ue_context_by_fteid_hash del\n",__file__,
                    strerror(ret));
        }

        /* delete context from user context */
        uint32_t temp_teid = context->s11_sgw_gtpc_teid;
        ue_context_delete_entry_teidKey(temp_teid);


        /* Delete UPFList entry from UPF Hash */
        if ((context->dns_enable && upflist_by_ue_hash_entry_delete(&context->imsi, sizeof(context->imsi))) < 0){
            clLog(clSystemLog, eCLSeverityCritical,
                    "%s %s - Error on upflist_by_ue_hash deletion of IMSI \n",__file__,
                    strerror(ret));
        }

#ifdef USE_CSID
        fqcsid_t *csids = context->sgw_fqcsid;

        /* Get the session ID by csid */
        for (uint16_t itr = 0; itr < csids->num_csid; itr++) {
            sess_csid *tmp = NULL;

            tmp = get_sess_csid_entry(csids->local_csid[itr]);
            if (tmp == NULL)
                continue;

            /* VS: Delete sess id from csid table */
            for(uint16_t cnt = 0; cnt < tmp->seid_cnt; cnt++) {
                if (sess_id == tmp->cp_seid[cnt]) {
                    for(uint16_t pos = cnt; pos < (tmp->seid_cnt - 1); pos++ )
                        tmp->cp_seid[pos] = tmp->cp_seid[pos + 1];

                    tmp->seid_cnt--;
                    clLog(clSystemLog, eCLSeverityDebug, "Session Deleted from csid table sid:%lu\n",
                            sess_id);
                }
            }

            if (tmp->seid_cnt == 0) {
                /* Cleanup Internal data structures */
                ret = del_peer_csid_entry(&csids->local_csid[itr], S5S8_PGWC_PORT_ID);
                if (ret) {
                    clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                            strerror(errno));
                    return -1;
                }

                /* Clean MME FQ-CSID */
                if (context->mme_fqcsid != 0) {
                    ret = del_peer_csid_entry(&(context->mme_fqcsid)->local_csid[itr], S5S8_PGWC_PORT_ID);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                strerror(errno));
                        return -1;
                    }
                    if (!(context->mme_fqcsid)->num_csid)
                        rte_free(context->mme_fqcsid);
                }

                /* Clean UP FQ-CSID */
                if (context->up_fqcsid != 0) {
                    ret = del_peer_csid_entry(&(context->up_fqcsid)->local_csid[itr],
                            SX_PORT_ID);
                    if (ret) {
                        clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
                                strerror(errno));
                        return -1;
                    }
                    if (!(context->up_fqcsid)->num_csid)
                        rte_free(context->up_fqcsid);
                }
            }

        }

#endif /* USE_CSID */

        //Free UE context
        rte_free(context);
    }
	return 0;
}

#ifdef FUTURE_NEED
void
fill_pfcp_sess_mod_req_delete( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, ue_context_t *context, pdn_connection_t *pdn)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer;

	RTE_SET_USED(context);  /* NK:to be checked */

	if ((ret = upf_context_entry_lookup(pdn->upf_ipv4.s_addr,
					&upf_ctx)) < 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return;
	}

	if( header != NULL)
		clLog(sxlogger, eCLSeverityDebug, "TEID[%d]\n", header->teid.has_teid.teid);

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
				clLog(clSystemLog, eCLSeverityDebug,"default pfcp sess mod req\n");
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
#endif

