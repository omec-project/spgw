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
#include "initial_attach_proc.h"
#include "pfcp_association_setup_proc.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern struct sockaddr_in s11_mme_sockaddr;
extern socklen_t s11_mme_sockaddr_len;

extern int s11logger;
extern int s5s8logger;
extern struct sockaddr_in s11_mme_sockaddr;
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
		if(ret != -1) {
            // send cs response 
			cs_error_response(msg, ret,
					cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		}
        // cleanup call locally 
		process_error_occured_handler_new((void *)msg, NULL);
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Error: %d \n",
				__file__, __func__, __LINE__, ret);
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
            ret = GTPV2C_CAUSE_INVALID_PEER;
			cs_error_response(msg, ret,
						      cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
	        process_error_occured_handler_new((void *)msg, NULL);
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
    dpkey.plmn.is_valid = true;
    dpkey.plmn.tac = csr->uli.tai2.tai_tac;
    memcpy((void *)(&dpkey.plmn.plmn[0]), (void *)(&csr->uli.tai2), 3);
    printf("csr uli mcc %d %d %d  mnc %d %d %d \n", csr->uli.tai2.tai_mcc_digit_1, csr->uli.tai2.tai_mcc_digit_2, csr->uli.tai2.tai_mcc_digit_3, csr->uli.tai2.tai_mnc_digit_1, csr->uli.tai2.tai_mnc_digit_2, csr->uli.tai2.tai_mnc_digit_3);
    
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
                RTE_LOG_DP(DEBUG, CP, "Received CSReq with static address %s"
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
        printf("%s %d - PCO length = %d \n", __FUNCTION__, __LINE__, csr->pco.header.len);
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

	clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
			"Msg_Type:PFCP_SESSION_ESTABLISHMENT_RESPONSE[%u], Seid:%lu, "
			"Procedure:%s, State:%s, Event:%s\n",
			__func__, msg->msg_type,
			msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
			get_proc_string(msg->proc),
			get_state_string(msg->state), get_event_string(msg->event));


	if(msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value != REQUESTACCEPTED) {
		msg->state = ERROR_OCCURED_STATE;
		msg->event = ERROR_OCCURED_EVNT;
		msg->proc = INITIAL_PDN_ATTACH_PROC;
		cs_error_response(msg,
						  GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER,
						  cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		process_error_occured_handler_new(&msg, NULL);
		clLog(sxlogger, eCLSeverityDebug, "Cause received Est response is %d\n",
				msg->pfcp_msg.pfcp_sess_est_resp.cause.cause_value);
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
		if(ret != -1){
			cs_error_response(msg, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			process_error_occured_handler_new(data, unused_param);
		}
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
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
		/* Send response on s11 interface */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,
				s11_mme_sockaddr_len);

		update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, ACC,S11);
        
        ue_context_t *ue_context = msg->ue_context; 
        proc_context_t *proc_context = ue_context->current_proc;
        transData_t *trans_rec = proc_context->gtpc_trans;
        uint16_t port_num = s11_mme_sockaddr.sin_port; 
        uint32_t sender_addr = s11_mme_sockaddr.sin_addr.s_addr; 
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
        free(proc_context);
        ue_context->current_proc = NULL;
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

	  s11_mme_sockaddr.sin_addr.s_addr =
	  				htonl(csr->sender_fteid_ctl_plane.ipv4_address);
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



