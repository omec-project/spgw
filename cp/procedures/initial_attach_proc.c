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
#include "initial_attach_proc.h"
#include "pfcp_association_setup_proc.h"
#include "tables/tables.h"
#include "rte_errno.h"
#include "ipc_api.h"
#include "util.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

extern int s11logger;
extern int s5s8logger;
extern udp_sock_t my_sock;

static uint32_t s5s8_sgw_gtpc_teid_offset;
extern const uint32_t s5s8_sgw_gtpc_base_teid; /* 0xE0FFEE */

/* local functions. Need to find suitable place for declarations  */

static int
fill_uli_info(gtp_user_loc_info_ie_t *uli, ue_context_t *context);

static int
fill_context_info(create_sess_req_t *csr, ue_context_t *context);

static int
fill_pdn_info(create_sess_req_t *csr, pdn_connection_t *pdn);

static int
fill_bearer_info(create_sess_req_t *csr, eps_bearer_t *bearer,
		ue_context_t *context, pdn_connection_t *pdn);

void
fill_rule_and_qos_inform_in_pdn(pdn_connection_t *pdn);

proc_context_t*
alloc_initial_proc(msg_info_t *msg)
{
    proc_context_t *csreq_proc;
    csreq_proc = calloc(1, sizeof(proc_context_t));
    csreq_proc->proc_type = msg->proc; 
    csreq_proc->handler = initial_attach_event_handler;
    msg->proc_context = csreq_proc;

    return csreq_proc;
}

void 
initial_attach_event_handler(void *proc, uint32_t event, void *data)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    switch(event) {
        case CS_REQ_RCVD_EVNT: {
            msg_info_t *msg = (msg_info_t *)data;
            handle_csreq_msg(proc_context, msg);
            break;
        }
        case PFCP_SESS_EST_RESP_RCVD_EVNT: {
            process_sess_est_resp_handler(data, NULL);
            break; 
        }
        default:
            assert(0); // unknown event 
    }
    return;
}

int
handle_csreq_msg(proc_context_t *csreq_proc, msg_info_t *msg)
{
    int ret = 0;
	ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

	ret = process_create_sess_req(&msg->gtpc_msg.csr,
			                      &context, &pdn, msg);
	if (ret) {
        // > 0 cause - Send reject message out 
        // -1 : just cleanup call locally 
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Error: %d \n",
				__file__, __func__, __LINE__, ret);
        proc_initial_attach_failure(msg, ret);
		return -1;
	}
    csreq_proc->ue_context = (void *)context;
    csreq_proc->pdn_context = (void *)pdn;
    context->current_proc = csreq_proc;
    
#ifdef TEMP
	if (pdn->upf_ipv4.s_addr == 0) {
#ifdef USE_DNS_QUERY
		uint32_t *upf_ip = NULL;
		upf_ip = &upf_ipv4.s_addr;

		/* VS: Select the UPF based on DNS */
		ret = dns_query_lookup(pdn, &upf_ip);
		if (ret) {
			clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Error: %d \n",
					__file__, __func__, __LINE__, ret);
			return ret;
		}

		pdn->upf_ipv4.s_addr = *upf_ip;
#else
		// if name is nit already resolved and no DNS enabled then use the configured
		// upf address 
		pdn->upf_ipv4.s_addr = cp_config->upf_pfcp_ip.s_addr;
#endif /* USE_DNS_QUERY */
	}
#endif
	upf_context_t *upf_context = context->upf_context;
    if (upf_context->state == PFCP_ASSOC_RESP_RCVD_STATE) {
        printf("UPF association already formed \n");
        transData_t *pfcp_trans = process_pfcp_sess_est_request(pdn, upf_context);
        if (pfcp_trans == NULL) {
            clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: pfcp send error \n",
                    __func__, __LINE__);
            proc_initial_attach_failure(msg, GTPV2C_CAUSE_INVALID_PEER);
            return -1;
        }
        csreq_proc->pfcp_trans = pfcp_trans;
        return 0;
    } else {
        // create PFCP association setup proccedure 
        // we are not modifying original msg content. This msg has ue/pdn context
        proc_context_t *proc_context = alloc_pfcp_association_setup_proc(msg); 
        proc_context->handler((void *)proc_context, (uint32_t)PFCP_ASSOCIATION_SETUP, (void *)msg);
    }
    return 0;
}

