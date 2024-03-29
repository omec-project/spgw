// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include "sm_struct.h"
#include "gtp_messages.h"
#include "spgw_config_struct.h"
#include "gtpv2_error_rsp.h"
#include "assert.h"
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
#include "proc_initial_attach.h"
#include "proc_pfcp_assoc_setup.h"
#include "ipc_api.h"
#include "util.h"
#include "cp_io_poll.h"
#include "spgwStatsPromEnum.h"
#include "pfcp_cp_util.h"
#include "gtpv2_ie_parsing.h"
#include "gtpv2_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "cp_log.h"
#include "upf_apis.h"
#include "proc.h"
#include "ue.h"
#include "cp_common.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

/* local functions. Need to find suitable place for declarations  */

int
fill_uli_info(gtp_user_loc_info_ie_t *uli, ue_context_t *context);

int
fill_context_info(create_sess_req_t *csr, ue_context_t *context);

int
fill_pdn_info(create_sess_req_t *csr, pdn_connection_t *pdn);

int
fill_bearer_info(create_sess_req_t *csr, eps_bearer_t *bearer,
		ue_context_t *context, pdn_connection_t *pdn);

void
fill_rule_and_qos_inform_in_pdn(pdn_connection_t *pdn);

void 
initiate_upf_session(proc_context_t *csreq_proc, msg_info_t *msg); 

proc_context_t*
alloc_initial_proc(msg_info_t *msg)
{
    proc_context_t *csreq_proc;
    csreq_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(csreq_proc->proc_name, "INITIAL_ATTACH");
    csreq_proc->proc_type = INITIAL_PDN_ATTACH_PROC; 
    csreq_proc->handler = initial_attach_event_handler;
    msg->proc_context = csreq_proc;
    SET_PROC_MSG(csreq_proc, msg);
    return csreq_proc;
}

void 
initial_attach_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;

    // FIXME : event name instead of number 
    LOG_MSG(LOG_DEBUG4,"Received event %d ",event);
    switch(event) {
        case CS_REQ_RCVD_EVNT: {
            handle_csreq_msg(proc_context, msg);
            break;
        }
        case CCA_RCVD_EVNT: {
            cca_msg_handler(proc_context, msg); 
            break;
        }
        case GX_CCRI_TIMEOUT_EVNT: 
        case GX_CCAI_FAILED: {
            process_gx_ccai_reject_handler(proc_context, msg);
            break; 
        }

        case PFCP_ASSOC_SETUP_SUCCESS:
        case PFCP_SESS_EST_EVNT: {
            initiate_upf_session(proc_context, msg);
            break;
        } 
        case PFCP_ASSOC_SETUP_FAILED: {
            process_sess_est_resp_association_failed_handler(proc_context, msg);
            break;
        }

        case PFCP_SESS_EST_RESP_RCVD_EVNT: {
            process_sess_est_resp_handler(proc_context, msg);
            break; 
        }
        case PFCP_SESS_EST_RESP_TIMEOUT_EVNT: {
            process_sess_est_resp_timeout_handler(proc_context, msg);
            break; 
        }
        default: {
            assert(0); // unhandled event 
        }
    }
    return;
}

