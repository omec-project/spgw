// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include "sm_struct.h"
#include "gtp_messages.h"
#include "spgw_config_struct.h"
#include "gtpv2_error_rsp.h"
#include "assert.h"
#include "cp_log.h"
#include "cp_peer.h"
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
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "proc_service_request.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_encoder.h"
#include "gtpv2_set_ie.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "proc.h"
#include "cp_common.h"


extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

proc_context_t*
alloc_service_req_proc(msg_info_t *msg)
{
    proc_context_t *service_req_proc;

    service_req_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(service_req_proc->proc_name, "SERVICE_REQ");
    service_req_proc->proc_type = SERVICE_REQUEST_PROC; 
    service_req_proc->ue_context = (void *)msg->ue_context;
    service_req_proc->pdn_context = (void *)msg->pdn_context; 
    service_req_proc->bearer_context = (void *)msg->bearer_context;
    service_req_proc->handler = service_req_event_handler;
    msg->proc_context = service_req_proc;
    SET_PROC_MSG(service_req_proc, msg);
 
    return service_req_proc;
}

void 
service_req_event_handler(void *proc, void *msg_info) 
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    switch(event) {
        case MB_REQ_RCVD_EVNT: {
            process_mb_req_handler(proc_context, msg);
            break;
        } 
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            process_service_request_pfcp_mod_sess_rsp(proc_context, msg);
            break;
        }
        default:
            assert(0); // wrong event 
    }
    return;
}

void
proc_service_request_success(proc_context_t *proc_context)
{
    proc_service_request_complete(proc_context);
}

void
proc_service_request_failure(msg_info_t *msg, uint8_t cause)
{
    proc_context_t *proc_context = (proc_context_t *)msg->proc_context;

    mbr_error_response(msg, cause,
            cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    increment_stat(PROCEDURES_SPGW_SERVICE_REQUEST_PROC_FAILURE);
    proc_service_request_complete(proc_context);
    return;
}

void
proc_service_request_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
    return;
}

void
process_pfcp_sess_mod_request_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    transData_t *pfcp_trans = (transData_t *)proc_context->pfcp_trans;
    ue_context_t   *context = (ue_context_t *)proc_context->ue_context;
    pfcp_trans->itr_cnt++;
    if(pfcp_trans->itr_cnt < cp_config->request_tries) {
	    LOG_MSG(LOG_ERROR, "PFCP session modification request retry(%d). IMSI %lu, " 
                       "PFCP sequence %d ", pfcp_trans->itr_cnt, context->imsi64, pfcp_trans->sequence);

        pfcp_timer_retry_send(my_sock.sock_fd_pfcp, pfcp_trans, &pfcp_trans->peer_sockaddr);
        restart_response_wait_timer(pfcp_trans);
        return;
    }
	LOG_MSG(LOG_ERROR, "PFCP session modification request timeout. IMSI %lu, " 
                       "PFCP sequence %d ", context->imsi64, pfcp_trans->sequence);

    msg_info_t *msg = (msg_info_t*)calloc(1, sizeof(msg_info_t));
    msg->msg_type = PFCP_SESSION_MODIFICATION_RESPONSE;
    msg->proc_context = proc_context;
    SET_PROC_MSG(proc_context, msg);
    proc_service_request_failure(msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_mb_req_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    mod_bearer_req_t *mb_req = &msg->rx_msg.mbr;
    ue_context_t *context;
    eps_bearer_t *bearer;
    pdn_connection_t *pdn;
    pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    context = (ue_context_t *)proc_context->ue_context;
    assert(context != NULL);

    bearer = (eps_bearer_t *)proc_context->bearer_context;
    assert(bearer != NULL);

    pdn = bearer->pdn;
    assert(pdn != NULL);

    // check if timezone has changed 
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

#ifdef FUTURE_NEED 
    if (mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.header.len &&
            (context->s11_mme_gtpc_teid != mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key))
        context->s11_mme_gtpc_teid = mb_req->bearer_contexts_to_be_modified.s11_u_mme_fteid.teid_gre_key;
#endif

    // we should never update bearer id 
    // bearer->eps_bearer_id = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;

    pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];
    pfcp_sess_mod_req.update_far_count = 0;

    uint8_t x2_handover = 0;
    if (mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len  != 0) {
        if(bearer->s1u_enb_gtpu_ipv4.s_addr != 0) {
            if((mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.teid_gre_key)
                    != bearer->s1u_enb_gtpu_teid  ||
                    (mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.ipv4_address) !=
                    bearer->s1u_enb_gtpu_ipv4.s_addr) {

                x2_handover = 1;
            }
        }

        /* Bug 370. No need to send end marker packet in DDN */
#ifdef TEMP
        if (CONN_SUSPEND_PROC == pdn->proc) {
            x2_handover = 0;
        }
#endif
    }

    if (mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.header.len  != 0) {
        for(int far_count = 0; far_count < bearer->pdr_count; far_count++) {
		    if (SOURCE_INTERFACE_VALUE_ACCESS == bearer->pdrs[far_count]->pdi.src_intfc.interface_value) {
                continue;
            }
            bearer->s1u_enb_gtpu_ipv4.s_addr = mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.ipv4_address;
            bearer->s1u_enb_gtpu_teid = mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.teid_gre_key;
            LOG_MSG(LOG_DEBUG, "bearer far %d, TEID = %d ",far_count, bearer->s1u_enb_gtpu_teid);

            update_far[far_count].upd_frwdng_parms.outer_hdr_creation.teid =
                bearer->s1u_enb_gtpu_teid;
            update_far[far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
                bearer->s1u_enb_gtpu_ipv4.s_addr;
            update_far[far_count].upd_frwdng_parms.dst_intfc.interface_value =
                check_interface_type(mb_req->bearer_contexts_to_be_modified.s1_enodeb_fteid.interface_type);
            update_far[far_count].apply_action.forw = PRESENT;
            pfcp_sess_mod_req.update_far_count++;
        }
    }

    if (mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.header.len  != 0) {
        for(int far_count = 0; far_count < bearer->pdr_count; far_count++) {
		    if (SOURCE_INTERFACE_VALUE_ACCESS == bearer->pdrs[far_count]->pdi.src_intfc.interface_value) {
                continue;
            }
            bearer->s5s8_sgw_gtpu_ipv4.s_addr = mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.ipv4_address;
            bearer->s5s8_sgw_gtpu_teid = mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.teid_gre_key;

            update_far[far_count].upd_frwdng_parms.outer_hdr_creation.teid =
                bearer->s5s8_sgw_gtpu_teid;
            update_far[far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
                bearer->s5s8_sgw_gtpu_ipv4.s_addr;
            update_far[far_count].upd_frwdng_parms.dst_intfc.interface_value =
                check_interface_type(mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.interface_type);
            if ( cp_config->cp_type != PGWC) {
                update_far[far_count].apply_action.forw = PRESENT;
            }
            pfcp_sess_mod_req.update_far_count++;
        }
    }

    sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &mb_req->header, bearer, pdn, update_far, x2_handover);

    uint8_t pfcp_msg[MAX_PFCP_MSG_SIZE] = {0};
    int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
    header->message_len = htons(encoded - 4);

    pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));

    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, process_pfcp_sess_mod_request_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;
    trans_entry->peer_sockaddr = context->upf_context->upf_sockaddr; 

    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

    /* Update UE State */
    pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

    return 0;
}