// return values > 0 : - GTP cause 
// -1 : context replacement, mis error 
// 0 : success
int
process_create_sess_req(create_sess_req_t *csr,
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
    struct in_addr upf_ipv4 = {0};
    /* ajay - Should we get default context ?*/
    upf_context_t *upf_context=NULL; 

	apn_profile_t *apn_requested = match_apn_profile((char *)csr->apn.apn, csr->apn.header.len);

    if(apn_requested == NULL) {
        // caller sends out csrsp 
        return GTPV2C_CAUSE_MISSING_UNKNOWN_APN; 
    }
// DNS would need changes here 
    /* TODO - IE presense should be validated before accessing them */
    sub_profile_t *sub_prof=NULL;
    sub_selection_keys_t dpkey = {0}; 

    dpkey.imsi.is_valid = true;
    dpkey.imsi.from_imsi = csr->imsi.imsi64; 
    dpkey.imsi.to_imsi =  csr->imsi.imsi64;
    
    dpkey.plmn.is_valid = true;
    dpkey.plmn.tac = csr->uli.tai2.tai_tac;
    memcpy((void *)(&dpkey.plmn.plmn[0]), (void *)(&csr->uli.tai2), 3);

    clLog(s11logger, eCLSeverityTrace, "Create Session Request ULI mcc %d %d %d " 
                " mnc %d %d %d \n", csr->uli.tai2.tai_mcc_digit_1, csr->uli.tai2.tai_mcc_digit_2, 
                csr->uli.tai2.tai_mcc_digit_3, csr->uli.tai2.tai_mnc_digit_1, 
                csr->uli.tai2.tai_mnc_digit_2, csr->uli.tai2.tai_mnc_digit_3);
    
    upf_context = get_upf_context_for_key(&dpkey, &sub_prof); 
    
    // no upf available 
    if(upf_context == NULL) 
    {
        // caller sends out csrsp 
        return GTPV2C_CAUSE_REQUEST_REJECTED ; 
    }

    upf_ipv4 = upf_context->upf_sockaddr.sin_addr;

    printf("Selected UPF address  %s \n", inet_ntoa(upf_ipv4));
	if(csr->mapped_ue_usage_type.header.len > 0) {
		apn_requested->apn_usage_type = csr->mapped_ue_usage_type.mapped_ue_usage_type;
	}

    /* Requirementss  Prio-5. New networks support all the 15 EBIs  */
	uint8_t ebi_index = csr->bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;

    ret = 0;
    if (cp_config->cp_type != SGWC) {
        struct in_addr *paa_ipv4 = (struct in_addr *) &csr->paa.pdn_addr_and_pfx[0];
        if (csr->paa.pdn_type == PDN_IP_TYPE_IPV4 && paa_ipv4->s_addr != 0) {
            bool found = false;
#ifdef STATIC_ADDR_ALLOC
#ifdef MULTI_UPFS
            if (dpInfo != NULL)
                found = reserve_ip_node(dpInfo->static_pool_tree, *paa_ipv4);
#else
            found = reserve_ip_node(static_addr_pool, *paa_ipv4);
#endif
#endif
            if (found == false) {
                clLog(sxlogger, eCLSeverityCritical,"Received CSReq with static address %s"
                        " . Invalid address received \n",
                        inet_ntoa(*paa_ipv4));
                // caller sends out csrsp 
                return GTPV2C_CAUSE_REQUEST_REJECTED;
            }
            ue_ip = *paa_ipv4;

            /* we want ue_ip in network order. To keep code aligned with dynamic
             * allocation  */
            ue_ip.s_addr = htonl(ue_ip.s_addr); 
            static_addr_pdn = true;
        } else { 
		    ret = acquire_ip(&ue_ip);
        }
    }
	if (ret) {
        // caller sends out csrsp 
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
    }

	printf("[%s] : %d -  Acquire ip = %s \n", __FUNCTION__, __LINE__, inet_ntoa(ue_ip));
	/* set s11_sgw_gtpc_teid= key->ue_context_by_fteid_hash */
	ret = create_ue_context(&csr->imsi.imsi_number_digits, csr->imsi.header.len,
			csr->bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi, &context, apn_requested,
			CSR_SEQUENCE(csr));
	if (ret) {
		return ret;
    }

    context->sub_prof = sub_prof;
	if (csr->mei.header.len)
		memcpy(&context->mei, &csr->mei.mei, csr->mei.header.len);

	memcpy(&context->msisdn, &csr->msisdn.msisdn_number_digits, csr->msisdn.header.len);

	if (fill_context_info(csr, context) != 0) {
			return -1;
    }

    if(csr->pco_new.header.len != 0) {
        printf("%s %d - PCO length = %d \n", __FUNCTION__, __LINE__, csr->pco_new.header.len);
        context->pco = calloc(1, sizeof(pco_ie_t));
        memcpy(context->pco, (void *)(&csr->pco_new), sizeof(pco_ie_t));
    }

	// TODOFIX - does not seem to be correct 
	if (cp_config->cp_type == PGWC) {
		context->s11_mme_gtpc_teid = csr->sender_fteid_ctl_plane.teid_gre_key;
    }

	/* Retrive procedure of CSR */
	pdn = context->eps_bearers[ebi_index]->pdn;
	pdn->proc = msg->proc; 
    if(static_addr_pdn == true)
        SET_PDN_ADDR_STATIC(pdn);

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

    
#if 0
	/* VS: Maintain the sequence number of CSR */
	pdn->apn_in_use = apn_requested;
#endif

    // DELETE THIS 
	/* Store upf ipv4 in pdn structure */
	pdn->upf_ipv4 = upf_ipv4;

	if (fill_pdn_info(csr, pdn) != 0)
		return -1;

	bearer = context->eps_bearers[ebi_index];

	if (cp_config->cp_type == SGWC) {
		pdn->ipv4.s_addr = htonl(ue_ip.s_addr);
		/* Note: s5s8_sgw_gtpc_teid =
		 *                  * s11_sgw_gtpc_teid
		 *                                   */
		//pdn->s5s8_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
		/* SGWC s55s8 TEID is unique for each PDN or PGWC */
		pdn->s5s8_sgw_gtpc_teid = s5s8_sgw_gtpc_base_teid + s5s8_sgw_gtpc_teid_offset;
		++s5s8_sgw_gtpc_teid_offset;

		context->pdns[ebi_index]->seid = SESS_ID(context->s11_sgw_gtpc_teid, bearer->eps_bearer_id);
	} else if (cp_config->cp_type == PGWC) {
		/* VS: Maitain the fqdn into table */
		memcpy(pdn->fqdn, (char *)csr->sgw_u_node_name.fqdn,
				csr->sgw_u_node_name.header.len);

		pdn->ipv4.s_addr = htonl(ue_ip.s_addr);
		context->pdns[ebi_index]->seid = SESS_ID(pdn->s5s8_pgw_gtpc_teid, bearer->eps_bearer_id);
	} else {
		pdn->ipv4.s_addr = htonl(ue_ip.s_addr);
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

    if ((cp_config->cp_type == PGWC) || (cp_config->cp_type == SAEGWC)) {
        if(cp_config->gx_enabled) {
            if (gen_ccr_request(context, ebi_index, csr)) {
                clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error: %s \n", __func__, __LINE__,
                        strerror(errno));
                return -1;
            }
        } else {
            fill_rule_and_qos_inform_in_pdn(pdn);
        }
	}

#ifdef USE_CSID
	/* Parse and stored MME and SGW FQ-CSID in the context */
	fqcsid_t *tmp = NULL;

	/* Allocate the memory for each session */
	context->mme_fqcsid = rte_zmalloc_socket(NULL, sizeof(fqcsid_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	context->sgw_fqcsid = rte_zmalloc_socket(NULL, sizeof(fqcsid_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (cp_config->cp_type != SAEGWC) {
		context->pgw_fqcsid = rte_zmalloc_socket(NULL, sizeof(fqcsid_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
		if (context->pgw_fqcsid == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Failed to allocate the memory for fqcsids entry\n",
					ERR_MSG);
			return -1;
		}
	}

	if ((context->mme_fqcsid == NULL) ||
			(context->sgw_fqcsid == NULL)) {
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"Failed to allocate the memory for fqcsids entry\n",
				ERR_MSG);
		return -1;
	}

	/* MME FQ-CSID */
	if (csr->mme_fqcsid.header.len) {
		/* Stored the MME CSID by MME Node address */
		tmp = get_peer_addr_csids_entry(csr->mme_fqcsid.node_address,
				ADD);
		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		/* check ntohl */
		tmp->node_addr = csr->mme_fqcsid.node_address;

		for(uint8_t itr = 0; itr < csr->mme_fqcsid.number_of_csids; itr++) {
			uint8_t match = 0;
			for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
				if (tmp->local_csid[itr1] == csr->mme_fqcsid.pdn_csid[itr])
					match = 1;
			}

			if (!match) {
				tmp->local_csid[tmp->num_csid++] =
					csr->mme_fqcsid.pdn_csid[itr];
			}
		}
		memcpy(context->mme_fqcsid, tmp, sizeof(fqcsid_t));
	} else {
		/* Stored the MME CSID by MME Node address */
		tmp = get_peer_addr_csids_entry(context->s11_mme_gtpc_ipv4.s_addr,
				ADD);
		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = context->s11_mme_gtpc_ipv4.s_addr;
		memcpy(context->mme_fqcsid, tmp, sizeof(fqcsid_t));
	}

	/* SGW FQ-CSID */
	if (csr->sgw_fqcsid.header.len) {
		/* Stored the SGW CSID by SGW Node address */
		if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
			tmp = get_peer_addr_csids_entry(csr->sgw_fqcsid.node_address,
					ADD);
		} else {
			/* PGWC */
			tmp = get_peer_addr_csids_entry(csr->sgw_fqcsid.node_address,
					ADD);
		}
		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = csr->sgw_fqcsid.node_address;

		for(uint8_t itr = 0; itr < csr->sgw_fqcsid.number_of_csids; itr++) {
			uint8_t match = 0;
			for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
				if (tmp->local_csid[itr1] == csr->sgw_fqcsid.pdn_csid[itr])
					match = 1;
			}
			if (!match) {
				tmp->local_csid[tmp->num_csid++] =
					csr->sgw_fqcsid.pdn_csid[itr];
			}
		}
		memcpy(context->sgw_fqcsid, tmp, sizeof(fqcsid_t));
	} else {
		if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
			tmp = get_peer_addr_csids_entry(context->s11_sgw_gtpc_ipv4.s_addr,
					ADD);
			if (tmp == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
						strerror(errno));
				return -1;
			}
			tmp->node_addr = ntohl(context->s11_sgw_gtpc_ipv4.s_addr);

			for(uint8_t itr = 0; itr < csr->sgw_fqcsid.number_of_csids; itr++) {
				uint8_t match = 0;
				for (uint8_t itr1 = 0; itr1 < tmp->num_csid; itr1++) {
					if (tmp->local_csid[itr1] == csr->sgw_fqcsid.pdn_csid[itr])
						match = 1;
				}
				if (!match) {
					tmp->local_csid[tmp->num_csid++] =
						csr->sgw_fqcsid.pdn_csid[itr];
				}
			}
			memcpy(context->sgw_fqcsid, tmp, sizeof(fqcsid_t));
		}
	}

	/* PGW FQ-CSID */
	if (cp_config->cp_type == PGWC) {
		tmp = get_peer_addr_csids_entry(pdn->s5s8_pgw_gtpc_ipv4.s_addr,
				ADD);
		if (tmp == NULL) {
			clLog(clSystemLog, eCLSeverityCritical, FORMAT"Error: %s \n", ERR_MSG,
					strerror(errno));
			return -1;
		}
		tmp->node_addr = pdn->s5s8_pgw_gtpc_ipv4.s_addr;
		memcpy(context->pgw_fqcsid, tmp, sizeof(fqcsid_t));
	}