void proc_initial_attach_success(proc_context_t *proc_context)
{
    increment_stat(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
    proc_initial_attach_complete(proc_context);
}
 
// TODO : need to define local cause above 255. That way we can increment stats 
void proc_initial_attach_failure(proc_context_t *proc_context, int cause)
{
    msg_info_t *msg = (msg_info_t *)proc_context->msg_info;
    transData_t *trans_rec = proc_context->gtpc_trans;
    if(cause != -1) {
        increment_proc_mme_peer_stats_reason(PROCEDURES_SPGW_INITIAL_ATTACH_FAILURE, 
                                    trans_rec->peer_sockaddr.sin_addr.s_addr, cause, proc_context->tac);
        // send cs response 
        cs_error_response(msg, cause,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
    } else {
        increment_proc_mme_peer_stats(PROCEDURES_SPGW_INITIAL_ATTACH_FAILURE, 
                                 trans_rec->peer_sockaddr.sin_addr.s_addr, proc_context->tac);
    }
    
    // UE context is not allocated and message is rejected
    proc_context->result = PROC_RESULT_FAILURE;
    proc_initial_attach_complete(proc_context);
    return;
}

void 
proc_initial_attach_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
    return;
}

int
handle_csreq_msg(proc_context_t *csreq_proc, msg_info_t *msg)
{
    int ret = 0;
	ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;
    transData_t *gtpc_trans = csreq_proc->gtpc_trans;

    csreq_proc->imsi64 = msg->rx_msg.csr.imsi.imsi64; 
    increment_proc_mme_peer_stats(PROCEDURES_SPGW_INITIAL_ATTACH,
                                  gtpc_trans->peer_sockaddr.sin_addr.s_addr,
                                  msg->rx_msg.csr.uli.tai2.tai_tac);

	ret = process_create_sess_req(csreq_proc,
                                  &msg->rx_msg.csr,
			                      &context, &pdn, msg);
	if (ret) {
        // > 0 cause - Send reject message out 
        // -1 : just cleanup call locally 
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
        csreq_proc->tac = msg->rx_msg.csr.uli.tai2.tai_tac;
        proc_initial_attach_failure(csreq_proc, ret);
		return -1;
	}

    csreq_proc->tac = msg->rx_msg.csr.uli.tai2.tai_tac;
    csreq_proc->ue_context = (void *)context;
    csreq_proc->pdn_context = (void *)pdn;
    TAILQ_INSERT_TAIL(&context->pending_sub_procs, csreq_proc, next_sub_proc);

    if ((cp_config->cp_type == PGWC) || (cp_config->cp_type == SAEGWC)) {
        if(cp_config->gx_enabled) {
            if (gen_ccr_request(csreq_proc, pdn->default_bearer_id-5, &msg->rx_msg.csr)) {
                LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
                return -1;
            }
            // Wait for CCA-I from Gx before initiating any further messages 
            return 0; 
        } else {
            fill_rule_and_qos_inform_in_pdn(pdn);
        }
    }
    msg->event = PFCP_SESS_EST_EVNT;
    csreq_proc->handler(csreq_proc, msg); 
    return 0;
}

void 
initiate_upf_session(proc_context_t *csreq_proc, msg_info_t *msg) 
{
    ue_context_t *context = (ue_context_t *)csreq_proc->ue_context;
	upf_context_t *upf_context = context->upf_context;

    if (upf_context->state == PFCP_ASSOC_REQ_SNT_STATE) {
        LOG_MSG(LOG_DEBUG1,"UPF association formation in progress. Buffer new CSReq  ");
        // After every setup timeout we would flush the buffered entries..
        // dont worry about #of packets in buffer for now 
        buffer_csr_request(csreq_proc);
        csreq_proc->flags |= UPF_ASSOCIATION_PENDING;
        return;
    } else if (upf_context->state == PFCP_ASSOC_RESP_RCVD_STATE) {
        LOG_MSG(LOG_DEBUG, "UPF association already formed ");
        pdn_connection_t *pdn = (pdn_connection_t *)msg->pdn_context;
        if(!IS_UPF_SUPP_FEAT_UEIP(upf_context)) {
            if(IF_PDN_ADDR_ALLOC_UPF(pdn)) {
              LOG_MSG(LOG_ERROR, "UPF does not support UE IP address allocation feature ");
              proc_initial_attach_failure(csreq_proc, GTPV2C_CAUSE_REQUEST_REJECTED);
              return;
            }
        } else {
            // FIXME : add proper comments 
            // UPF should allocate address..but if control plane has already allocated address
            // then release address 
            LOG_MSG(LOG_DEBUG, "UPF supports UE IP address allocation feature ");
            if(IF_PDN_ADDR_ALLOC_CONTROL(pdn)) {
                LOG_MSG(LOG_ERROR, "PDN has address assigned from control plane. But UPF supports address allocation ");
                struct in_addr ue_ip = pdn->ipv4;
                release_ip(ue_ip); // api needs address in host order
                pdn->ipv4.s_addr = 0;
                RESET_PDN_ADDR_METHOD(pdn, PDN_ADDR_ALLOC_CONTROL);
                SET_PDN_ADDR_METHOD(pdn, PDN_ADDR_ALLOC_UPF);
                LOG_MSG(LOG_DEBUG, "PDN flags %x ",pdn->pdn_flags);
            }
        }
        transData_t *pfcp_trans = process_pfcp_sess_est_request(csreq_proc, upf_context);
        if (pfcp_trans == NULL) {
            LOG_MSG(LOG_ERROR, "Error: pfcp send error ");
            proc_initial_attach_failure(csreq_proc, GTPV2C_CAUSE_INVALID_PEER);
            return;
        }
        csreq_proc->pfcp_trans = pfcp_trans;
        return;
    } else {
        // create PFCP association setup proccedure 
        // we are not modifying original msg content. This msg has ue/pdn context
        buffer_csr_request(csreq_proc);
        csreq_proc->flags |= UPF_ASSOCIATION_PENDING;
        proc_context_t *proc_context = alloc_pfcp_association_setup_proc((void *)context->upf_context); 
        start_upf_procedure(proc_context, (msg_info_t*)proc_context->msg_info);
        return;
    }
    return;
}

// return values > 0 : - GTP cause 
// -1 : context replacement, mis error 
// 0 : success
int
process_create_sess_req(proc_context_t *proc_ctxt,
                        create_sess_req_t *csr,
		                ue_context_t **_context,
                        pdn_connection_t **pdn_ctxt,
                        msg_info_t *msg)
{
	int ret = 0;
	struct in_addr ue_ip = {0};
	ue_context_t *context = NULL;
	eps_bearer_t *bearer = NULL;
	pdn_connection_t *pdn = NULL;
    bool static_addr_pdn = false;
    uint32_t addr_upf_alloc = PDN_ADDR_ALLOC_CONTROL; /* default allocation is by control Plane */
    struct in_addr upf_ipv4 = {0};
    /* Should we get default context ?*/
    upf_context_t *upf_context=NULL; 
    sub_config_t *sub_profile = NULL;
    sub_selection_keys_t dpkey = {0}; 


    if(match_apn_profile((char *)csr->apn.apn, csr->apn.header.len) == NULL) {
        // caller sends out csrsp 
        return GTPV2C_CAUSE_MISSING_UNKNOWN_APN; 
    }
    // DNS would need changes here 
    /* TODO - IE presense should be validated before accessing them */

    dpkey.imsi.is_valid = true;
    dpkey.imsi.from_imsi = csr->imsi.imsi64; 
    dpkey.imsi.to_imsi =  csr->imsi.imsi64;
    
    dpkey.plmn.is_valid = true;
    dpkey.plmn.tac = csr->uli.tai2.tai_tac;
    memcpy((void *)(&dpkey.plmn.plmn[0]), (void *)(&csr->uli.tai2), 3);

    dpkey.apn.is_valid = true;
    memcpy((void *)(&dpkey.apn.requested_apn[0]), (void *)(&csr->apn.apn), csr->apn.header.len);

    LOG_MSG(LOG_DEBUG, "Create Session Request ULI mcc %d %d %d " 
                " mnc %d %d %d ", csr->uli.tai2.tai_mcc_digit_1, csr->uli.tai2.tai_mcc_digit_2, 
                csr->uli.tai2.tai_mcc_digit_3, csr->uli.tai2.tai_mnc_digit_1, 
                csr->uli.tai2.tai_mnc_digit_2, csr->uli.tai2.tai_mnc_digit_3);
    
    sub_profile = match_sub_selection(&dpkey); 
    
    // no upf available 
    if(sub_profile == NULL) {
        // caller sends out csrsp 
        return GTPV2C_CAUSE_REQUEST_REJECTED ; 
    }
    proc_ctxt->sub_config = (sub_config_t *) sub_profile;
    {
        // FIXME - UPF profile may have been deleted by now
        upf_context = get_upf_context(sub_profile->user_plane_service, sub_profile->global_address);
        if((upf_context == NULL) ||  (upf_context->upf_sockaddr.sin_addr.s_addr == 0)) {
            return GTPV2C_CAUSE_REQUEST_REJECTED ; 
        }
        upf_ipv4 = upf_context->upf_sockaddr.sin_addr;
    }

    LOG_MSG(LOG_DEBUG, "Selected UPF address  %s ", inet_ntoa(upf_ipv4));
#if 0
	if(csr->mapped_ue_usage_type.header.len > 0) {
		apn_requested->apn_usage_type = csr->mapped_ue_usage_type.mapped_ue_usage_type;
	}
#endif

    /* Requirementss  Prio-5. New networks support all the 15 EBIs  */
	uint8_t ebi_index = csr->bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;

    ret = 0;
    {
        struct in_addr *paa_ipv4 = (struct in_addr *) &csr->paa.pdn_addr_and_pfx[0];
        if (csr->paa.pdn_type == PDN_IP_TYPE_IPV4 && paa_ipv4->s_addr != 0) {
            bool found = false;
            struct in_addr temp = *paa_ipv4;
            temp.s_addr = ntohl(temp.s_addr);
            found = reserve_ip_node(temp); // api needs address in host order
            if (found == false) {
                LOG_MSG(LOG_ERROR,"Received CSReq with static address %s"
                        " . Invalid address received ",
                        inet_ntoa(*paa_ipv4));
                // caller sends out csrsp 
                return GTPV2C_CAUSE_REQUEST_REJECTED;
            }
            ue_ip = *paa_ipv4; // network order 
            static_addr_pdn = true;
        } else {
            if(IS_UPF_SUPP_FEAT_UEIP(upf_context)) {
                addr_upf_alloc = PDN_ADDR_ALLOC_UPF;
                LOG_MSG(LOG_DEBUG, "UPF supports UE IP address allocation feature ");
            }
            else if(upf_context->global_address == true) {
              LOG_MSG(LOG_DEBUG, "Using control plane support of UE IP address allocation ");
              // if global address allocation required then pull 1 IP address from central control Plane 
              addr_upf_alloc = PDN_ADDR_ALLOC_CONTROL;
              ret = acquire_ip(&ue_ip); // address in network order
              if (ret) {
                      // caller sends out csrsp 
                      return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
              }
            } else {
                // UPF allocates IP address. Control Plane does validation
                // at later stage to reject
                addr_upf_alloc = PDN_ADDR_ALLOC_UPF;
            }
        }
    }


	LOG_MSG(LOG_INFO, "IMSI - %lu, Acquire ip = %s ",csr->imsi.imsi64, inet_ntoa(ue_ip));
	/* set s11_sgw_gtpc_teid= key->ue_context_by_fteid_hash */
	ret = create_ue_context(&csr->imsi.imsi_number_digits, csr->imsi.header.len,
			csr->bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi, &context, csr->apn.apn, csr->apn.header.len);
	if (ret) {
        // Free allocated UE Ip address. ue_ip is in network order  
        if(static_addr_pdn == false) {
            ue_ip.s_addr = ntohl(ue_ip.s_addr);
            release_ip(ue_ip); // api needs address in host order
        }
		return ret;
    }

    context->imsi64 = csr->imsi.imsi64;
	if (csr->mei.header.len)
		memcpy(&context->mei, &csr->mei.mei, csr->mei.header.len);

	memcpy(&context->msisdn, &csr->msisdn.msisdn_number_digits, csr->msisdn.header.len);

	if (fill_context_info(csr, context) != 0) {
			return -1;
    }

    if(csr->pco_new.header.len != 0) {
        proc_ctxt->req_pco = (pco_ie_t *)calloc(1, sizeof(pco_ie_t));
        memcpy(proc_ctxt->req_pco, (void *)(&csr->pco_new), sizeof(pco_ie_t));
    }

	// TODOFIX - does not seem to be correct 
	if (cp_config->cp_type == PGWC) {
		context->s11_mme_gtpc_teid = csr->sender_fteid_ctl_plane.teid_gre_key;
    }

	/* Retrive procedure of CSR */
	pdn = context->eps_bearers[ebi_index]->pdn;

    SET_PDN_ADDR_STATIC(pdn, static_addr_pdn);

    SET_PDN_ADDR_METHOD(pdn, addr_upf_alloc);

	/* VS: Stored the RAT TYPE information in UE context */
	if (csr->rat_type.header.len != 0) {
		context->rat_type.rat_type = csr->rat_type.rat_type;
		context->rat_type.len = csr->rat_type.header.len;
	}

	/* VS: Stored the RAT TYPE information in UE context */
	if (csr->uli.header.len != 0) {
		if (fill_uli_info(&csr->uli, context) != 0)
			return -1;
	}

	/* VS: Stored the mapped ue usage type information in UE context */
	if (csr->mapped_ue_usage_type.header.len != 0) {
		context->mapped_ue_usage_type =
			csr->mapped_ue_usage_type.mapped_ue_usage_type;
	} else
		context->mapped_ue_usage_type = -1;

    
	strcpy((char *)pdn->apn, (char *)csr->apn.apn);
    pdn->apn_len = csr->apn.header.len;

    // DELETE THIS 
	/* Store upf ipv4 in pdn structure */
	pdn->upf_ipv4 = upf_ipv4;

	if (fill_pdn_info(csr, pdn) != 0)
		return -1;

	bearer = context->eps_bearers[ebi_index];

    if (cp_config->cp_type == PGWC) {
		/* VS: Maitain the fqdn into table */
		memcpy(pdn->fqdn, (char *)csr->sgw_u_node_name.fqdn,
				csr->sgw_u_node_name.header.len);

		pdn->ipv4.s_addr = ntohl(ue_ip.s_addr);
		context->pdns[ebi_index]->seid = SESS_ID(pdn->s5s8_pgw_gtpc_teid, bearer->eps_bearer_id);
	} else {
		pdn->ipv4.s_addr = ntohl(ue_ip.s_addr);
		context->pdns[ebi_index]->seid = SESS_ID(context->s11_sgw_gtpc_teid, bearer->eps_bearer_id);
	}

	if (fill_bearer_info(csr, bearer, context, pdn) != 0)
		return -1;

	/* SGW Handover Storage */
	if (csr->indctn_flgs.header.len != 0 && 
		csr->indctn_flgs.indication_oi == 1)
	{
		pdn->ipv4.s_addr = pdn->ipv4.s_addr;
		context->indication_flag.oi = csr->indctn_flgs.indication_oi;
		pdn->s5s8_pgw_gtpc_teid = csr->pgw_s5s8_addr_ctl_plane_or_pmip.teid_gre_key;
		bearer->s5s8_pgw_gtpu_ipv4.s_addr = csr->bearer_contexts_to_be_created.s5s8_u_pgw_fteid.ipv4_address;
		bearer->s5s8_pgw_gtpu_teid = csr->bearer_contexts_to_be_created.s5s8_u_pgw_fteid.teid_gre_key;
		bearer->s1u_enb_gtpu_teid =   csr->bearer_contexts_to_be_created.s1u_enb_fteid.teid_gre_key;
		bearer->s1u_enb_gtpu_ipv4.s_addr = csr->bearer_contexts_to_be_created.s1u_enb_fteid.ipv4_address;

	}
	context->pdns[ebi_index]->dp_seid = 0;


	/* VS: Store the context of ue in pdn*/
	pdn->context = context;

	/* VS: Return the UE context */
	*_context = context;
    *pdn_ctxt = pdn; 

    context->upf_context = upf_context;

    msg->pdn_context = pdn;
    msg->ue_context  = context;

    pdn->upf_ipv4 = upf_ipv4; 
    return 0;
}

int
process_sess_est_resp_association_failed_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    transData_t *pfcp_trans = (transData_t *)proc_context->pfcp_trans;
	LOG_MSG(LOG_ERROR, "PFCP session establishment failed due to association failed. IMSI %lu, " 
                       "PFCP sequence %d ", proc_context->imsi64, pfcp_trans ? pfcp_trans->sequence:0);
    proc_initial_attach_failure(proc_context, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    return 0;
}

int
process_sess_est_resp_timeout_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    transData_t *pfcp_trans = (transData_t *)proc_context->pfcp_trans;
    pfcp_trans->itr_cnt++;
    if(pfcp_trans->itr_cnt < cp_config->request_tries) {
	    LOG_MSG(LOG_ERROR, "PFCP session establishment request retry. IMSI %lu, " 
                           "PFCP sequence %d, attempt %d, pfcp_fd = %d ", proc_context->imsi64, 
                           pfcp_trans->sequence, pfcp_trans->itr_cnt, my_sock.sock_fd_pfcp);

        pfcp_timer_retry_send(my_sock.sock_fd_pfcp, pfcp_trans, &pfcp_trans->peer_sockaddr);
        restart_response_wait_timer(pfcp_trans);
    } else {
	    LOG_MSG(LOG_ERROR, "PFCP session establishment request timeout. IMSI %lu, " 
                       "PFCP sequence %d ", proc_context->imsi64, pfcp_trans->sequence);
        proc_initial_attach_failure(proc_context, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING);
    }
    
    return 0;
}

