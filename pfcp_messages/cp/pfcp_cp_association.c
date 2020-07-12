// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_enum.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "clogger.h"
#include "cp_main.h"
#include "pfcp.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "cp_config.h"
#include "gtpv2c_error_rsp.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "pfcp_timer.h"
#include "pfcp_transactions.h"
#include "cp_peer.h"
#include "gen_utils.h"
#include "sm_structs_api.h"
#include "gw_adapter.h"


#if defined(USE_DNS_QUERY)
#include "sm_pcnd.h"
#include "cdnsutil.h"
#endif /* USE_DNS_QUERY */

extern udp_sock_t my_sock;

void
fill_pfcp_association_release_req(pfcp_assn_rel_req_t *pfcp_ass_rel_req)
{
	uint32_t seq  = 1;
	memset(pfcp_ass_rel_req, 0, sizeof(pfcp_assn_rel_req_t)) ;

	/*filing of pfcp header*/
	seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_RELEASE_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_rel_req->header),
			PFCP_ASSOCIATION_RELEASE_REQUEST, NO_SEID, seq);
	/*filling of node id*/
	char pAddr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_ass_rel_req->node_id), node_value);
}

void
fill_pfcp_association_update_req(pfcp_assn_upd_req_t *pfcp_ass_update_req)
{
	uint32_t seq  = 1;

	memset(pfcp_ass_update_req, 0, sizeof(pfcp_assn_upd_req_t)) ;

	seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_UPDATE_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_update_req->header),
			 PFCP_ASSOCIATION_UPDATE_REQUEST, NO_SEID, seq);

	char peer_addr[INET_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), peer_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(peer_addr);
	set_node_id(&(pfcp_ass_update_req->node_id), node_value);

	set_upf_features(&(pfcp_ass_update_req->up_func_feat));

	set_cpf_features(&(pfcp_ass_update_req->cp_func_feat));

	set_pfcp_ass_rel_req(&(pfcp_ass_update_req->up_assn_rel_req));

	set_graceful_release_period(&(pfcp_ass_update_req->graceful_rel_period));

}

void
fill_pfcp_association_setup_req(pfcp_assn_setup_req_t *pfcp_ass_setup_req)
{

	uint32_t seq  = 1;
	char node_addr[INET_ADDRSTRLEN] = {0};

	memset(pfcp_ass_setup_req, 0, sizeof(pfcp_assn_setup_req_t)) ;

	seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_SETUP_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_setup_req->header),
			PFCP_ASSOCIATION_SETUP_REQUEST, NO_SEID, seq);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), node_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(node_addr);
	set_node_id(&(pfcp_ass_setup_req->node_id), node_value);

	set_recovery_time_stamp(&(pfcp_ass_setup_req->rcvry_time_stmp));

	/* As we are not supporting this feature
	set_cpf_features(&(pfcp_ass_setup_req->cp_func_feat)); */
}