#endif /* USE_CSID */

	/* VS: Store the context of ue in pdn*/
	pdn->context = context;

	/* VS: Return the UE context */
	*_context = context;
    *pdn_ctxt = pdn; 

    context->upf_context = upf_context;

    msg->upf_context = upf_context;
    msg->pdn_context = pdn;
    msg->ue_context  = context;

    pdn->upf_ipv4 = upf_ipv4; 
#ifdef SINGLE_UPF
	pdn->upf_ipv4.s_addr = cp_config->upf_pfcp_ip.s_addr;
#endif
    return 0;
}

int
process_sess_est_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
    proc_context_t *proc_context = msg->proc_context;

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_ESTABLISHMENT_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));


	if(msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value != REQUESTACCEPTED) {
		clLog(sxlogger, eCLSeverityDebug, "Cause received Est response is %d\n",
				msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value);
        proc_initial_attach_failure(msg, GTPV2C_CAUSE_REQUEST_REJECTED);
		return -1;
	}


	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_est_resp(msg,
			&msg->pfcp_msg.pfcp_sess_est_resp, gtpv2c_tx);
	//ret = process_pfcp_sess_est_resp(
	//		msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
	//		gtpv2c_tx,
	//		msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid);

	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
        proc_initial_attach_failure(msg, ret);
		return -1;
	}
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