int
process_sess_est_resp_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	uint16_t payload_length = 0;

	LOG_MSG(LOG_DEBUG, "Callback called for "
			"Msg_Type:PFCP_SESSION_ESTABLISHMENT_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, Event:%s",
			msg->msg_type,
			msg->rx_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_event_string(msg->event));


	if(msg->rx_msg.pfcp_sess_est_resp.cause.cause_value != REQUESTACCEPTED) {
		LOG_MSG(LOG_DEBUG, "Cause received Est response is %d",
				msg->rx_msg.pfcp_sess_est_resp.cause.cause_value);
        ue_context_t *context = (ue_context_t *)proc_context->ue_context;
        upf_context_t *upf_ctxt = context->upf_context;
        proc_initial_attach_failure(proc_context, GTPV2C_CAUSE_REQUEST_REJECTED);
        if(msg->rx_msg.pfcp_sess_est_resp.cause.cause_value == NOESTABLISHEDPFCPASSOCIATION) {
		    LOG_MSG(LOG_INFO, "Initiate pfcp associations ");
            upf_ctxt->state = 0;
            schedule_pfcp_association(1,upf_ctxt); 
        }
		return -1;
	}


	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_est_resp(msg,
			&msg->rx_msg.pfcp_sess_est_resp, gtpv2c_tx);

	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
        proc_initial_attach_failure(proc_context, ret);
		return -1;
	}
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (cp_config->cp_type == PGWC) {
        transData_t *trans_rec = proc_context->gtpc_trans;
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &trans_rec->peer_sockaddr,
		        sizeof(struct sockaddr_in));
//        increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_CSRSP, trans_rec->s5s8_recv_sockaddr.sin_addr.s_addr);

		//s5s8_sgwc_msgcnt++;
	} else if (cp_config->cp_type == SAEGWC) {
        transData_t *trans_rec = proc_context->gtpc_trans;
		/* Send response on s11 interface */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &trans_rec->peer_sockaddr,
				sizeof(struct sockaddr_in));

        increment_mme_peer_stats(MSG_TX_GTPV2_S11_CSRSP, trans_rec->peer_sockaddr.sin_addr.s_addr);
        increment_proc_mme_peer_stats(PROCEDURES_SPGW_INITIAL_ATTACH_SUCCESS, trans_rec->peer_sockaddr.sin_addr.s_addr, proc_context->tac);
    
        ue_context_t *context = (ue_context_t *)proc_context->ue_context;
        pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
        increment_ue_info_stats(SUBSCRIBERS_INFO_SPGW_PDN, context->imsi64, pdn->ipv4.s_addr);
        proc_initial_attach_success(proc_context);
        // Dont access proc_context now onward 
        
	}
	return 0;
}