/* Fill pfd mgmt cstm ie */
uint16_t
set_pfd_contents(pfcp_pfd_contents_ie_t *pfd_conts, struct msgbuf *cstm_buf)
{
	pfd_conts->pfd_contents_spare = 0;
	pfd_conts->pfd_contents_cp = 1;
	/*pfd_conts->dn = 0;
	pfd_conts->url = 0;
	pfd_conts->fd = 0;
	pfd_conts->pfd_contents_spare2 = 0x00;*/

	if(pfd_conts->fd != 0){
		pfd_conts->len_of_flow_desc = 0;
		pfd_conts->flow_desc = 0;
	}

	if(pfd_conts->url != 0){
		pfd_conts->length_of_url = 0;
		pfd_conts->url2 = 0;
	}

	if(pfd_conts->dn != 0){
		pfd_conts->len_of_domain_nm = 0;
		pfd_conts->domain_name = 0;
	}

	if(pfd_conts->pfd_contents_cp != 0){
		uint16_t struct_len = 0;
		switch (cstm_buf->mtype) {
			case MSG_SDF_CRE:
			case MSG_ADC_TBL_CRE:
			case MSG_PCC_TBL_CRE:
			case MSG_SESS_TBL_CRE:
			case MSG_MTR_CRE:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct cb_args_table));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt, "%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt+struct_len, (uint8_t *)&cstm_buf->msg_union.msg_table,
														sizeof(cstm_buf->msg_union.msg_table));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(cstm_buf->msg_union.msg_table) + struct_len;
				break;

			case MSG_EXP_CDR:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct msg_ue_cdr));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
												"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len,
					  (uint8_t *)&cstm_buf->msg_union.ue_cdr, sizeof(struct msg_ue_cdr));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct msg_ue_cdr) + struct_len;
				break;
			case MSG_SDF_DES:
			case MSG_ADC_TBL_DES:
			case MSG_PCC_TBL_DES:
			case MSG_SESS_TBL_DES:
			case MSG_MTR_DES:
				break;
			case MSG_SDF_ADD:
			case MSG_SDF_DEL:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct pkt_filter));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
													"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len,
					  (uint8_t *)&cstm_buf->msg_union.pkt_filter_entry, sizeof(struct pkt_filter));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct pkt_filter)+struct_len;
				break;
			case MSG_ADC_TBL_ADD:
			case MSG_ADC_TBL_DEL:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct adc_rules));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
												"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len,
					  (uint8_t *)&cstm_buf->msg_union.adc_filter_entry, sizeof(struct adc_rules));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct adc_rules)+ struct_len;
				break;
			case MSG_PCC_TBL_ADD:
			case MSG_PCC_TBL_DEL:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct pcc_rules));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
												"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len,
					  (uint8_t *)&cstm_buf->msg_union.pcc_entry, sizeof(struct pcc_rules));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct pcc_rules) + struct_len;
				break;
			case MSG_SESS_CRE:
			case MSG_SESS_MOD:
			case MSG_SESS_DEL:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct session_info));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
												"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len,
					  (uint8_t *)&cstm_buf->msg_union.sess_entry, sizeof(struct session_info));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct session_info) + struct_len;
				break;
			case MSG_MTR_ADD:
			case MSG_MTR_DEL:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct mtr_entry));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
													"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len ,
					  (uint8_t *)&cstm_buf->msg_union.mtr_entry, sizeof(struct mtr_entry));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct mtr_entry) + struct_len;
				break;
			case MSG_DDN_ACK:
				pfd_conts->cstm_pfd_cntnt = malloc(sizeof(struct downlink_data_notification));
				/* Fill msg type */
				struct_len = sprintf((char *)pfd_conts->cstm_pfd_cntnt,
																"%"PRId64" ",cstm_buf->mtype);
				/* Fill cstm ie contents frome rule structure as string */
				memcpy(pfd_conts->cstm_pfd_cntnt + struct_len ,
			          (uint8_t *)&cstm_buf->msg_union.mtr_entry, sizeof(struct downlink_data_notification));
				pfd_conts->len_of_cstm_pfd_cntnt = sizeof(struct downlink_data_notification) + struct_len;
				break;
			default:
				clLog(apilogger, eCLSeverityCritical, "build_dp_msg: Invalid msg type\n");
				break;
		}
	}
	/* set pfd contents header */
	pfcp_set_ie_header(&pfd_conts->header, PFCP_IE_PFD_CONTENTS,
			(pfd_conts->len_of_cstm_pfd_cntnt + 3));
	return (pfd_conts->len_of_cstm_pfd_cntnt + 3);
}

/**
 * @brief  : This function fills in values to pfd context ie
 * @param  : pfd_contxt is pointer to structure of pfd context ie
 * @return : This function dose not return anything
 */
static void
set_pfd_context(pfcp_pfd_context_ie_t *pfd_conxt)
{

	pfcp_set_ie_header(&pfd_conxt->header, PFCP_IE_PFD_CONTEXT,
			(pfd_conxt->pfd_contents[0].header.len + sizeof(pfcp_ie_header_t)));
	pfd_conxt->pfd_contents_count = 1;

}

