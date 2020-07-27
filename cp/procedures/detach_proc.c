
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "pfcp.h"
#include "gx_interface.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "gtpv2c_error_rsp.h"
#include "gtpc_session.h"
#include "cp_config.h"
#include "clogger.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "gtpv2_evt_handler.h"
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "detach_proc.h"
#include "pfcp_messages_encoder.h"
#include "cp_transactions.h"
#include "gtp_ies.h"

extern udp_sock_t my_sock;
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
		/* Forward s11 delete_session_request on s5s8 */
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
				s5s8_sockaddr_len);

		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, SENT,S5S8);
		update_sys_stat(number_of_users, DECREMENT);
		update_sys_stat(number_of_active_session, DECREMENT);
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
		update_cli_stats(peer_addr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, ACC,S11);

		update_sys_stat(number_of_users, DECREMENT);
		update_sys_stat(number_of_active_session, DECREMENT);

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
    update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_TERMINATE, SENT, GX);
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
    update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
            pfcp_sess_del_req.header.message_type,SENT,SX);
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
	if (get_sess_entry(context->pdns[ebi_index]->seid, &context) != 0){
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