int8_t
process_pfcp_sess_est_resp(msg_info_t *msg, 
                           pfcp_sess_estab_rsp_t *pfcp_sess_est_rsp, 
                           gtpv2c_header_t *gtpv2c_tx)
{
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint64_t sess_id = pfcp_sess_est_rsp->header.seid_seqno.has_seid.seid;
	uint64_t dp_sess_id = pfcp_sess_est_rsp->up_fseid.seid;
	uint32_t teid = UE_SESS_ID(sess_id);

	LOG_MSG(LOG_DEBUG, "sess_id %lu teid = %u ", sess_id, teid);
    proc_context_t *proc_context;

	/* Retrieve the UE context */
	context = (ue_context_t *)get_ue_context(teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
    proc_context = (proc_context_t *)msg->proc_context; 
    assert(proc_context != NULL);
	proc_context->state = PFCP_SESS_EST_RESP_RCVD_STATE;


    pdn = (pdn_connection_t *)proc_context->pdn_context;
    assert(pdn != NULL);
    assert(sess_id == pdn->seid);
    uint8_t ebi_index = pdn->default_bearer_id - 5; 
    assert(context->eps_bearers[ebi_index] != NULL);
	pdn = context->eps_bearers[ebi_index]->pdn;
    assert(proc_context->pdn_context == pdn);
	bearer = context->eps_bearers[ebi_index];
    assert(bearer != NULL);
	pdn->dp_seid = dp_sess_id;
    if(pfcp_sess_est_rsp->created_pdr.ue_ip_address.v4 != 0)
        pdn->ipv4.s_addr = ntohl(pfcp_sess_est_rsp->created_pdr.ue_ip_address.ipv4_address);
    struct in_addr temp_addr = pdn->ipv4; 
    temp_addr.s_addr = temp_addr.s_addr;
	LOG_MSG(LOG_DEBUG, "DP session id %lu. UE address = %s  ", dp_sess_id, inet_ntoa(temp_addr));

	/* Update the UE state */
	pdn->state = PFCP_SESS_EST_RESP_RCVD_STATE;

	if (cp_config->cp_type == SAEGWC) {
        transData_t *gtpc_trans = proc_context->gtpc_trans; 
		set_create_session_response(proc_context,
				gtpv2c_tx, gtpc_trans->sequence, context, pdn, bearer);
	} else if (cp_config->cp_type == PGWC) {
		/*TODO: This needs to be change after support libgtpv2 on S5S8*/
		/* set_pgwc_s5s8_create_session_response(gtpv2c_tx,
				(htonl(context->sequence) >> 8), pdn, bearer); */

		create_sess_rsp_t cs_resp = {0};

		//uint32_t  seq_no = 0;
		//seq_no = bswap_32(resp->sequence);
		//seq_no = seq_no >> 8;
        uint32_t sequence = 0; // TODO...use transactions 
		fill_pgwc_create_session_response(&cs_resp,
			sequence, context, ebi_index);


		uint16_t msg_len = encode_create_sess_rsp(&cs_resp, (uint8_t*)gtpv2c_tx);
		msg_len = msg_len - 4;
		gtpv2c_header_t *header;
		header = (gtpv2c_header_t*) gtpv2c_tx;
			header->gtpc.message_len = htons(msg_len);

		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
						htonl(pdn->s5s8_sgw_gtpc_ipv4.s_addr);
	}

	/* Update the UE state */
	pdn->state = CONNECTED_STATE;
	return 0;
}

/**
 * @brief  : Fill ULI information into UE context from CSR
 * @param  : uli is pointer to structure to store uli info
 * @param  : context is a pointer to ue context structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
fill_uli_info(gtp_user_loc_info_ie_t *uli, ue_context_t *context)
{
	if (uli->lai) {
		context->uli.lai = uli->lai;
		context->uli.lai2.lai_mcc_digit_2 = uli->lai2.lai_mcc_digit_2;
		context->uli.lai2.lai_mcc_digit_1 = uli->lai2.lai_mcc_digit_1;
		context->uli.lai2.lai_mnc_digit_3 = uli->lai2.lai_mnc_digit_3;
		context->uli.lai2.lai_mcc_digit_3 = uli->lai2.lai_mcc_digit_3;
		context->uli.lai2.lai_mnc_digit_2 = uli->lai2.lai_mnc_digit_2;
		context->uli.lai2.lai_mnc_digit_1 = uli->lai2.lai_mnc_digit_1;
		context->uli.lai2.lai_lac = uli->lai2.lai_lac;
	}

	if (uli->tai) {
		context->uli.tai = uli->tai;
		context->uli.tai2.tai_mcc_digit_2 = uli->tai2.tai_mcc_digit_2;
		context->uli.tai2.tai_mcc_digit_1 = uli->tai2.tai_mcc_digit_1;
		context->uli.tai2.tai_mnc_digit_3 = uli->tai2.tai_mnc_digit_3;
		context->uli.tai2.tai_mcc_digit_3 = uli->tai2.tai_mcc_digit_3;
		context->uli.tai2.tai_mnc_digit_2 = uli->tai2.tai_mnc_digit_2;
		context->uli.tai2.tai_mnc_digit_1 = uli->tai2.tai_mnc_digit_1;
		context->uli.tai2.tai_tac = uli->tai2.tai_tac;
	}

	if (uli->rai) {
		context->uli.rai = uli->rai;
		context->uli.rai2.ria_mcc_digit_2 = uli->rai2.ria_mcc_digit_2;
		context->uli.rai2.ria_mcc_digit_1 = uli->rai2.ria_mcc_digit_1;
		context->uli.rai2.ria_mnc_digit_3 = uli->rai2.ria_mnc_digit_3;
		context->uli.rai2.ria_mcc_digit_3 = uli->rai2.ria_mcc_digit_3;
		context->uli.rai2.ria_mnc_digit_2 = uli->rai2.ria_mnc_digit_2;
		context->uli.rai2.ria_mnc_digit_1 = uli->rai2.ria_mnc_digit_1;
		context->uli.rai2.ria_lac = uli->rai2.ria_lac;
		context->uli.rai2.ria_rac = uli->rai2.ria_rac;
	}

	if (uli->sai) {
		context->uli.sai = uli->sai;
		context->uli.sai2.sai_mcc_digit_2 = uli->sai2.sai_mcc_digit_2;
		context->uli.sai2.sai_mcc_digit_1 = uli->sai2.sai_mcc_digit_1;
		context->uli.sai2.sai_mnc_digit_3 = uli->sai2.sai_mnc_digit_3;
		context->uli.sai2.sai_mcc_digit_3 = uli->sai2.sai_mcc_digit_3;
		context->uli.sai2.sai_mnc_digit_2 = uli->sai2.sai_mnc_digit_2;
		context->uli.sai2.sai_mnc_digit_1 = uli->sai2.sai_mnc_digit_1;
		context->uli.sai2.sai_lac = uli->sai2.sai_lac;
		context->uli.sai2.sai_sac = uli->sai2.sai_sac;
	}

	if (uli->cgi) {
		context->uli.cgi = uli->cgi;
		context->uli.cgi2.cgi_mcc_digit_2 = uli->cgi2.cgi_mcc_digit_2;
		context->uli.cgi2.cgi_mcc_digit_1 = uli->cgi2.cgi_mcc_digit_1;
		context->uli.cgi2.cgi_mnc_digit_3 = uli->cgi2.cgi_mnc_digit_3;
		context->uli.cgi2.cgi_mcc_digit_3 = uli->cgi2.cgi_mcc_digit_3;
		context->uli.cgi2.cgi_mnc_digit_2 = uli->cgi2.cgi_mnc_digit_2;
		context->uli.cgi2.cgi_mnc_digit_1 = uli->cgi2.cgi_mnc_digit_1;
		context->uli.cgi2.cgi_lac = uli->cgi2.cgi_lac;
		context->uli.cgi2.cgi_ci = uli->cgi2.cgi_ci;
	}

	if (uli->ecgi) {
		context->uli.ecgi = uli->ecgi;
		context->uli.ecgi2.ecgi_mcc_digit_2 = uli->ecgi2.ecgi_mcc_digit_2;
		context->uli.ecgi2.ecgi_mcc_digit_1 = uli->ecgi2.ecgi_mcc_digit_1;
		context->uli.ecgi2.ecgi_mnc_digit_3 = uli->ecgi2.ecgi_mnc_digit_3;
		context->uli.ecgi2.ecgi_mcc_digit_3 = uli->ecgi2.ecgi_mcc_digit_3;
		context->uli.ecgi2.ecgi_mnc_digit_2 = uli->ecgi2.ecgi_mnc_digit_2;
		context->uli.ecgi2.ecgi_mnc_digit_1 = uli->ecgi2.ecgi_mnc_digit_1;
		context->uli.ecgi2.ecgi_spare = uli->ecgi2.ecgi_spare;
		context->uli.ecgi2.eci = uli->ecgi2.eci;
	}

	if (uli->macro_enodeb_id) {
		context->uli.macro_enodeb_id = uli->macro_enodeb_id;
		context->uli.macro_enodeb_id2.menbid_mcc_digit_2 =
			uli->macro_enodeb_id2.menbid_mcc_digit_2;
		context->uli.macro_enodeb_id2.menbid_mcc_digit_1 =
			uli->macro_enodeb_id2.menbid_mcc_digit_1;
		context->uli.macro_enodeb_id2.menbid_mnc_digit_3 =
			uli->macro_enodeb_id2.menbid_mnc_digit_3;
		context->uli.macro_enodeb_id2.menbid_mcc_digit_3 =
			uli->macro_enodeb_id2.menbid_mcc_digit_3;
		context->uli.macro_enodeb_id2.menbid_mnc_digit_2 =
			uli->macro_enodeb_id2.menbid_mnc_digit_2;
		context->uli.macro_enodeb_id2.menbid_mnc_digit_1 =
			uli->macro_enodeb_id2.menbid_mnc_digit_1;
		context->uli.macro_enodeb_id2.menbid_spare =
			uli->macro_enodeb_id2.menbid_spare;
		context->uli.macro_enodeb_id2.menbid_macro_enodeb_id =
			uli->macro_enodeb_id2.menbid_macro_enodeb_id;
		context->uli.macro_enodeb_id2.menbid_macro_enb_id2 =
			uli->macro_enodeb_id2.menbid_macro_enb_id2;

	}

	if (uli->extnded_macro_enb_id) {
		context->uli.extnded_macro_enb_id = uli->extnded_macro_enb_id;
		context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_1 =
			uli->extended_macro_enodeb_id2.emenbid_mcc_digit_1;
		context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_3 =
			uli->extended_macro_enodeb_id2.emenbid_mnc_digit_3;
		context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_3 =
			uli->extended_macro_enodeb_id2.emenbid_mcc_digit_3;
		context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_2 =
			uli->extended_macro_enodeb_id2.emenbid_mnc_digit_2;
		context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_1 =
			uli->extended_macro_enodeb_id2.emenbid_mnc_digit_1;
		context->uli.extended_macro_enodeb_id2.emenbid_smenb =
			uli->extended_macro_enodeb_id2.emenbid_smenb;
		context->uli.extended_macro_enodeb_id2.emenbid_spare =
			uli->extended_macro_enodeb_id2.emenbid_spare;
		context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id =
			uli->extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id;
		context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2 =
			uli->extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2;
	}

	return 0;
}

/**
 * @brief  : Fill ue context info from incoming data in create sess request
 * @param  : csr holds data in csr
 * @param  : context , pointer to ue context structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
fill_context_info(create_sess_req_t *csr, ue_context_t *context)
{
 	if (cp_config->cp_type == SAEGWC) {
	    /* Check ntohl case */
	    //context->s11_sgw_gtpc_ipv4.s_addr = ntohl(cp_config->s11_ip.s_addr);
	    context->s11_sgw_gtpc_ipv4.s_addr = cp_config->s11_ip.s_addr;
	    context->s11_mme_gtpc_teid = csr->sender_fteid_ctl_plane.teid_gre_key;
	    context->s11_mme_gtpc_ipv4.s_addr = csr->sender_fteid_ctl_plane.ipv4_address;
	}


	/* VS: Stored the serving network information in UE context */
	context->serving_nw.mnc_digit_1 = csr->serving_network.mnc_digit_1;
	context->serving_nw.mnc_digit_2 = csr->serving_network.mnc_digit_2;
	context->serving_nw.mnc_digit_3 = csr->serving_network.mnc_digit_3;
	context->serving_nw.mcc_digit_1 = csr->serving_network.mcc_digit_1;
	context->serving_nw.mcc_digit_2 = csr->serving_network.mcc_digit_2;
	context->serving_nw.mcc_digit_3 = csr->serving_network.mcc_digit_3;

    if (cp_config->cp_type == SAEGWC) {
        if(csr->indctn_flgs.header.len != 0) {
            context->indication_flag.oi = csr->indctn_flgs.indication_oi;
        }
    }

	return 0;
}
/**
 * @brief  : Fill pdn info from data in incoming csr
 * @param  : csr holds data in csr
 * @param  : pdn , pointer to pdn connction structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
fill_pdn_info(create_sess_req_t *csr, pdn_connection_t *pdn)
{

	pdn->apn_ambr.ambr_downlink = csr->apn_ambr.apn_ambr_dnlnk;
	pdn->apn_ambr.ambr_uplink = csr->apn_ambr.apn_ambr_uplnk;
	pdn->apn_restriction = csr->max_apn_rstrct.rstrct_type_val;

	if (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4)
		pdn->pdn_type.ipv4 = 1;
	else if (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV6)
		pdn->pdn_type.ipv6 = 1;
	else if (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4V6) {
		pdn->pdn_type.ipv4 = 1;
		pdn->pdn_type.ipv6 = 1;
	}

	if (csr->chrgng_char.header.len)
		memcpy(&pdn->charging_characteristics,
				&csr->chrgng_char.chrgng_char_val,
				sizeof(csr->chrgng_char.chrgng_char_val));

	pdn->ue_time_zone_flag = FALSE;
	if(csr->ue_time_zone.header.len)
	{
		pdn->ue_time_zone_flag = TRUE;
		pdn->ue_tz.tz = csr->ue_time_zone.time_zone;
		pdn->ue_tz.dst = csr->ue_time_zone.daylt_svng_time;
	}

	if(csr->rat_type.header.len)
	{
		pdn->rat_type = csr->rat_type.rat_type;
	}

	if (cp_config->cp_type == SAEGWC) {
		pdn->s5s8_sgw_gtpc_ipv4 = cp_config->s5s8_ip;
		pdn->s5s8_sgw_gtpc_ipv4.s_addr = ntohl(pdn->s5s8_sgw_gtpc_ipv4.s_addr);
		pdn->s5s8_pgw_gtpc_ipv4.s_addr = csr->pgw_s5s8_addr_ctl_plane_or_pmip.ipv4_address;

	} else if (cp_config->cp_type == PGWC){
		pdn->s5s8_pgw_gtpc_ipv4 = cp_config->s5s8_ip;
		pdn->s5s8_pgw_gtpc_ipv4.s_addr = ntohl(pdn->s5s8_pgw_gtpc_ipv4.s_addr); //NIKHIL
		pdn->s5s8_sgw_gtpc_ipv4.s_addr = csr->sender_fteid_ctl_plane.ipv4_address;

		/* Note: s5s8_pgw_gtpc_teid generated from
		 * s5s8_pgw_gtpc_base_teid and incremented
		 * for each pdn connection, similar to
		 * s11_sgw_gtpc_teid
		 */
		set_s5s8_pgw_gtpc_teid(pdn);
		/* Note: s5s8_sgw_gtpc_teid =
		 *                  * s11_sgw_gtpc_teid
		 *                                   */
		pdn->s5s8_sgw_gtpc_teid = csr->sender_fteid_ctl_plane.teid_gre_key;


	}

	//pdn->s5s8_pgw_gtpc_teid = csr->s5s8pgw_pmip.teid_gre;

	return 0;
}