/**
 * @brief  : This function fills in values to pfd application id ie
 * @param  : app_id is pointer to structure of pfd application id ie
 * @return : This function dose not return anything
 */
static void
set_pfd_application_id(pfcp_application_id_ie_t *app_id)
{
	//REVIEW: Remove this hardcoded value
	pfcp_set_ie_header(&app_id->header, PFCP_IE_APPLICATION_ID, 8);
	memcpy(app_id->app_ident, "_app_1  ", 8);

}

/**
 * @brief  : This function fills pfd app id and pfd context
 * @param  : app_id_pfds_t is pointer to structure of  ie
 * @param  : len denotes total length of ie
 * @return : This function dose not return anything
 */
static void
set_app_ids_pfds(pfcp_app_ids_pfds_ie_t *app_ids_pfds_t , uint16_t len)
{
	/* Fill app id */
	set_pfd_application_id(&app_ids_pfds_t->application_id);
	app_ids_pfds_t->pfd_context_count = 1;

	/* Fill pfd context */
	for(int i = 0; i < app_ids_pfds_t->pfd_context_count; ++i){
		set_pfd_context(&app_ids_pfds_t->pfd_context[i]);
		len = app_ids_pfds_t->pfd_context[i].header.len
			+ app_ids_pfds_t->application_id.header.len
			+ sizeof(pfcp_ie_header_t)
			+ sizeof(pfcp_ie_header_t);
	}
	/* set app id pfds header  */
	pfcp_set_ie_header(&app_ids_pfds_t->header, PFCP_IE_APP_IDS_PFDS, len);
}


void
fill_pfcp_pfd_mgmt_req(pfcp_pfd_mgmt_req_t *pfcp_pfd_req, uint16_t len)
{

	uint32_t seq  = 0;
	seq = get_pfcp_sequence_number(PFCP_PFD_MGMT_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_pfd_req->header),
			PFCP_PFD_MGMT_REQUEST, NO_SEID, seq);
	pfcp_pfd_req->app_ids_pfds_count = 1;

	for(int i=0; i < pfcp_pfd_req->app_ids_pfds_count; ++i){
		set_app_ids_pfds(&pfcp_pfd_req->app_ids_pfds[i], len);
	}
}


int
buffer_csr_request(ue_context_t *context,
		upf_context_t *upf_context, uint8_t ebi)
{
	pending_csreq_key_t *key =
					rte_zmalloc_socket(NULL, sizeof(pending_csreq_key_t),
						RTE_CACHE_LINE_SIZE, rte_socket_id());

	key->teid = context->s11_sgw_gtpc_teid;
	key->sender_teid = context->s11_mme_gtpc_teid;
	key->sequence = context->sequence;
	key->ebi_index = ebi;

    LIST_INSERT_HEAD(&upf_context->pendingCSRs, key, csrentries);
	return 0;
}

#ifdef USE_DNS_QUERY
int
get_upf_ip(ue_context_t *ctxt, upfs_dnsres_t **_entry,
		uint32_t **upf_ip)
{
	upfs_dnsres_t *entry = NULL;

	if (upflist_by_ue_hash_entry_lookup(&ctxt->imsi,
			sizeof(ctxt->imsi), &entry) != 0)
		return -1;

	if (entry->current_upf > entry->upf_count) {
		/* TODO: Add error log : Tried sending
		 * association request to all upf.*/
		/* Remove entry from hash ?? */
		return -1;
	}

	*upf_ip = &(entry->upf_ip[entry->current_upf].s_addr);
	*_entry = entry;
	return 0;
}
#endif /* USE_DNS_QUERY */

upf_context_t *get_upf_context(uint32_t upf_ip)
{
    int ret = 0;
    upf_context_t *upf_context = NULL;

	ret = rte_hash_lookup_data(upf_context_by_ip_hash,
			(const void*) &(upf_ip), (void **) &(upf_context));

	if (ret >= 0) {
        printf("Found upf context \n");
        return upf_context;
    } else {
        printf("Found upf context \n");
    }
    return NULL;
}