#ifdef FUTURE_NEED
	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == PGWC)) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s5s8_recv_sockaddr,
				s5s8_sockaddr_len);


		update_cli_stats(s5s8_recv_sockaddr.sin_addr.s_addr,
							gtpv2c_tx->gtpc.message_type,SENT,S5S8);

		if (SGWC == cp_config->cp_type) {
			add_gtpv2c_if_timer_entry(
				UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid),
				&s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid) - 5,
				S5S8_IFACE);
		}

		//s5s8_sgwc_msgcnt++;
	} else {
#endif
        transData_t *trans_rec = proc_context->gtpc_trans;
		/* Send response on s11 interface */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &trans_rec->peer_sockaddr,
				sizeof(struct sockaddr_in));

		update_cli_stats(trans_rec->peer_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, ACC,S11);
    
        proc_initial_attach_complete(proc_context);
        
#ifdef FUTURE_NEED
	}
#endif
	RTE_SET_USED(unused_param);
	return 0;
}

/**
 * @brief  : Fill ULI information into UE context from CSR
 * @param  : uli is pointer to structure to store uli info
 * @param  : context is a pointer to ue context structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
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
static int
fill_context_info(create_sess_req_t *csr, ue_context_t *context)
{
 	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
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

    if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
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
static int
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

	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
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

	/*VS:TODO*/
	if (cp_config->cp_type == SGWC) {
		my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(csr->pgw_s5s8_addr_ctl_plane_or_pmip.ipv4_address);
	}

	/* Note: s5s8_pgw_gtpc_teid updated by
	 *                  * process_sgwc_s5s8_create_session_response (...)
	 *                                   */
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
static int
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

	/* TODO: Revisit this for change in yang*/
    if(cp_config->gx_enabled) {
        if (cp_config->cp_type != SGWC){
            bearer->qer_count = NUMBER_OF_QER_PER_BEARER;
            for(uint8_t itr=0; itr < bearer->qer_count; itr++){
                bearer->qer_id[itr].qer_id = generate_qer_id();
                fill_qer_entry(pdn, bearer,itr);
            }
        }
    }
	/*SP: As per discussion Per bearer two pdrs and fars will be there*/
	/************************************************
	 *  cp_type  count      FTEID_1        FTEID_2 *
	 *************************************************
	 SGWC         2      s1u  SGWU      s5s8 SGWU
	 PGWC         2      s5s8 PGWU          NA
	 SAEGWC       2      s1u SAEGWU         NA
	 ************************************************/

	if (cp_config->cp_type == SGWC){

		bearer->pdr_count = NUMBER_OF_PDR_PER_BEARER;
		for(uint8_t itr=0; itr < bearer->pdr_count; itr++){
			switch(itr){
				case SOURCE_INTERFACE_VALUE_ACCESS:
					fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_ACCESS, itr);
					break;
				case SOURCE_INTERFACE_VALUE_CORE:
					fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_CORE, itr);
					break;
				default:
					break;
			}
		}
	}

	bearer->pdn = pdn;

	RTE_SET_USED(context);
	return 0;
}

void
fill_rule_and_qos_inform_in_pdn(pdn_connection_t *pdn)
{
	dynamic_rule_t *dynamic_rule = dynamic_rule = &pdn->policy.pcc_rule[0].dyn_rule;
	uint8_t ebi_index = pdn->default_bearer_id - 5;
	eps_bearer_t *bearer = pdn->eps_bearers[ebi_index];

	pdn->policy.default_bearer_qos_valid = TRUE;
	bearer_qos_ie *def_qos = &pdn->policy.default_bearer_qos;

	pdn->policy.num_charg_rule_install = DEFAULT_RULE_COUNT;
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
	dynamic_rule->qos.ul_mbr =  REQUESTED_BANDWIDTH_UL;
	dynamic_rule->qos.dl_mbr =  REQUESTED_BANDWIDTH_DL;
	dynamic_rule->qos.ul_gbr =  GURATEED_BITRATE_UL;
	dynamic_rule->qos.dl_gbr =  GURATEED_BITRATE_DL;

}