/**
 * @brief  : Fill bearer info from incoming data in csr
 * @param  : csr holds data in csr
 * @param  : bearer , pointer to eps bearer structure
 * @param  : context , pointer to ue context structure
 * @param  : pdn , pointer to pdn connction structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
fill_bearer_info(create_sess_req_t *csr, eps_bearer_t *bearer,
		ue_context_t *context, pdn_connection_t *pdn)
{
	/* Need to re-vist this ARP[Allocation/Retention priority] handling portion */
	bearer->qos.arp.priority_level =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.pl;
	bearer->qos.arp.preemption_capability =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.pci;
	bearer->qos.arp.preemption_vulnerability =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.pvi;

	/* TODO: Implement TFTs on default bearers
	 * if (create_session_request.bearer_tft_ie) {
	 * }**/

	/* VS: Fill the QCI value */
	bearer->qos.qci =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.qci;
	bearer->qos.ul_mbr =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.max_bit_rate_uplnk;
	bearer->qos.dl_mbr =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.max_bit_rate_dnlnk;
	bearer->qos.ul_gbr =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.guarntd_bit_rate_uplnk;
	bearer->qos.dl_gbr =
		csr->bearer_contexts_to_be_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk;

	bearer->s1u_sgw_gtpu_teid = 0;
	bearer->s5s8_sgw_gtpu_teid = 0;

	if (cp_config->cp_type == PGWC){
		bearer->s5s8_sgw_gtpu_ipv4.s_addr = csr->bearer_contexts_to_be_created.s5s8_u_sgw_fteid.ipv4_address;
		bearer->s5s8_sgw_gtpu_teid = csr->bearer_contexts_to_be_created.s5s8_u_sgw_fteid.teid_gre_key;
	}

/*
    //Just set number of URRs in bearer and generate urr id for this bearer urr. 
    bearer->urr_count = NUMBER_OF_URR_PER_BEARER;
    for(uint8_t itr=0; itr < bearer->urr_count;itr++) {
        bearer->urr_id[itr].urr_id = generate_urr_id();
    }
*/
	/* TODO: Revisit this for change in yang*/
/*
    if(cp_config->gx_enabled) {
        if (cp_config->cp_type != SGWC){
            bearer->qer_count = NUMBER_OF_QER_PER_BEARER;
            for(uint8_t itr=0; itr < bearer->qer_count; itr++){
                bearer->qer_id[itr].qer_id = generate_qer_id();
                fill_qer_entry(pdn, bearer,itr);
            }
        }
    }
*/

	/*SP: As per discussion Per bearer two pdrs and fars will be there*/
	/************************************************
	 *  cp_type  count      FTEID_1        FTEID_2 *
	 *************************************************
	 SGWC         2      s1u  SGWU      s5s8 SGWU
	 PGWC         2      s5s8 PGWU          NA
	 SAEGWC       2      s1u SAEGWU         NA
	 ************************************************/

	bearer->pdn = pdn;

	return 0;
}