int create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt) 
{
    int ret;
    upf_context_t *upf_context = NULL;
	upf_context  = rte_zmalloc_socket(NULL, sizeof(upf_context_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());

	if (upf_context == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate upf context: "
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);

		return -1;
	}
    *upf_ctxt = upf_context;
	bzero(upf_context->upf_sockaddr.sin_zero, sizeof(upf_context->upf_sockaddr.sin_zero));
	upf_context->upf_sockaddr.sin_family = AF_INET;
	upf_context->upf_sockaddr.sin_port = htons(cp_config->upf_pfcp_port);
	upf_context->upf_sockaddr.sin_addr.s_addr = upf_ip; 

	ret = upf_context_entry_add(&upf_ip, upf_context);
	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return -1;
	}

	upf_context->assoc_status = ASSOC_NOT_INITIATED;
    LIST_INIT(&upf_context->pendingCSRs);
    return 0;

} 
/**
 * @brief  : This function creates association setup request and sends to peer
 * @param  : context holds information of ue
 * @param  : ebi_index denotes index of bearer stored in array
 * @return : This function dose not return anything
 */
static int
assoication_setup_request(uint32_t upf_ip, upf_context_t **upf_ctxt)
{
	upf_context_t *upf_context = NULL;
	//char sgwu_fqdn_res[MAX_HOSTNAME_LENGTH] = {0};
	pfcp_assn_setup_req_t pfcp_ass_setup_req = {0};
	struct in_addr test; test.s_addr = upf_ip;
	printf("Initiate PFCP setup to peer address = %s \n", inet_ntoa(test));

    upf_context = get_upf_context(upf_ip);

    if(upf_context == NULL && create_upf_context(upf_ip, &upf_context) < 0 )
    {
        return -1;
    }
    *upf_ctxt = upf_context;
	upf_context->assoc_status = ASSOC_IN_PROGRESS;
	upf_context->state = PFCP_ASSOC_REQ_SNT_STATE;

	fill_pfcp_association_setup_req(&pfcp_ass_setup_req);

	uint8_t pfcp_msg[256] = {0};
	int encoded = encode_pfcp_assn_setup_req_t(&pfcp_ass_setup_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

    /* Let's have separate transaction and peer */
	/* fill and add timer entry */
#ifdef DELETE_THIS
	peerData_t *timer_entry = NULL;
	timer_entry =  pfcp_fill_timer_entry_data(PFCP_IFACE, &context->upf_ctxt->upf_sockaddr,
			pfcp_msg, encoded, cp_config->request_tries, context->s11_sgw_gtpc_teid, ebi_index);

	if(!(pfcp_add_timer_entry(timer_entry, cp_config->request_timeout, pfcp_peer_timer_callback))) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Faild to add timer entry...\n",
				__FILE__, __func__, __LINE__);
	}

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_ctxt->upf_sockaddr) < 0 ) {
		clLog(clSystemLog, eCLSeverityDebug,"Error sending\n\n");
	} else {

		update_cli_stats(upf_ip, pfcp_ass_setup_req.header.message_type, SENT, SX);

		upf_context->timer_entry = timer_entry;
		if (starttimer(&timer_entry->pt) < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Periodic Timer failed to start...\n",
					__FILE__, __func__, __LINE__);
		}
	}
#else
   	transData_t *trans_entry = NULL;
	trans_entry =  create_transaction(upf_context, pfcp_msg, encoded); 

	if(!(start_transaction_timer(trans_entry, cp_config->request_timeout, transaction_timeout_callback))) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Faild to add timer entry...\n",
				__FILE__, __func__, __LINE__);
	}

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &upf_context->upf_sockaddr) < 0 ) {
		clLog(clSystemLog, eCLSeverityDebug,"Error sending\n\n");
	} else {

		update_cli_stats(upf_ip, pfcp_ass_setup_req.header.message_type, SENT, SX);

		if (starttimer(&trans_entry->rt) < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Periodic Timer failed to start...\n",
					__FILE__, __func__, __LINE__);
		}
	}
    upf_context->timer_entry = trans_entry;