void proc_initial_attach_failure(msg_info_t *msg, int cause)
{
    if(cause != -1) {
        // send cs response 
        cs_error_response(msg, cause,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
    }
    if(msg->proc_context != NULL && msg->ue_context == NULL)
    {
        proc_context_t *proc_context = msg->proc_context;
        if(proc_context->gtpc_trans != NULL) {
            printf("Delete gtpc procs \n");
            /* Only MME initiated transactions as of now */
            uint16_t port_num = proc_context->gtpc_trans->peer_sockaddr.sin_port; 
            uint32_t sender_addr = proc_context->gtpc_trans->peer_sockaddr.sin_addr.s_addr; 
            uint32_t seq_num = proc_context->gtpc_trans->sequence; 
            transData_t *gtpc_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
            assert(gtpc_trans == proc_context->gtpc_trans);
            stop_transaction_timer(proc_context->gtpc_trans);
            free(proc_context->gtpc_trans);
            proc_context->gtpc_trans = NULL;
        }
        msg->proc_context = NULL;
        free(proc_context); 
    }
    // cleanup call locally 
    process_error_occured_handler_new((void *)msg, NULL);
    return;
}

void proc_initial_attach_complete(proc_context_t *proc_context)
{
    transData_t *gtpc_trans = proc_context->gtpc_trans;
    ue_context_t *ue_context = proc_context->ue_context;

    uint16_t port_num = gtpc_trans->peer_sockaddr.sin_port; 
    uint32_t sender_addr = gtpc_trans->peer_sockaddr.sin_addr.s_addr; 
    uint32_t seq_num = gtpc_trans->sequence; 

    transData_t *temp_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
    assert(temp_trans != NULL);

    /* Let's cross check if transaction from the table is matchig with the one we have 
     * in subscriber 
     */
    assert(gtpc_trans == temp_trans);
    proc_context->gtpc_trans =  NULL;

    /* PFCP transaction is already complete. */
    assert(proc_context->pfcp_trans == NULL);
    free(gtpc_trans);
    free(proc_context);
    ue_context->current_proc = NULL;
    return;
}

#ifdef FUTURE_NEED
/**
 * @brief  : parses gtpv2c message and populates parse_sgwc_s5s8_create_session_response_t structure
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

int
parse_sgwc_s5s8_create_session_response(gtpv2c_header_t *gtpv2c_rx,
		struct parse_sgwc_s5s8_create_session_response_t *csr)
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
						IE_INSTANCE_ZERO) {
					csr->s5s8_pgw_gtpu_fteid = current_group_ie;
				}
			}

		} else if (current_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
				current_ie->instance == IE_INSTANCE_ONE) {
			csr->pgw_s5s8_gtpc_fteid =
				IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie, current_ie);
		} else if (current_ie->type == GTP_IE_PDN_ADDR_ALLOC &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->pdn_addr_alloc_ie = current_ie;
		} else if (current_ie->type == GTP_IE_APN_RESTRICTION &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			csr->apn_restriction_ie = current_ie;
		}
	}

	if (!csr->apn_restriction_ie
		|| !csr->bearer_context_to_be_created_ebi
		|| !csr->pgw_s5s8_gtpc_fteid) {
		clLog(clSystemLog, eCLSeverityCritical, "Dropping packet\n");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	return 0;
}

int
gen_sgwc_s5s8_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, pdn_connection_t *pdn,
		eps_bearer_t *bearer, char *sgwu_fqdn)
{

	gtpv2c_ie *current_rx_ie;
	gtpv2c_ie *limit_rx_ie;

	set_gtpv2c_teid_header(gtpv2c_tx, GTP_CREATE_SESSION_REQ,
		    0, sequence);

	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_rx_ie, limit_rx_ie)
	{
		if (current_rx_ie->type == GTP_IE_BEARER_CONTEXT &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			gtpv2c_ie *bearer_context_group =
				create_bearer_context_ie(gtpv2c_tx,
			    IE_INSTANCE_ZERO);
			add_grouped_ie_length(bearer_context_group,
			    set_ebi_ie(gtpv2c_tx, IE_INSTANCE_ZERO,
			    bearer->eps_bearer_id));
			add_grouped_ie_length(bearer_context_group,
				set_bearer_qos_ie(gtpv2c_tx, IE_INSTANCE_ZERO,
				bearer));
			add_grouped_ie_length(bearer_context_group,
			    set_ipv4_fteid_ie(gtpv2c_tx, GTPV2C_IFTYPE_S5S8_SGW_GTPU,
			    IE_INSTANCE_TWO, bearer->s5s8_sgw_gtpu_ipv4,
			    htonl(bearer->s5s8_sgw_gtpu_teid)));
		} else if (current_rx_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
				current_rx_ie->instance == IE_INSTANCE_ONE) {
			continue;
		} else if (current_rx_ie->type == GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ipv4_fteid_ie(gtpv2c_tx, GTPV2C_IFTYPE_S5S8_SGW_GTPC,
				IE_INSTANCE_ZERO,
				pdn->s5s8_sgw_gtpc_ipv4, htonl(pdn->s5s8_sgw_gtpc_teid));
		} else if (current_rx_ie->type == GTP_IE_ACC_PT_NAME &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_APN_RESTRICTION &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_IMSI &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_AGG_MAX_BIT_RATE &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_PDN_TYPE &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_CHRGNG_CHAR &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_INDICATION &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			continue;
		} else if (current_rx_ie->type == GTP_IE_MBL_EQUIP_IDNTY &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			continue;
		} else if (current_rx_ie->type == GTP_IE_MSISDN &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_USER_LOC_INFO &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_SERVING_NETWORK &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_RAT_TYPE &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_SELECTION_MODE &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_PDN_ADDR_ALLOC &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_PROT_CFG_OPTS &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		}
	}

	set_fqdn_ie(gtpv2c_tx, sgwu_fqdn);

	return 0;
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
		clLog(clSystemLog, eCLSeverityCritical, "%s:Dropping packet\n", __func__);
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;
	}
	if (IE_TYPE_PTR_FROM_GTPV2C_IE(pdn_type_ie, csr->pdn_type_ie)->ipv6) {
		clLog(clSystemLog, eCLSeverityCritical, "IPv6 Not Yet Implemented - Dropping packet\n");
		return GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED;
	}
	return 0;
}

void 
process_pgwc_s5s8_create_session_request_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
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
	ret = upf_context_entry_lookup((upf_ipv4->s_addr), &(upf_context));

	if (ret < 0) {
		clLog(s5s8logger, eCLSeverityDebug, "NO ENTRY FOUND IN UPF HASH [%u]\n", upf_ipv4->s_addr);
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
	ret = create_ue_context(imsi_val,
			ntohs(create_s5s8_session_request.imsi_ie->length),
			*create_s5s8_session_request.bearer_context_to_be_created_ebi, &context, apn_requested,
			0);
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
		pdn->apn_in_use = apn_requested;
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

	/* Update the sequence number */
	context->sequence = gtpv2c_rx->teid.has_teid.seq;

	/* Update UE State */
	pdn->state = PFCP_SESS_EST_REQ_SNT_STATE;
	pdn->proc = proc;

	/* VS: Allocate the memory for response
	 */
	resp = rte_malloc_socket(NULL,
					sizeof(struct x resp_info),
					RTE_CACHE_LINE_SIZE, rte_socket_id());

	/* Set create session response */
	resp->eps_bearer_id = *create_s5s8_session_request.bearer_context_to_be_created_ebi;
	//resp->seq = gtpv2c_rx->teid.has_teid.seq;
	//resp->s11_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
	//resp->context = context;
	resp->msg_type = GTP_CREATE_SESSION_REQ;
	resp->state = PFCP_SESS_EST_REQ_SNT_STATE;
	resp->proc = proc;

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

	/*Filling sequence number */
	//pfcp_sess_est_req.header.seid_seqno.has_seid.seq_no  = sequence;

	/* Filling PDN structure*/
	pfcp_sess_est_req.pdn_type.header.type = PFCP_IE_PDN_TYPE;
	pfcp_sess_est_req.pdn_type.header.len = sizeof(uint8_t);
	pfcp_sess_est_req.pdn_type.pdn_type_spare = 0;
	//pfcp_sess_est_req.pdn_type.pdn_type =  0;//Vikram TBD PFCP_PDN_IP_TYPE_IPV4; // create_s5s8_session_request.pdn_type_ie->type ;

	if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4 && pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV6 ) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4V6;
	} else if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4 ;
	} else if (pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV4V6) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV6;
	}

	if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4 && pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV6 ) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4V6;
	} else if (pdn->pdn_type.ipv4 == PDN_IP_TYPE_IPV4) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV4 ;
	} else if (pdn->pdn_type.ipv6 == PDN_IP_TYPE_IPV4V6) {
		pfcp_sess_est_req.pdn_type.pdn_type = PDN_IP_TYPE_IPV6;
	}


	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_estab_req_t(&pfcp_sess_est_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	/*Send the packet to PGWU*/
	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Error in sending CSR to PGW-U. err_no: %i\n", errno);
	} else {

		update_cli_stats((uint32_t)context->upf_context->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_est_req.header.message_type,SENT,SX);

        transData_t *trans_entry;
		trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, process_pgwc_s5s8_create_session_request_pfcp_timeout);
        pdn->trans_entry = trans_entry;
	}

	if (add_sess_entry_seid(context->pdns[ebi_index]->seid, resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, " %s %s %d Failed to add response in entry in SM_HASH\n",__file__,
				__func__, __LINE__);
		return -1;
	}

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