void
fill_rule_and_qos_inform_in_pdn(pdn_connection_t *pdn)
{
	dynamic_rule_t *dynamic_rule = (dynamic_rule_t*)calloc(1, sizeof(dynamic_rule_t)); 
	uint8_t ebi_index = pdn->default_bearer_id - 5;
	eps_bearer_t *bearer = pdn->eps_bearers[ebi_index];

	pdn->policy.default_bearer_qos_valid = TRUE;
	bearer_qos_ie *def_qos = &pdn->policy.default_bearer_qos;

	def_qos->qci = QCI_VALUE;
	def_qos->arp.priority_level = GX_PRIORITY_LEVEL;
	def_qos->arp.preemption_capability = PREEMPTION_CAPABILITY_DISABLED;
	def_qos->arp.preemption_vulnerability = PREEMPTION_VALNERABILITY_ENABLED;

	bearer->qos.qci = QCI_VALUE;
	bearer->qos.arp.priority_level = GX_PRIORITY_LEVEL;
	bearer->qos.arp.preemption_capability = PREEMPTION_CAPABILITY_DISABLED;
	bearer->qos.arp.preemption_vulnerability = PREEMPTION_VALNERABILITY_ENABLED;

	memset(dynamic_rule->rule_name, '\0', sizeof(dynamic_rule->rule_name));
	strncpy(dynamic_rule->rule_name, RULE_NAME, RULE_LENGTH );

	dynamic_rule->online = ENABLE_ONLINE;
	dynamic_rule->offline = DISABLE_OFFLINE;
	dynamic_rule->flow_status = GX_ENABLE;
	dynamic_rule->precedence = PRECEDENCE;
	dynamic_rule->service_id = SERVICE_INDENTIFIRE;
	dynamic_rule->rating_group = RATING_GROUP;
	dynamic_rule->num_flw_desc = GX_FLOW_COUNT;

	for(uint8_t idx = 0; idx < GX_FLOW_COUNT; idx++) {
		strcpy(dynamic_rule->flow_desc[idx].sdf_flow_description,"permit out ip from 0.0.0.0/0 to assigned");
		dynamic_rule->flow_desc[idx].flow_direction = BIDIRECTIONAL;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.proto_id = PROTO_ID;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.local_ip_mask = LOCAL_IP_MASK;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.local_ip_addr.s_addr = LOCAL_IP_ADDR;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.local_port_low = PORT_LOW;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.local_port_high = PORT_HIGH;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.remote_ip_mask = REMOTE_IP_MASK;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.remote_ip_addr.s_addr = REMOTE_IP_ADDR;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.remote_port_low = PORT_LOW;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.remote_port_high = PORT_HIGH;
		dynamic_rule->flow_desc[idx].sdf_flw_desc.direction = TFT_DIRECTION_BIDIRECTIONAL;
	}

	dynamic_rule->qos.qci = QCI_VALUE;
	dynamic_rule->qos.arp.priority_level = GX_PRIORITY_LEVEL;
	dynamic_rule->qos.arp.preemption_capability = PREEMPTION_CAPABILITY_DISABLED;
	dynamic_rule->qos.arp.preemption_vulnerability = PREEMPTION_VALNERABILITY_ENABLED;
	dynamic_rule->qos.ul_mbr =  REQUESTED_BANDWIDTH_UL; // FIXME : we need bearer rate from profile
	dynamic_rule->qos.dl_mbr =  REQUESTED_BANDWIDTH_DL;
	dynamic_rule->qos.ul_gbr =  GURATEED_BITRATE_UL;
	dynamic_rule->qos.dl_gbr =  GURATEED_BITRATE_DL;
    pcc_rule_t *pcc_rule = (pcc_rule_t *)calloc(1, sizeof(pcc_rule_t)); 
    pcc_rule->action = RULE_ACTION_ADD; 
    pcc_rule->dyn_rule = dynamic_rule;
    TAILQ_INSERT_TAIL(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
}

/**
 * @brief  : Table 7.2.1-1: Information Elements in a Create Session Request -
 *           incomplete list
 */
struct parse_pgwc_s5s8_create_session_request_t {
	uint8_t *bearer_context_to_be_created_ebi;
	fteid_ie *s5s8_sgw_gtpc_fteid;
	gtpv2c_ie *apn_ie;
	gtpv2c_ie *apn_restriction_ie;
	gtpv2c_ie *imsi_ie;
	gtpv2c_ie *uli_ie;
	gtpv2c_ie *serving_network_ie;
	gtpv2c_ie *msisdn_ie;
	gtpv2c_ie *apn_ambr_ie;
	gtpv2c_ie *pdn_type_ie;
	gtpv2c_ie *charging_characteristics_ie;
	gtpv2c_ie *bearer_qos_ie;
	gtpv2c_ie *bearer_tft_ie;
	gtpv2c_ie *s5s8_sgw_gtpu_fteid;
};


/**
 * @brief  : parses gtpv2c message and populates parse_pgwc_s5s8_create_session_request_t structure
 * @param  : gtpv2c_rx
 *           buffer containing create bearer response message
 * @param  : csr
 *           data structure to contain required information elements from create
 *           create session response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */


static int
parse_pgwc_s5s8_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		struct parse_pgwc_s5s8_create_session_request_t *csr)
{
	gtpv2c_ie *current_ie;
	gtpv2c_ie *current_group_ie;
	gtpv2c_ie *limit_ie;
	gtpv2c_ie *limit_group_ie;

	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_ie, limit_ie)
	{
		if (current_ie->type == GTP_IE_BEARER_CONTEXT &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			FOR_EACH_GROUPED_IE(current_ie, current_group_ie,
					limit_group_ie)
			{
				if (current_group_ie->type == GTP_IE_EPS_BEARER_ID &&
					current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					csr->bearer_context_to_be_created_ebi =
					    IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t,
							    current_group_ie);
				} else if (current_group_ie->type ==
						GTP_IE_BEARER_QLTY_OF_SVC &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					csr->bearer_qos_ie = current_group_ie;
				} else if (current_group_ie->type == GTP_IE_EPS_BEARER_LVL_TRAFFIC_FLOW_TMPL &&
						current_group_ie->instance ==
							IE_INSTANCE_ZERO) {
					csr->bearer_tft_ie = current_group_ie;
				} else if (current_group_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
						current_group_ie->instance ==
						IE_INSTANCE_TWO) {
					csr->s5s8_sgw_gtpu_fteid = current_group_ie;
				}
			}

		} else if (current_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->s5s8_sgw_gtpc_fteid =
				IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie, current_ie);
		} else if (current_ie->type == GTP_IE_ACC_PT_NAME &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->apn_ie = current_ie;
		} else if (current_ie->type == GTP_IE_APN_RESTRICTION &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->apn_restriction_ie = current_ie;
		} else if (current_ie->type == GTP_IE_IMSI &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->imsi_ie = current_ie;
		} else if (current_ie->type == GTP_IE_AGG_MAX_BIT_RATE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->apn_ambr_ie = current_ie;
		} else if (current_ie->type == GTP_IE_PDN_TYPE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->pdn_type_ie = current_ie;
		} else if (current_ie->type == GTP_IE_CHARGING_ID &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->charging_characteristics_ie = current_ie;
		} else if (current_ie->type == GTP_IE_MSISDN &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->msisdn_ie = current_ie;
		} else if (current_ie->type == GTP_IE_USER_LOC_INFO &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->uli_ie = current_ie;
		} else if (current_ie->type == GTP_IE_SERVING_NETWORK &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->serving_network_ie = current_ie;
		}
	}

	if (!csr->apn_ie
		|| !csr->apn_restriction_ie
		|| !csr->bearer_context_to_be_created_ebi
		|| !csr->s5s8_sgw_gtpc_fteid
		|| !csr->imsi_ie
		|| !csr->uli_ie
		|| !csr->serving_network_ie
		|| !csr->apn_ambr_ie
		|| !csr->pdn_type_ie
		|| !csr->bearer_qos_ie
		|| !csr->msisdn_ie
		|| !IE_TYPE_PTR_FROM_GTPV2C_IE(pdn_type_ie,
				csr->pdn_type_ie)->ipv4) {
		LOG_MSG(LOG_ERROR, "Dropping packet");
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;
	}
	if (IE_TYPE_PTR_FROM_GTPV2C_IE(pdn_type_ie, csr->pdn_type_ie)->ipv6) {
		LOG_MSG(LOG_ERROR, "IPv6 Not Yet Implemented - Dropping packet");
		return GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED;
	}
	return 0;
}

void 
process_pgwc_s5s8_create_session_request_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p", data);
    return;
}

int
process_pgwc_s5s8_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		struct in_addr *upf_ipv4, uint8_t proc)
{
	int ret;
	uint32_t sequence = 0;
	struct in_addr ue_ip;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	create_sess_req_t csr = {0};

	struct parse_pgwc_s5s8_create_session_request_t create_s5s8_session_request = { 0 };

	upf_context_t *upf_context = NULL;
	upf_context = (upf_context_t *)upf_context_entry_lookup((upf_ipv4->s_addr));

	if (upf_context == NULL) {
		LOG_MSG(LOG_DEBUG, "NO ENTRY FOUND IN UPF HASH [%u] for proc = %d ", upf_ipv4->s_addr, proc);
		return 0;
	}

	ret = parse_pgwc_s5s8_create_session_request(gtpv2c_rx,
			&create_s5s8_session_request);
	if (ret)
		return ret;

	apn_profile_t *apn_requested = match_apn_profile(
	    APN_PTR_FROM_APN_IE(create_s5s8_session_request.apn_ie),
	    ntohs(create_s5s8_session_request.apn_ie->length));

	if (!apn_requested)
		return GTPV2C_CAUSE_MISSING_UNKNOWN_APN;

	uint8_t ebi_index =
		*create_s5s8_session_request.bearer_context_to_be_created_ebi - 5;

	ret = acquire_ip(&ue_ip);
	if (ret)
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;

	/* Overload s11_sgw_gtpc_teid
	 * set s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid =
	 * key->ue_context_by_fteid_hash */
	uint64_t *imsi_val = (uint64_t *)IE_TYPE_PTR_FROM_GTPV2C_IE(uint64_t,
			create_s5s8_session_request.imsi_ie);
    uint8_t *apn_ie = (uint8_t *)(APN_PTR_FROM_APN_IE(create_s5s8_session_request.apn_ie));
    uint8_t apn_len = ntohs(create_s5s8_session_request.apn_ie->length);
	ret = create_ue_context(imsi_val,
			ntohs(create_s5s8_session_request.imsi_ie->length),
			*create_s5s8_session_request.bearer_context_to_be_created_ebi, &context, apn_ie, apn_len);
	if (ret)
		return ret;

	/* Store upf ipv4 address */
	//context->upf_ipv4 = *upf_ipv4;
	context->pdns[ebi_index]->upf_ipv4 = *upf_ipv4;

	if (create_s5s8_session_request.msisdn_ie) {
		memcpy(&context->msisdn,
		    IE_TYPE_PTR_FROM_GTPV2C_IE(uint64_t,
				    create_s5s8_session_request.msisdn_ie),
		    ntohs(create_s5s8_session_request.msisdn_ie->length));
	}

	pdn = context->eps_bearers[ebi_index]->pdn;
	{
		strcpy((char *)pdn->apn, (char *)(APN_PTR_FROM_APN_IE(create_s5s8_session_request.apn_ie))); 
        pdn->apn_len = ntohs(create_s5s8_session_request.apn_ie->length);

		pdn->apn_ambr = *IE_TYPE_PTR_FROM_GTPV2C_IE(ambr_ie,
		    create_s5s8_session_request.apn_ambr_ie);
		pdn->apn_restriction = *IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t,
		    create_s5s8_session_request.apn_restriction_ie);
		pdn->ipv4 = ue_ip;
		pdn->pdn_type = *IE_TYPE_PTR_FROM_GTPV2C_IE(pdn_type_ie,
		    create_s5s8_session_request.pdn_type_ie);
		if (create_s5s8_session_request.charging_characteristics_ie) {
			pdn->charging_characteristics =
				*IE_TYPE_PTR_FROM_GTPV2C_IE(
						charging_characteristics_ie,
						create_s5s8_session_request.
						charging_characteristics_ie);
		}

		pdn->s5s8_sgw_gtpc_ipv4 =
			create_s5s8_session_request.
			s5s8_sgw_gtpc_fteid->ip_u.ipv4;
		pdn->s5s8_sgw_gtpc_teid =
			create_s5s8_session_request.
			s5s8_sgw_gtpc_fteid->fteid_ie_hdr.teid_or_gre;

		pdn->s5s8_pgw_gtpc_ipv4 = cp_config->s5s8_ip;

		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
						pdn->s5s8_sgw_gtpc_ipv4.s_addr;

		/* Note: s5s8_pgw_gtpc_teid generated from
		 * s5s8_pgw_gtpc_base_teid and incremented
		 * for each pdn connection, similar to
		 * s11_sgw_gtpc_teid
		 */
		set_s5s8_pgw_gtpc_teid(pdn);
	}
	bearer = context->eps_bearers[ebi_index];
	{
		/* TODO: Implement TFTs on default bearers
		if (create_s5s8_session_request.bearer_tft_ie) {
		}
		*/
		bearer->qos = *IE_TYPE_PTR_FROM_GTPV2C_IE(bearer_qos_ie,
		    create_s5s8_session_request.bearer_qos_ie);

		bearer->s5s8_sgw_gtpu_ipv4 =
				IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
						create_s5s8_session_request.s5s8_sgw_gtpu_fteid)->
							ip_u.ipv4;
		bearer->s5s8_sgw_gtpu_teid =
				IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
						create_s5s8_session_request.s5s8_sgw_gtpu_fteid)->
							fteid_ie_hdr.teid_or_gre;

		bearer->s5s8_pgw_gtpu_ipv4.s_addr = htonl(upf_context->s5s8_pgwu_ip);

		set_s5s8_pgw_gtpu_teid(bearer, context);
		bearer->pdn = pdn;
	}

	/* Update UE State */
	pdn->state = PFCP_SESS_EST_REQ_SNT_STATE;