#endif
    printf("UPF context allocated \n");
	return 0;
}

int
process_pfcp_assoication_request(pdn_connection_t *pdn, uint8_t ebi_index) 
{
	int ret = 0;
	struct in_addr upf_ipv4 = {0};
	upf_context_t *upf_context = NULL;

    //if upf address is not yet selected then go for DNS query or Static  
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
		/* Need to think on it*/
		upf_ipv4.s_addr = *upf_ip;
#else
		// if name is nit already resolved and no DNS enabled then use the configured
		// upf address 
		pdn->upf_ipv4.s_addr = cp_config->upf_pfcp_ip.s_addr;
		upf_ipv4.s_addr =      cp_config->upf_pfcp_ip.s_addr;
#endif /* USE_DNS_QUERY */
	}

	/* Requirement :use upf_context reference from user_context */ 
	/* VS: Retrive association state based on UPF IP. */
	ret = rte_hash_lookup_data(upf_context_by_ip_hash,
			(const void*) &(upf_ipv4.s_addr), (void **) &(upf_context));
	if (ret >= 0 && upf_context->assoc_status != ASSOC_NOT_INITIATED) {
        printf("UPF found \n");
        pdn->context->upf_ctxt = upf_context;
		if (upf_context->state == PFCP_ASSOC_RESP_RCVD_STATE) {
            printf("UPF association already formed \n");
			ret = process_pfcp_sess_est_request(pdn->context->s11_sgw_gtpc_teid, pdn, upf_context);
			if (ret) {
					clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
							__func__, __LINE__, ret);
					return ret;
			}
		} else {
            printf("UPF association formation in progress \n");
			ret = buffer_csr_request(pdn->context, upf_context, ebi_index);
			if (ret) {
				clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
						__func__, __LINE__, ret);
				return -1;
			}
		}
	} else {
		printf("[%s] - %d -  Initiate PFCP association setup line  %s \n",__FUNCTION__,__LINE__, inet_ntoa(pdn->ipv4));
        uint32_t upf_ip = pdn->upf_ipv4.s_addr;
		ret = assoication_setup_request(upf_ip, &upf_context);
		if (ret) {
				clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
						__func__, __LINE__, ret);
				return ret;
		}
        /* We should keep track of UPF used for the UE 
         * sometime this may be need to be PDN based but 
         * currently we are far away from multiple PDN & 
         */
        pdn->context->upf_ctxt = upf_context;
	    ret = buffer_csr_request(pdn->context, pdn->context->upf_ctxt, ebi_index);
	    if (ret) {
	    	clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
	    			__func__, __LINE__, ret);
	    	return -1;
	    }
	}
	return 0;
}


void
fill_pfcp_node_report_req(pfcp_node_rpt_req_t *pfcp_node_rep_req)
{
	uint32_t seq  = 1;
	char node_addr[INET_ADDRSTRLEN] = {0} ;
	memset(pfcp_node_rep_req, 0, sizeof(pfcp_node_rpt_req_t)) ;

	seq = get_pfcp_sequence_number(PFCP_NODE_REPORT_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_node_rep_req->header),
			PFCP_NODE_REPORT_REQUEST, NO_SEID, seq);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), node_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(node_addr);
	set_node_id(&(pfcp_node_rep_req->node_id), node_value);

	set_node_report_type(&(pfcp_node_rep_req->node_rpt_type));

	set_user_plane_path_failure_report(&(pfcp_node_rep_req->user_plane_path_fail_rpt));
}

