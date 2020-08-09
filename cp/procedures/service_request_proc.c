// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_common.h"
#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "gtpv2_error_rsp.h"
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
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "service_request_proc.h"
#include "pfcp_cp_util.h"
#include "pfcp_messages_encoder.h"
#include "gtpv2_set_ie.h"
#include "tables/tables.h"
#include "util.h"


extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern udp_sock_t my_sock;


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

    clLog(s11logger, eCLSeverityDebug, "%s: Msg_Type:%s[%u], Teid:%u, "
            "Procedure:%s, State:%s, Event:%s\n",
            __func__, gtp_type_str(msg->msg_type), msg->msg_type,
            mbr_header->teid.has_teid.teid,
            get_proc_string(msg->proc),
            get_state_string(msg->state), get_event_string(msg->event));

    ret = process_pfcp_sess_mod_request(msg);
    if (ret != 0) {
        // function never returns failure 
        assert(0);
    }

    RTE_SET_USED(unused_param);
    return 0;
}

void
process_pfcp_sess_mod_request_timeout(void *data)
{
    ue_context_t *ue_context = (ue_context_t *)data;
    proc_context_t *proc_context = ue_context->current_proc;
    msg_info_t msg = {0};

    msg.msg_type = PFCP_SESSION_MODIFICATION_RESPONSE;
    msg.proc_context = proc_context;
    proc_service_request_failure(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return;
}

int
process_pfcp_sess_mod_request(msg_info_t *msg)
{
    mod_bearer_req_t *mb_req = &msg->gtpc_msg.mbr;
    uint8_t ebi_index = 0;
    ue_context_t *context;
    eps_bearer_t *bearer;
    pdn_connection_t *pdn;
    pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    uint32_t sequence;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

    context = msg->ue_context;
    assert(context != NULL);

    bearer = msg->bearer_context;
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

    uint8_t pfcp_msg[sizeof(pfcp_sess_mod_req_t)] = {0};
    int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
    header->message_len = htons(encoded - 4);

    if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0 ){
        clLog(clSystemLog, eCLSeverityDebug,"Error sending: %i\n",errno);
        // Assume that its success and let retry take care of error handling  
    } 
    update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
            pfcp_sess_mod_req.header.message_type,SENT,SX);
    transData_t *trans_entry;
    trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pfcp_sess_mod_request_timeout);
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;

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

    proc_context_t *proc_context = msg->proc_context;

    /*Validate the modification is accepted or not. */
    if(msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value != REQUESTACCEPTED){
        clLog(sxlogger, eCLSeverityDebug, "Cause received Modify response is %d\n",
                msg->pfcp_msg.pfcp_sess_mod_resp.cause.cause_value);
        proc_service_request_failure(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
        return ;
    }
    /* Retrive the session information based on session id. */
    if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
                &context) != 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s: Session entry not found Msg_Type:%u,"
                "Sess ID:%lu, n",
                __func__, msg->msg_type,
                msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
        proc_service_request_failure(msg, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
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
        clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
                __func__, __LINE__, ret);
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

    update_cli_stats(trans->peer_sockaddr.sin_addr.s_addr,
            gtpv2c_tx->gtpc.message_type,ACC,S11);

    proc_service_request_complete(proc_context);

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
    ue_context_t *context;
    pdn_connection_t *pdn = NULL;
    uint32_t teid = UE_SESS_ID(sess_id);

    /* Retrieve the UE context */
    ret = get_ue_context(teid, &context);
    if (ret < 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
                __func__, __LINE__,
                teid);
    }

    assert(proc_context->ue_context == context);

    context = proc_context->ue_context;

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

    transData_t *gtpc_trans = proc_context->gtpc_trans;
    uint32_t sequence = gtpc_trans->sequence;
    /* Fill the modify bearer response */
    set_modify_bearer_response(gtpv2c_tx,
            sequence, context, bearer);

    /* Update the UE state */
    pdn->state = CONNECTED_STATE;

    return 0;
}

void
proc_service_request_failure(msg_info_t *msg, uint8_t cause)
{
    proc_context_t *proc_context = msg->proc_context;

    mbr_error_response(msg, cause,
            cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);

    proc_service_request_complete(proc_context);
    return;
}

void
proc_service_request_complete(proc_context_t *proc_context)
{
    ue_context_t *ue_context = proc_context->ue_context; 
    assert(proc_context->gtpc_trans != NULL);
    transData_t *trans = proc_context->gtpc_trans;

    uint16_t port_num = trans->peer_sockaddr.sin_port; 
    uint32_t sender_addr = trans->peer_sockaddr.sin_addr.s_addr; 
    uint32_t seq_num = proc_context->gtpc_trans->sequence; 

    transData_t *gtpc_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
    assert(gtpc_trans != NULL);

    /* Let's cross check if transaction from the table is matchig with the one we have 
     * in subscriber 
     */
    assert(gtpc_trans == trans);

    proc_context->gtpc_trans =  NULL;

    /* PFCP transaction is already complete. */
    assert(proc_context->pfcp_trans == NULL);

    free(gtpc_trans);
    free(proc_context);
    ue_context->current_proc = NULL;

    return;
}