//int
//process_sgwc_s5s8_create_session_response(gtpv2c_header_t *gtpv2c_rx)
//{
//	int ret;
//	ue_context *context = NULL;
//	pdn_connection_t *pdn = NULL;
//	eps_bearer_t *bearer = NULL;
//	static uint32_t process_sgwc_s5s8_cs_rsp_cnt;
//	static uint32_t process_spgwc_s11_cs_res_cnt;
//
//	struct resp_info *resp = NULL;
//	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
//	//struct parse_sgwc_s5s8_create_session_response_t create_s5s8_session_response = {0};
//	//ret = parse_sgwc_s5s8_create_session_response(gtpv2c_rx,
//	//		&create_s5s8_session_response);
//
//	create_sess_rsp_t cs_rsp = {0};
//	ret = decode_create_sess_rsp((uint8_t *)gtpv2c_rx, &cs_rsp);
//	if (!ret)
//		return ret;
//
//	uint8_t ebi_index =
//		/**create_s5s8_session_response.bearer_context_to_be_created_ebi - 5;*/
//		cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
//	//gtpv2c_rx->teid.has_teid.teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
//
//	/* s11_sgw_gtpc_teid= s5s8_sgw_gtpc_teid =
//	 * key->ue_context_by_fteid_hash */
//	ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
//	    (const void *) &gtpv2c_rx->teid.has_teid.teid,
//	    (void **) &context);
//
//	clLog(s5s8logger, eCLSeverityDebug, "NGIC- create_s5s8_session.c::"
//			"\n\tprocess_sgwc_s5s8_create_session_response"
//			"\n\tprocess_sgwc_s5s8_cs_rsp_cnt= %u;"
//			"\n\tgtpv2c_rx->teid.has_teid.teid= %X\n",
//			process_sgwc_s5s8_cs_rsp_cnt,
//			gtpv2c_rx->teid.has_teid.teid);
//	if (ret < 0 || !context)
//		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
//
//	pdn = context->pdns[ebi_index];
//	{
//		pdn->apn_restriction =/* *IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t,
//		    //create_s5s8_session_response.apn_restriction_ie);*/
//			cs_rsp.apn_restriction.rstrct_type_val;
//		struct in_addr ip;
//		/*ip = get_ipv4_paa_ipv4(
//					create_s5s8_session_response.pdn_addr_alloc_ie);*/
//		ip =*(struct in_addr *) cs_rsp.paa.pdn_addr_and_pfx;
//
//		pdn->ipv4.s_addr = htonl(ip.s_addr);
//
//	//	pdn->s5s8_pgw_gtpc_ipv4.s_addr =
//	//			htonl(create_s5s8_session_response.
//	//			pgw_s5s8_gtpc_fteid->ip_u.ipv4.s_addr);
//		pdn->s5s8_pgw_gtpc_ipv4.s_addr =
//			cs_rsp.pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc.ipv4_address;
//		/* Note: s5s8_pgw_gtpc_teid updated by
//		 * create_s5s8_session_response.pgw_s5s8_gtpc_fteid...
//		 */
//	//	pdn->s5s8_pgw_gtpc_teid =
//	//			htonl(create_s5s8_session_response.
//	//			pgw_s5s8_gtpc_fteid->fteid_ie_hdr.teid_or_gre);
//
//		pdn->s5s8_pgw_gtpc_teid =
//			htonl(cs_rsp.pgw_s5s8_s2as2b_fteid_pmip_based_intfc_or_gtp_based_ctl_plane_intfc.teid_gre_key);
//
//	}
//	bearer = context->eps_bearers[ebi_index];
//	{
//		/* TODO: Implement TFTs on default bearers
//		if (create_s5s8_session_response.bearer_tft_ie) {
//		}
//		*/
//		/* TODO: Implement PGWC S5S8 bearer QoS */
//		if (cs_rsp.bearer_contexts_created.bearer_lvl_qos.header.len) {
//			//bearer->qos = *IE_TYPE_PTR_FROM_GTPV2C_IE(bearer_qos_ie,
//				//create_s5s8_session_response.bearer_qos_ie);
//				bearer->qos.qci = cs_rsp.bearer_contexts_created.bearer_lvl_qos.qci;
//				bearer->qos.ul_mbr = cs_rsp.bearer_contexts_created.bearer_lvl_qos.max_bit_rate_uplnk;
//				bearer->qos.dl_mbr = cs_rsp.bearer_contexts_created.bearer_lvl_qos.max_bit_rate_dnlnk;
//				bearer->qos.ul_gbr = cs_rsp.bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_uplnk;
//				bearer->qos.dl_gbr = cs_rsp.bearer_contexts_created.bearer_lvl_qos.guarntd_bit_rate_dnlnk;
//				bearer->qos.arp.preemption_vulnerability = cs_rsp.bearer_contexts_created.bearer_lvl_qos.pvi;
//				bearer->qos.arp.spare1 = cs_rsp.bearer_contexts_created.bearer_lvl_qos.spare2;
//				bearer->qos.arp.priority_level = cs_rsp.bearer_contexts_created.bearer_lvl_qos.pl;
//				bearer->qos.arp.preemption_capability =cs_rsp.bearer_contexts_created.bearer_lvl_qos.pci;
//				bearer->qos.arp.spare2 = cs_rsp.bearer_contexts_created.bearer_lvl_qos.spare3;
//		}
//	//	bearer->s5s8_pgw_gtpu_ipv4.s_addr =
//	//			htonl(IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
//	//					create_s5s8_session_response.s5s8_pgw_gtpu_fteid)->
//	//						ip_u.ipv4.s_addr);
//	//	bearer->s5s8_pgw_gtpu_teid =
//	//			IE_TYPE_PTR_FROM_GTPV2C_IE(fteid_ie,
//	//					create_s5s8_session_response.s5s8_pgw_gtpu_fteid)->
//	//						fteid_ie_hdr.teid_or_gre;
//
//	//	bearer->s5s8_pgw_gtpu_ipv4.s_addr = cs_rsp.bearer_contexts_created.s5s8_u_pgw_fteid.ipv4_address;
//		bearer->s5s8_pgw_gtpu_ipv4.s_addr = cs_rsp.bearer_contexts_created.s1u_sgw_fteid.ipv4_address;
//	//	bearer->s5s8_pgw_gtpu_teid = cs_rsp.bearer_contexts_created.s5s8_u_pgw_fteid.teid_gre_key;
//		bearer->s5s8_pgw_gtpu_teid = cs_rsp.bearer_contexts_created.s1u_sgw_fteid.teid_gre_key;
//		bearer->pdn = pdn;
//	}
//
//	s11_mme_sockaddr.sin_addr.s_addr =
//					htonl(context->s11_mme_gtpc_ipv4.s_addr);
//
//	clLog(s5s8logger, eCLSeverityDebug, "NGIC- create_s5s8_session.c::"
//			"\n\tprocess_sgwc_s5s8_cs_rsp_cnt= %u;"
//			"\n\tprocess_spgwc_s11_cs_res_cnt= %u;"
//			"\n\tue_ip= pdn->ipv4= %s;"
//			"\n\tpdn->s5s8_sgw_gtpc_ipv4= %s;"
//			"\n\tpdn->s5s8_sgw_gtpc_teid= %X;"
//			"\n\tbearer->s5s8_sgw_gtpu_ipv4= %s;"
//			"\n\tbearer->s5s8_sgw_gtpu_teid= %X;"
//			"\n\tpdn->s5s8_pgw_gtpc_ipv4= %s;"
//			"\n\tpdn->s5s8_pgw_gtpc_teid= %X;"
//			"\n\tbearer->s5s8_pgw_gtpu_ipv4= %s;"
//			"\n\tbearer->s5s8_pgw_gtpu_teid= %X\n",
//			process_sgwc_s5s8_cs_rsp_cnt++,
//			process_spgwc_s11_cs_res_cnt++,
//			inet_ntoa(pdn->ipv4),
//			inet_ntoa(pdn->s5s8_sgw_gtpc_ipv4),
//			pdn->s5s8_sgw_gtpc_teid,
//			inet_ntoa(bearer->s5s8_sgw_gtpu_ipv4),
//			bearer->s5s8_sgw_gtpu_teid,
//			inet_ntoa(pdn->s5s8_pgw_gtpc_ipv4),
//			pdn->s5s8_pgw_gtpc_teid,
//			inet_ntoa(bearer->s5s8_pgw_gtpu_ipv4),
//			bearer->s5s8_pgw_gtpu_teid);
//
//	uint32_t  seq_no = 0;
//	seq_no = bswap_32(gtpv2c_rx->teid.has_teid.seq) ;
//	seq_no = seq_no >> 8;
//
//	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, NULL, context, bearer, pdn);
//	if(pfcp_sess_mod_req.create_pdr_count){
//		pfcp_sess_mod_req.create_pdr[0].pdi.local_fteid.teid = htonl(bearer->s5s8_pgw_gtpu_teid) ;
//		pfcp_sess_mod_req.create_pdr[0].pdi.local_fteid.ipv4_address = htonl(bearer->s5s8_pgw_gtpu_ipv4.s_addr) ;
//		pfcp_sess_mod_req.create_pdr[0].pdi.ue_ip_address.ipv4_address = (pdn->ipv4.s_addr);
//		pfcp_sess_mod_req.create_pdr[0].pdi.src_intfc.interface_value = SOURCE_INTERFACE_VALUE_ACCESS;
//	}else if(pfcp_sess_mod_req.update_far_count){
//		pfcp_sess_mod_req.update_far[0].upd_frwdng_parms.outer_hdr_creation.teid = bearer->s5s8_pgw_gtpu_teid;
//		pfcp_sess_mod_req.update_far[0].upd_frwdng_parms.outer_hdr_creation.ipv4_address  = htonl(bearer->s5s8_pgw_gtpu_ipv4.s_addr) ;
//		pfcp_sess_mod_req.update_far[0].upd_frwdng_parms.dst_intfc.interface_value = SOURCE_INTERFACE_VALUE_CORE;
//	}
//	pfcp_sess_mod_req.header.seid_seqno.has_seid.seq_no = gtpv2c_rx->teid.has_teid.seq ;
//
//	uint8_t pfcp_msg[512]={0};
//	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
//	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
//	header->message_len = htons(encoded - 4);
//
//
//	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr) < 0)
//		clLog(clSystemLog, eCLSeverityCritical, "Error in sending MBR to SGW-U. err_no: %i\n", errno);
//	else
//	{
//		cp_stats.session_modification_req_sent++;
//		get_current_time(cp_stats.session_modification_req_sent_time);
//	}
//	/* Update UE State */
//	context->state = PFCP_SESS_MOD_REQ_SNT_STATE;
//
//	/* Lookup Stored the session information. */
//	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0) {
//		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
//		return -1;
//	}
//
//	/* Set create session response */
//	resp->sequence = seq_no;
//	//resp->eps_bearer_id =
//	//		*create_s5s8_session_response.bearer_context_to_be_created_ebi;
//	resp->eps_bearer_id = cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi;
//	resp->s11_sgw_gtpc_teid = context->s11_sgw_gtpc_teid;
//	resp->context = context;
//	resp->msg_type = GTP_CREATE_SESSION_RSP;
//	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
//
//	return 0;
//}
#endif