void 
process_service_request_pfcp_mod_sess_rsp(proc_context_t *proc_context, msg_info_t *msg)
{
    ue_context_t *context = NULL;
    int ret = 0;
    uint16_t payload_length = 0;

    /*Validate the modification is accepted or not. */
    if(msg->rx_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
        LOG_MSG(LOG_DEBUG, "Cause received Modify response is %d",
                msg->rx_msg.pfcp_sess_mod_resp.cause.cause_value);
        // TODO : mapping the pfcp cause on GTP
        proc_service_request_failure(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
        return ;
    }
    /* Retrive the session information based on session id. */
    context = (ue_context_t *)get_sess_entry_seid(msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
    if(context == NULL) {
        LOG_MSG(LOG_ERROR, "Session entry not found Msg_Type:%u,"
                "Sess ID:%lu ",
                msg->msg_type,
                msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
        proc_service_request_failure(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
        return;
    }
    assert(context == ((proc_context_t *)msg->proc_context)->ue_context);

    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:PFCP_SESSION_MODIFICATION_RESPONSE[%u], Seid:%lu, "
            "Procedure:%s, Event:%s",
            msg->msg_type,
            msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
            get_proc_string(msg->proc),
            get_event_string(msg->event));


    bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
    gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

    ret = process_srreq_pfcp_sess_mod_resp(proc_context, 
            msg->rx_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
            gtpv2c_tx);
    if (ret != 0) {
        LOG_MSG(LOG_ERROR, "Error: %d ", ret);
        if(ret != -1) {
            proc_service_request_failure(msg, ret);
        }
        return;
    }

    payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
        + sizeof(gtpv2c_tx->gtpc);

    transData_t *trans = proc_context->gtpc_trans;

    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
            (struct sockaddr *) &trans->peer_sockaddr,
            sizeof(struct sockaddr_in));

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_MBRSP, trans->peer_sockaddr.sin_addr.s_addr);

    increment_stat(PROCEDURES_SPGW_SERVICE_REQUEST_PROC_SUCCESS);
    proc_service_request_success(proc_context);

    return;
}

uint8_t
process_srreq_pfcp_sess_mod_resp(proc_context_t *proc_context, 
        uint64_t sess_id, 
        gtpv2c_header_t *gtpv2c_tx)
{
    uint8_t ebi_index = 0;
    eps_bearer_t *bearer  = NULL;
    ue_context_t *context;
    pdn_connection_t *pdn = NULL;
    uint32_t teid = UE_SESS_ID(sess_id);

    /* Retrieve the UE context */
    context = (ue_context_t *)get_ue_context(teid);
    if (context == NULL) {
        LOG_MSG(LOG_ERROR, "Failed to find UE context for teid: %u", teid);
        assert(0);
    }

    assert(proc_context->ue_context == context);

    context = (ue_context_t *)proc_context->ue_context;

    ebi_index = UE_BEAR_ID(sess_id) - 5;
    bearer = context->eps_bearers[ebi_index];

    if (!bearer) {
        LOG_MSG(LOG_ERROR,
                "Retrive modify bearer context but EBI is non-existent- "
                "Bitmap Inconsistency - Dropping packet");
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    /* Update the UE state */
    pdn = GET_PDN(context, ebi_index);
    assert(proc_context->pdn_context == pdn);

    transData_t *gtpc_trans = proc_context->gtpc_trans;
    uint32_t sequence = gtpc_trans->sequence;
    /* Fill the modify bearer response */
    set_modify_bearer_response(gtpv2c_tx,
            sequence, context, bearer);

    /* Update the UE state */
    pdn->state = CONNECTED_STATE;

    return 0;
}