#ifdef TEMP
	/* VS: Allocate the memory for response
	 */
	resp = calloc(1, sizeof(struct x resp_info));

	/* Set create session response */
	resp->eps_bearer_id = *create_s5s8_session_request.bearer_context_to_be_created_ebi;
	//resp->seq = gtpv2c_rx->teid.has_teid.seq;
	//resp->s11_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
	//resp->context = context;
	resp->msg_type = GTP_CREATE_SESSION_REQ;
	resp->state = PFCP_SESS_EST_REQ_SNT_STATE;
	resp->proc = proc;
#endif

	if (create_s5s8_session_request.uli_ie) {
		csr.uli = *(gtp_user_loc_info_ie_t *)create_s5s8_session_request.uli_ie;
	}

	if (create_s5s8_session_request.serving_network_ie) {
		csr.serving_network = *(gtp_serving_network_ie_t *)create_s5s8_session_request.serving_network_ie;

		/* VS: Remove the following assignment when support libgtpv2c */
		/* VS: Stored the serving network information in UE context */
		context->serving_nw.mnc_digit_1 = csr.serving_network.mnc_digit_1;
		context->serving_nw.mnc_digit_2 = csr.serving_network.mnc_digit_2;
		context->serving_nw.mnc_digit_3 = csr.serving_network.mnc_digit_3;
		context->serving_nw.mcc_digit_1 = csr.serving_network.mcc_digit_1;
		context->serving_nw.mcc_digit_2 = csr.serving_network.mcc_digit_2;
		context->serving_nw.mcc_digit_3 = csr.serving_network.mcc_digit_3;
	}

	pfcp_sess_estab_req_t pfcp_sess_est_req = {0};

	/* Below Passing 3rd Argumt. as NULL to reuse the fill_pfcp_sess_estab*/
	context->pdns[ebi_index]->seid = SESS_ID(pdn->s5s8_pgw_gtpc_teid, bearer->eps_bearer_id);
	/* Merge conflict
	context->seid = SESS_ID(pdn->s5s8_pgw_gtpc_teid, bearer->eps_bearer_id);
	*/
	sequence = get_pfcp_sequence_number(PFCP_SESSION_ESTABLISHMENT_REQUEST, sequence);
	fill_pfcp_sess_est_req(&pfcp_sess_est_req, context->pdns[ebi_index], sequence);
    LOG_MSG(LOG_DEBUG,"fill_pfcp_sess_est_req complete");

	/* Filling PDN structure*/
	pfcp_sess_est_req.pdn_type.header.type = PFCP_IE_PDN_TYPE;
	pfcp_sess_est_req.pdn_type.header.len = sizeof(uint8_t);
	pfcp_sess_est_req.pdn_type.pdn_type_spare = 0;

	if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4 && pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV6 ) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4V6;
	} else if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4 ;
	} else if (pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV4V6) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV6;
	}

	uint8_t pfcp_msg[MAX_PFCP_MSG_SIZE]={0};
	int encoded = encode_pfcp_sess_estab_req_t(&pfcp_sess_est_req, pfcp_msg);
	LOG_MSG(LOG_ERROR, "encoded PFCP message has size %d ",encoded);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	/*Send the packet to PGWU*/
	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSESTREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pgwc_s5s8_create_session_request_pfcp_timeout);
    pdn->trans_entry = trans_entry;

	return 0;
}

/**
 * @brief  : from parameters, populates gtpv2c message 'create session response' and
 *           populates required information elements as defined by
 *           clause 7.2.2 3gpp 29.274
 * @param  : gtpv2c_tx
 *           transmission buffer to contain 'create session response' message
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : context
 *           UE Context data structure pertaining to the session to be created
 * @param  : pdn
 *           PDN Connection data structure pertaining to the session to be created
 * @param  : bearer
 *           Default EPS Bearer corresponding to the PDN Connection to be created
 * @return : returns nothing
 */
void
set_pgwc_s5s8_create_session_response(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, pdn_connection_t *pdn,
		eps_bearer_t *bearer)
{

	set_gtpv2c_teid_header(gtpv2c_tx, GTP_CREATE_SESSION_RSP,
	    pdn->s5s8_sgw_gtpc_teid, sequence);

	set_cause_accepted_ie(gtpv2c_tx, IE_INSTANCE_ZERO);
	set_ipv4_fteid_ie(gtpv2c_tx, GTPV2C_IFTYPE_S5S8_PGW_GTPC,
			IE_INSTANCE_ONE,
			pdn->s5s8_pgw_gtpc_ipv4, htonl(pdn->s5s8_pgw_gtpc_teid));
	set_ipv4_paa_ie(gtpv2c_tx, IE_INSTANCE_ZERO, pdn->ipv4);
	set_apn_restriction_ie(gtpv2c_tx, IE_INSTANCE_ZERO,
			pdn->apn_restriction);
	{
		gtpv2c_ie *bearer_context_group =
				create_bearer_context_ie(gtpv2c_tx,
		    IE_INSTANCE_ZERO);
		add_grouped_ie_length(bearer_context_group,
		    set_ebi_ie(gtpv2c_tx, IE_INSTANCE_ZERO,
				    bearer->eps_bearer_id));
		add_grouped_ie_length(bearer_context_group,
		    set_cause_accepted_ie(gtpv2c_tx, IE_INSTANCE_ZERO));

		add_grouped_ie_length(bearer_context_group,
			set_bearer_qos_ie(gtpv2c_tx, IE_INSTANCE_ZERO,
			bearer));

		add_grouped_ie_length(bearer_context_group,
	    set_ipv4_fteid_ie(gtpv2c_tx, GTPV2C_IFTYPE_S5S8_PGW_GTPU,
			    IE_INSTANCE_ZERO, bearer->s5s8_pgw_gtpu_ipv4,
			    htonl(bearer->s5s8_pgw_gtpu_teid)));
	}
}

void
process_gx_ccri_timeout(void *data)
{
    LOG_MSG(LOG_ERROR, "Gx CCRI timeout");
    proc_context_t *proc_context = (proc_context_t *)data;
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    SET_PROC_MSG(proc_context,msg);
    msg->proc_context = proc_context;
    msg->event = GX_CCRI_TIMEOUT_EVNT;
    proc_context->handler(proc_context, msg); 
    return;
}