void
fill_pfcp_sess_report_resp(pfcp_sess_rpt_rsp_t *pfcp_sess_rep_resp,
		 uint32_t seq)
{
	memset(pfcp_sess_rep_resp, 0, sizeof(pfcp_sess_rpt_rsp_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_rep_resp->header),
		PFCP_SESSION_REPORT_RESPONSE, HAS_SEID, seq);

	set_cause(&(pfcp_sess_rep_resp->cause), REQUESTACCEPTED);

	//pfcp_sess_rep_resp->header.message_len = pfcp_sess_rep_resp->cause.header.len + 4;

	//pfcp_sess_rep_resp->header.message_len += sizeof(pfcp_sess_rep_resp->header.seid_seqno.has_seid);
}

uint8_t
process_pfcp_ass_resp(msg_info *msg, struct sockaddr_in *peer_addr)
{
    int ret = 0;
    pdn_connection_t *pdn = NULL;
    upf_context_t *upf_context = msg->upf_context;
    assert(upf_context != NULL); /* if NULL we should have already dropped msg */
    upf_context->assoc_status = ASSOC_ESTABLISHED;
    upf_context->state = PFCP_ASSOC_RESP_RCVD_STATE;

    upf_context->up_supp_features =
        msg->pfcp_msg.pfcp_ass_resp.up_func_feat.sup_feat;

    switch (cp_config->cp_type)
    {
        case SGWC :
            if (msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;

            if( msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].src_intfc ==
                    SOURCE_INTERFACE_VALUE_CORE )
                upf_context->s5s8_sgwu_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[1].ipv4_address;
            break;

        case PGWC :
            if (msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s5s8_pgwu_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

        case SAEGWC :
            if( msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

    }

    /* teid_range from first user plane ip IE is used since, for same CP ,
     * DP will assigne single teid_range , So all IE's will have same value for teid_range*/
    /* Change teid base address here */
    if(msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teidri != 0){
        /* Requirement : This data should go in the upf context */
        set_base_teid(msg->pfcp_msg.pfcp_ass_resp.user_plane_ip_rsrc_info[0].teid_range);
    }

    pending_csreq_key_t *key;
    key = LIST_FIRST(&upf_context->pendingCSRs);
    while (key != NULL) {
        LIST_REMOVE(key, csrentries);
        if (get_pdn(key->teid, &pdn) < 0){
            clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to get pdn for teid: %u\n",
                    __func__, __LINE__, key->teid);
        }

        ret = process_pfcp_sess_est_request(key->teid, pdn, upf_context);
        if (ret) {
            clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
            if(ret != -1) {
                cs_error_response(msg, ret,
                        cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
                process_error_occured_handler(&msg, NULL);
            }
        }
        rte_free(key);
        key = LIST_FIRST(&upf_context->pendingCSRs);
    }

    /* Adding ip to cp  heartbeat when dp returns the association response*/
    add_ip_to_heartbeat_hash(peer_addr,
            msg->pfcp_msg.pfcp_ass_resp.rcvry_time_stmp.rcvry_time_stmp_val);

    if ((add_node_conn_entry((uint32_t)peer_addr->sin_addr.s_addr,
                    SX_PORT_ID)) != 0) {

        clLog(clSystemLog, eCLSeverityCritical, "Failed to add connection entry for SGWU/SAEGWU");
    }
    return 0;

}

uint8_t
process_pfcp_report_req(pfcp_sess_rpt_req_t *pfcp_sess_rep_req)
{

	/*DDN Handling */
	uint8_t ebi_index;
	int ret = 0, encoded = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint8_t pfcp_msg[250] = {0};
	struct resp_info *resp = NULL;
	pfcp_sess_rpt_rsp_t pfcp_sess_rep_resp = {0};
	uint64_t sess_id = pfcp_sess_rep_req->header.seid_seqno.has_seid.seid;

	uint32_t sequence = 0;
	uint32_t s11_sgw_gtpc_teid = UE_SESS_ID(sess_id);

	/* Stored the session information*/
	if (get_sess_entry(sess_id, &resp) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	ebi_index =  UE_BEAR_ID(sess_id) - 5;
	/* Retrive the s11 sgwc gtpc teid based on session id.*/
	sequence = pfcp_sess_rep_req->header.seid_seqno.has_seid.seq_no;
	resp->msg_type = PFCP_SESSION_REPORT_REQUEST;

	clLog(sxlogger, eCLSeverityDebug, "DDN Request recv from DP for sess:%lu\n", sess_id);

	if (pfcp_sess_rep_req->report_type.dldr == 1) {
		ret = ddn_by_session_id(sess_id);
		if (ret) {
			clLog(clSystemLog, eCLSeverityCritical, "DDN %s: (%d) \n", __func__, ret);
			return -1;
		}
		/* Update the Session state */
		resp->state = DDN_REQ_SNT_STATE;
	}

	/* Update the UE State */
	ret = update_ue_state(s11_sgw_gtpc_teid,
			DDN_REQ_SNT_STATE, ebi_index);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State for teid: %u\n", __func__,
				s11_sgw_gtpc_teid);
	}

	/* Retrieve the UE context */
	ret = get_ue_context(s11_sgw_gtpc_teid, &context);
	if (ret < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					s11_sgw_gtpc_teid);
	}
	pdn = GET_PDN(context, ebi_index);
	pdn->state = DDN_REQ_SNT_STATE;

	/*Fill and send pfcp session report response. */
	fill_pfcp_sess_report_resp(&pfcp_sess_rep_resp,
			sequence);

	pfcp_sess_rep_resp.header.seid_seqno.has_seid.seid = pdn->dp_seid;

	encoded =  encode_pfcp_sess_rpt_rsp_t(&pfcp_sess_rep_resp, pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	if (pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_ctxt->upf_sockaddr) < 0 ) {
		clLog(sxlogger, eCLSeverityCritical, "Error REPORT REPONSE message: %i\n", errno);
		return -1;
	}
	else {
		update_cli_stats((uint32_t)context->upf_ctxt->upf_sockaddr.sin_addr.s_addr,
				pfcp_sess_rep_resp.header.message_type,ACC,SX);
	}

	return 0;
}