int
gen_ccr_request(ue_context_t *context, uint8_t ebi_index, create_sess_req_t *csr)
{
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
	gx_context_t *gx_context = NULL;

	/* VS: Generate unique call id per PDN connection */
	context->pdns[ebi_index]->call_id = generate_call_id();

	/** Allocate the memory for Gx Context
	 */
	gx_context = rte_malloc_socket(NULL,
					sizeof(gx_context_t),
					RTE_CACHE_LINE_SIZE, rte_socket_id());

	/* VS: Generate unique session id for communicate over the Gx interface */
	if (gen_sess_id_for_ccr(gx_context->gx_sess_id,
				context->pdns[ebi_index]->call_id)) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error: %s \n", __func__, __LINE__,
				strerror(errno));
		return -1;
	}

	/* Maintain the gx session id in context */
	memcpy(context->pdns[ebi_index]->gx_sess_id,
			gx_context->gx_sess_id , strlen(gx_context->gx_sess_id));

	/* VS: Maintain the PDN mapping with call id */
	if (add_pdn_conn_entry(context->pdns[ebi_index]->call_id,
				context->pdns[ebi_index]) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to add pdn entry with call id\n", __func__, __LINE__);
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
		ccr_request.data.ccr.subscription_id.list  = rte_malloc_socket(NULL,
				(sizeof(GxSubscriptionId)*1),
				RTE_CACHE_LINE_SIZE, rte_socket_id());

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

	/* TODO: Removing this ie as it is not require.
	 * It's showing padding in pcap
	ccr_request.data.ccr.presence.framed_ip_address = PRESENT;

	ccr_request.data.ccr.framed_ip_address.len = inet_ntoa(ccr_request.data.ccr.framed_ip_address.val,
			                                               context->pdns[ebi_index]);

	char *temp = inet_ntoa(context->pdns[ebi_index]->ipv4);
	memcpy(ccr_request.data.ccr.framed_ip_address.val, &temp, strlen(temp)); */

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
	if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, gx_context->gx_sess_id) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
		return -1;
	}

	struct sockaddr_in saddr_in;
    	saddr_in.sin_family = AF_INET;
    	inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    	update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_INITIAL, SENT, GX);


	/* Update UE State */
	context->pdns[ebi_index]->state = CCR_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCR_SNT_STATE;
	gx_context->proc = context->pdns[ebi_index]->proc;

	/* VS: Maintain the Gx context mapping with Gx Session id */
	if (gx_context_entry_add((uint8_t*)gx_context->gx_sess_id, gx_context) < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error: %s \n", __func__, __LINE__,
				strerror(errno));
		return -1;
	}

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_ccr_calc_length(&ccr_request.data.ccr);
	buffer = rte_zmalloc_socket(NULL, msg_len + sizeof(ccr_request.msg_type),
	    RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (buffer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return -1;
	}

	/* VS: Fill the CCR header values */
	memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

	if (gx_ccr_pack(&(ccr_request.data.ccr),
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type));
	return 0;
}

#ifdef FUTURE_NEED
int
gx_setup_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

	ret = process_create_sess_req(&msg->gtpc_msg.csr, &context, &pdn, msg);
	if (ret != 0 && ret != -2) {
		if (ret != -1){

			cs_error_response(msg, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			process_error_occured_handler_new(data, unused_param);
		}
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Error: %d \n",
				__file__, __func__, __LINE__, ret);
		return -1;
	}

	RTE_SET_USED(unused_param);
	return ret;
}
#endif