int
gen_ccr_request(proc_context_t *proc_context, uint8_t ebi_index, create_sess_req_t *csr)
{
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
    ue_context_t *context = (ue_context_t *)proc_context->ue_context;

	/* VS: Generate unique call id per PDN connection */
	context->pdns[ebi_index]->call_id = generate_call_id();

	/* Generate unique session id for communicate over the Gx interface */
	if (gen_sess_id_for_ccr(context->pdns[ebi_index]->gx_sess_id,
				context->pdns[ebi_index]->call_id)) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}

	/* VS: Maintain the PDN mapping with call id */
	if (add_pdn_conn_entry(context->pdns[ebi_index]->call_id,
				context->pdns[ebi_index]) != 0) {
		LOG_MSG(LOG_ERROR, "Failed to add pdn entry with call id");
		return -1;
	}

	/* VS: Set the Msg header type for CCR */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* VS: Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = INITIAL_REQUEST ;

	/* VG: Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = ESTABLISHMENT ;

	/* VS:TODO: Need to check the bearer identifier value */
	ccr_request.data.ccr.presence.bearer_identifier = PRESENT ;
	ccr_request.data.ccr.bearer_identifier.len =
		int_to_str((char *)ccr_request.data.ccr.bearer_identifier.val,
				(context->eps_bearers[ebi_index])->eps_bearer_id);

	/* Subscription-Id */
	if(csr->imsi.header.len  || csr->msisdn.header.len)
	{
		uint8_t idx = 0;
		ccr_request.data.ccr.presence.subscription_id = PRESENT;
		ccr_request.data.ccr.subscription_id.count = 1; // IMSI & MSISDN
		ccr_request.data.ccr.subscription_id.list  = (GxSubscriptionId *)calloc(1, sizeof(GxSubscriptionId)*1);

		/* Fill IMSI */
		if(csr->imsi.header.len != 0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].presence.subscription_id_type = PRESENT;
			ccr_request.data.ccr.subscription_id.list[idx].presence.subscription_id_data = PRESENT;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_IMSI;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len = csr->imsi.header.len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&csr->imsi.imsi_number_digits,
					csr->imsi.header.len);
			idx++;
		}

#if 0
		/* Fill MSISDN */
		if(csr->msisdn.header.len !=0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_E164;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len = csr->msisdn.header.len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&csr->msisdn.msisdn_number_digits,
					csr->msisdn.header.len);
		}
#endif
	}

	ccr_request.data.ccr.presence.network_request_support = PRESENT;
	ccr_request.data.ccr.network_request_support = NETWORK_REQUEST_SUPPORTED;

    uint32_t ip = htonl(context->pdns[ebi_index]->ipv4.s_addr);
	ccr_request.data.ccr.presence.framed_ip_address = PRESENT;
	memcpy(ccr_request.data.ccr.framed_ip_address.val, &ip, 4); 
	ccr_request.data.ccr.framed_ip_address.len = 4;


	/*
	 * nEED TO ADd following to Complete CCR_I, these are all mandatory IEs
	 * AN-GW Addr (SGW)
	 * User Eqip info (IMEI)
	 * 3GPP-ULI
	 * calling station id (APN)
	 * Access n/w charging addr (PGW addr)
	 * Charging Id
	 */


	/* VS: Fill the Credit Crontrol Request to send PCRF */
	if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, context->pdns[ebi_index]->gx_sess_id) != 0) {
		LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
		return -1;
	}

	struct sockaddr_in saddr_in;
   	saddr_in.sin_family = AF_INET;
   	inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_I, saddr_in.sin_addr.s_addr);


	/* Update UE State */
	context->pdns[ebi_index]->state = CCR_SNT_STATE;

	/* VS: Maintain the Gx context mapping with Gx Session id */
	if (gxsessid_context_entry_add((uint8_t*)context->pdns[ebi_index]->gx_sess_id, context) < 0) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_ccr_calc_length(&ccr_request.data.ccr);
	buffer = (char *)calloc(1, msg_len + sizeof(ccr_request.msg_type)+sizeof(ccr_request.seq_num));
	if (buffer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
		return -1;
	}

	/* VS: Fill the CCR header values */
	memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));
	memcpy(buffer+sizeof(ccr_request.msg_type), &ccr_request.seq_num, sizeof(ccr_request.seq_num));

	if (gx_ccr_pack(&(ccr_request.data.ccr),
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msg_len) == 0) {
		LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	uint32_t seq_num = gx_send(my_sock.gx_app_sock, buffer,
			           msg_len + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));

	transData_t *trans_entry = start_response_wait_timer(proc_context, NULL, 0, process_gx_ccri_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    trans_entry->sequence = seq_num;
    // trans_entry->peer_sockaddr = 0; no retransmission support
    add_gx_transaction(seq_num, (void*)trans_entry);  
    proc_context->gx_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	return 0;
}

/*
This function Handles the msgs received from PCEF
*/
int
cca_msg_handler(proc_context_t *proc_context, msg_info_t *msg)
{
    int ret = 0;
	pdn_connection_t *pdn = NULL;

	if (msg->rx_msg.cca.cc_request_type != INITIAL_REQUEST) {
		LOG_MSG(LOG_ERROR, "Expected GX CCAI and received GX CCR-T Response..!! ");
        gx_msg_proc_failure(proc_context);
		return 0;
	}

	ret = parse_gx_cca_msg(&msg->rx_msg.cca, &pdn);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Failed to establish IPCAN session, Send Failed CSResp back to MME");
        gx_msg_proc_failure(proc_context);
		return ret;
	}
    assert(proc_context->pdn_context == pdn);

    msg->event = PFCP_SESS_EST_EVNT;
    proc_context->handler(proc_context, msg); 
	return 0;
}

int
process_gx_ccai_reject_handler(proc_context_t *proc_context, msg_info_t *msg)
{
	LOG_MSG(LOG_ERROR, "Gx IP Can session setup failed %p ", msg);
    proc_initial_attach_failure(proc_context, GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER);
    return 0;
}

transData_t*
process_pfcp_sess_est_request(proc_context_t *proc_context, upf_context_t *upf_ctx)
{
	eps_bearer_t *bearer = NULL;
	pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
	ue_context_t *context = (ue_context_t *)proc_context->ue_context;
	pfcp_sess_estab_req_t pfcp_sess_est_req = {0};
	uint32_t sequence = 0;
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;

	bearer = pdn->eps_bearers[pdn->default_bearer_id - 5];
	if (cp_config->cp_type == SAEGWC) {
		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	} else if (cp_config->cp_type == PGWC){
		set_s5s8_pgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	}

	sequence = get_pfcp_sequence_number(PFCP_SESSION_ESTABLISHMENT_REQUEST, sequence);

	if (cp_config->cp_type == PGWC) {
		/* VS: Update the PGWU IP address */
		bearer->s5s8_pgw_gtpu_ipv4.s_addr =
			htonl(upf_ctx->s5s8_pgwu_ip);

		/* Filling PDN structure*/
		pfcp_sess_est_req.pdn_type.header.type = PFCP_IE_PDN_TYPE;
		pfcp_sess_est_req.pdn_type.header.len = sizeof(uint8_t);
		pfcp_sess_est_req.pdn_type.pdn_type_spare = 0;
		pfcp_sess_est_req.pdn_type.pdn_type =  1;
	} else {
		bearer->s5s8_sgw_gtpu_ipv4.s_addr = htonl(upf_ctx->s5s8_sgwu_ip);
		bearer->s1u_sgw_gtpu_ipv4.s_addr = htonl(upf_ctx->s1u_ip);
	}

	fill_pfcp_sess_est_req(&pfcp_sess_est_req, pdn, sequence);

	/* Update PDN State */
	pdn->state = PFCP_SESS_EST_REQ_SNT_STATE;

	proc_context->state = PFCP_SESS_EST_REQ_SNT_STATE;

    transData_t *trans_entry = NULL;
	uint8_t pfcp_msg[MAX_PFCP_MSG_SIZE]={0};
	int encoded = encode_pfcp_sess_estab_req_t(&pfcp_sess_est_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    /*pfcp-session-estab-req-sent*/
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSESTREQ,GET_UPF_ADDR(context->upf_context)); 

    trans_entry = start_response_wait_timer(proc_context, pfcp_msg, encoded, process_pfcp_sess_est_request_timeout);
    SET_TRANS_SELF_INITIATED(trans_entry);
    trans_entry->sequence = sequence;
    trans_entry->peer_sockaddr = context->upf_context->upf_sockaddr; 
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  

    if (add_sess_entry_seid(pdn->seid, context) != 0) {
        LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH");
        assert(0);
    }

    proc_context->pfcp_trans = trans_entry;
    trans_entry->proc_context = (void *)proc_context;

	return trans_entry;
}

void
process_pfcp_sess_est_request_timeout(void *data)
{
    LOG_MSG(LOG_ERROR, "PFCP Session Establishment Request timeout");
    proc_context_t *proc_context = (proc_context_t *)data;
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    SET_PROC_MSG(proc_context,msg);
    msg->proc_context = proc_context;
    msg->event = PFCP_SESS_EST_RESP_TIMEOUT_EVNT;
    proc_context->handler(proc_context, msg); 
    return;
}