void
fill_pfcp_heartbeat_req(pfcp_hrtbeat_req_t *pfcp_heartbeat_req, uint32_t seq)
{

	memset(pfcp_heartbeat_req, 0, sizeof(pfcp_hrtbeat_req_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_heartbeat_req->header),
			PFCP_HEARTBEAT_REQUEST,	NO_SEID, seq);

	set_recovery_time_stamp(&(pfcp_heartbeat_req->rcvry_time_stmp));
	seq++;
}
void
fill_pfcp_heartbeat_resp(pfcp_hrtbeat_rsp_t *pfcp_heartbeat_resp)
{

	uint32_t seq  = 1;
	memset(pfcp_heartbeat_resp, 0, sizeof(pfcp_hrtbeat_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_heartbeat_resp->header),
			PFCP_HEARTBEAT_RESPONSE, NO_SEID, seq);

	set_recovery_time_stamp(&(pfcp_heartbeat_resp->rcvry_time_stmp));
}

int process_pfcp_heartbeat_req(struct sockaddr_in *peer_addr, uint32_t seq)
{
	uint8_t pfcp_msg[250]={0};
	int encoded = 0;

	pfcp_hrtbeat_req_t pfcp_heartbeat_req  = {0};
	pfcp_hrtbeat_rsp_t *pfcp_hearbeat_resp =
						malloc(sizeof(pfcp_hrtbeat_rsp_t));

	memset(pfcp_hearbeat_resp,0,sizeof(pfcp_hrtbeat_rsp_t));
	fill_pfcp_heartbeat_req(&pfcp_heartbeat_req, seq);

	encoded = encode_pfcp_hrtbeat_req_t(&pfcp_heartbeat_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, peer_addr) < 0 ) {
				clLog(sxlogger, eCLSeverityDebug, "Error sending: %i\n", errno);
	}

	return 0;

}
