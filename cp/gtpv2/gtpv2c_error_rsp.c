/*
 * Copyright (c) 2019 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// AJAY-MUST clean

#include "gtp_ies.h"
#include "gtpv2c_error_rsp.h"
#include "sm_arr.h"
#include "cp_config.h"
#include "cp_stats.h"
#include "ipc_api.h"
#include "cp_timer.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages_encoder.h"
#include "gtp_messages_encoder.h"
#include "gtpv2c_set_ie.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"

struct sockaddr_in upf_pfcp_sockaddr;
extern socklen_t s11_mme_sockaddr_len;
extern socklen_t s5s8_sockaddr_len;
extern uint16_t payload_length;
extern int s5s8_fd;
extern int pfcp_fd;
extern struct sockaddr_in s5s8_recv_sockaddr;

int8_t
clean_up_while_error(uint8_t ebi, uint32_t teid, uint64_t *imsi_val, uint16_t imsi_len,uint32_t seq)
{
	uint64_t imsi = UINT64_MAX;
	ue_context_t *context = NULL;
	upf_context_t *upf_context = NULL;
	pdn_connection_t *pdn = NULL;
	context_key *key = NULL;
	uint8_t ebi_index = ebi - 5;
	struct resp_info *resp;
	int ret = 0;
	if(teid != 0) {
		if (get_ue_context_while_error(teid, &context) == 0) {
			//pdn = GET_PDN(context, ebi_index);
			if(context != NULL && context->eps_bearers[ebi_index] != NULL
				&& context->eps_bearers[ebi_index]->pdn != NULL) {
				pdn = context->eps_bearers[ebi_index]->pdn;
				if (pdn){
					if (get_sess_entry(pdn->seid, &resp) == 0) {
						if (cp_config->cp_type == SGWC){
							if(resp->state == PFCP_SESS_DEL_REQ_SNT_STATE) {
								goto del_ue_cntx_imsi;
							}
							pfcp_sess_del_req_t pfcp_sess_del_req = {0};
							fill_pfcp_sess_del_req(&pfcp_sess_del_req);

							pfcp_sess_del_req.header.seid_seqno.has_seid.seid = pdn->dp_seid;
							uint8_t pfcp_msg[512]={0};
							int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
							pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
							header->message_len = htons(encoded - 4);

							if(pfcp_send(pfcp_fd, pfcp_msg,encoded, &upf_pfcp_sockaddr) < 0) {
								fprintf(stderr , " %s:%s:%u Error sending: %i\n",
										__FILE__, __func__, __LINE__, errno);
							}else {
#ifdef CP_BUILD
							add_pfcp_if_timer_entry(context->s11_sgw_gtpc_teid,
								&upf_pfcp_sockaddr, pfcp_msg, encoded, ebi_index);
#endif /* CP_BUILD */
							}
						} else {
							if(resp->state == PFCP_SESS_DEL_REQ_SNT_STATE) {
								goto del_ue_cntx_imsi;
							}
						}
						resp->state = ERROR_OCCURED_STATE;
						resp->msg_type = GTP_CREATE_SESSION_RSP;
						resp->eps_bearer_id = ebi_index;
					}
					 else {
						if(*imsi_val > 0 && imsi_len > 0) {
							memcpy(&imsi, imsi_val, imsi_len);
							ret = rte_hash_lookup_data(ue_context_by_imsi_hash,&imsi,
										(void **) &(*context));

							if (ret == -ENOENT){
								return -1;
							}else {
								rte_hash_del_key(ue_context_by_imsi_hash,(const void *) &imsi);
								rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid);
								rte_free(context);
								context = NULL;
							}
						}
						else {
							if(pdn != NULL) {
								if ((upf_context_entry_lookup(pdn->upf_ipv4.s_addr,&upf_context)) ==  0) {
									if(upf_context->state < PFCP_ASSOC_RESP_RCVD_STATE){
										if(ret >= 0) {
                                            LIST_FOREACH(key, &upf_context->pendingCSRs, csrentries) {
												if(key != NULL && key->teid == context->s11_sgw_gtpc_teid ) {
                                                    LIST_REMOVE(key, csrentries);
                                                    rte_free(key);
                                                    break;
                                                }
                                            }
                                            if(LIST_EMPTY(&upf_context->pendingCSRs)) {
									        		ret = rte_hash_del_key(upf_context_by_ip_hash,
												(const void *) &pdn->upf_ipv4.s_addr);
												rte_free(upf_context);
												upf_context  = NULL;
                                            }
										}
									}
								}
							}
							ret = rte_hash_del_key(ue_context_by_imsi_hash,
											(const void *) &context->imsi);
							if(ret < 0)
								return -1;
							ret = rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid);
							if (context != NULL)
								rte_free(context);
							context = NULL;
						}
					}

					pdn->state = ERROR_OCCURED_STATE;
				}
			}
		}else {
			return -1;
		}
	}
	 else {
		del_ue_cntx_imsi:
		memcpy(&imsi, imsi_val, imsi_len);
		ret = rte_hash_lookup_data(ue_context_by_imsi_hash,&imsi,(void **) &(*context));

		if (ret == -ENOENT)
			return -1;

		rte_hash_del_key(ue_context_by_imsi_hash,(const void *) &imsi);
		rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid);
		rte_free(context);
		context = NULL;
	}
	return 0;
	RTE_SET_USED(seq);
}

void get_error_rsp_info(msg_info *msg, err_rsp_info *rsp_info) 
{

	int ret = 0;
	ue_context_t *context = NULL;
#ifdef GX_BUILD
	pdn_connection_t *pdn = NULL;
#endif
	struct resp_info *resp = NULL;

	switch(msg->msg_type) {

		case GTP_CREATE_SESSION_REQ:{

			rsp_info->sender_teid = msg->gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key;
			rsp_info->seq = msg->gtpc_msg.csr.header.teid.has_teid.seq;
			rsp_info->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid =  msg->gtpc_msg.csr.header.teid.has_teid.teid;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.header.len)
				rsp_info->offending = GTP_IE_CREATE_SESS_REQUEST_BEARER_CTXT_TO_BE_CREATED;

			if (!msg->gtpc_msg.csr.sender_fteid_ctl_plane.header.len)
				rsp_info->offending = GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT;

			if (!msg->gtpc_msg.csr.imsi.header.len)
				rsp_info->offending = GTP_IE_IMSI;

			if (!msg->gtpc_msg.csr.apn_ambr.header.len)
				rsp_info->offending = GTP_IE_AGG_MAX_BIT_RATE;

			if (!msg->gtpc_msg.csr.pdn_type.header.len)
					rsp_info->offending = GTP_IE_PDN_TYPE;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.bearer_lvl_qos.header.len)
				rsp_info->offending = GTP_IE_BEARER_QLTY_OF_SVC;

			if (!msg->gtpc_msg.csr.rat_type.header.len)
				rsp_info->offending = GTP_IE_RAT_TYPE;

			if (!msg->gtpc_msg.csr.apn.header.len)
				rsp_info->offending = GTP_IE_ACC_PT_NAME;

			break;
		}

		case PFCP_ASSOCIATION_SETUP_RESPONSE:{

			upf_context_t *upf_context = NULL;

			/*Retrive association state based on UPF IP. */
			ret = rte_hash_lookup_data(upf_context_by_ip_hash,
					(const void*) &(msg->upf_ipv4.s_addr), (void **) &(upf_context));
			if(ret < 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UPF context not found for Msg_Type:%u, UPF IP:%u\n",
										    __file__, __func__, __LINE__,msg->msg_type, msg->upf_ipv4.s_addr);
				return;
			}

	        RTE_SET_USED(index);
            context_key *key = LIST_FIRST(&upf_context->pendingCSRs);
            assert(key == NULL);
            LIST_REMOVE(key, csrentries);
			if (get_ue_context(key->teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->ebi_index = key->ebi_index + 5;
			rsp_info->teid = key->teid;
            // ajay - memleak .. Prio1.. Shopuld we not free the key ?
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {


			if(get_sess_entry(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid, &resp) != 0) {

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						 __file__, __func__, __LINE__, msg->msg_type, msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid, ret);
			}

			if(get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid), &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid);
			if(resp)
				rsp_info->ebi_index = resp->eps_bearer_id + 5;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{


			if (get_ue_context_while_error(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			if(msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				rsp_info->ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid = msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid;
			break;
		}

		case GTP_MODIFY_BEARER_REQ:{

			rsp_info->seq = msg->gtpc_msg.mbr.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mbr.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mbr.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;
			if (get_ue_context(msg->gtpc_msg.mbr.header.teid.has_teid.teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case GTP_MODIFY_BEARER_RSP: {

			rsp_info->seq = msg->gtpc_msg.mb_rsp.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi;

			if (get_ue_context_while_error(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0){
							clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);

			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_MODIFICATION_RESPONSE: {

			if(get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid, &resp) != 0) {

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]: Session entry not found Msg_Type:%u, Sess ID:%lu, Error_no:%d\n",
						 __file__, __func__, __LINE__, msg->msg_type, msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid, ret);
			}


			if (get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid), &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
			if(resp)
				rsp_info->ebi_index = resp->eps_bearer_id;
			break;
		}

		case GTP_DELETE_SESSION_REQ: {

			rsp_info->seq = msg->gtpc_msg.dsr.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.dsr.header.teid.has_teid.teid;

			if(get_ue_context(msg->gtpc_msg.dsr.header.teid.has_teid.teid,
							&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {

			if (get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid), &context)!= 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			rsp_info->ebi_index = UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			break;

		}

		case GTP_DELETE_SESSION_RSP: {

			if(get_ue_context_while_error(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid, &context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid;
			break;
		}
#ifdef GX_BUILD
		case GX_CCA_MSG: {
			if(parse_gx_cca_msg(&msg->gx_msg.cca, &pdn) < 0) {
				return;
			}
			if(pdn != NULL && pdn->context != NULL ) {
				context = pdn->context;
				rsp_info->ebi_index = pdn->default_bearer_id;
				rsp_info->sender_teid = context->s11_mme_gtpc_teid;
				rsp_info->seq = context->sequence;
				rsp_info->teid = context->s11_sgw_gtpc_teid;
			}

			break;
		}
#endif /*GX_BUILD*/

		case GTP_UPDATE_BEARER_REQ :{
			if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.ub_req.header.teid.has_teid.teid,
																				&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_req.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_req.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_req.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
			break;
		}

		case GTP_UPDATE_BEARER_RSP:{

			if(get_ue_context(msg->gtpc_msg.ub_rsp.header.teid.has_teid.teid, &context)){

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_rsp.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_rsp.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_rsp.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
			break;
		}
	}
}

void cs_error_response(msg_info *msg, uint8_t cause_value, int iface) 
{

	upf_context_t *upf_context = NULL;
	int ret = 0;

	ret = rte_hash_lookup_data(upf_context_by_ip_hash,
					(const void*) &(msg->upf_ipv4.s_addr), (void **) &(upf_context));

    while(true) {
		err_rsp_info rsp_info = {0};
		get_error_rsp_info(msg, &rsp_info);

				//Sending CCR-T in case of failure
#ifdef GX_BUILD
		if (cp_config->cp_type != SGWC){
			send_ccr_t_req(msg, rsp_info.ebi_index, rsp_info.teid);
            		struct sockaddr_in saddr_in;
            		saddr_in.sin_family = AF_INET;
            		inet_aton("127.0.0.1", &(saddr_in.sin_addr));
            		update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_TERMINATE, SENT, GX);
		}
#endif
		bzero(&tx_buf, sizeof(tx_buf));
		gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) tx_buf;

		create_sess_rsp_t cs_resp = {0};

		set_gtpv2c_teid_header(&cs_resp.header,
				GTP_CREATE_SESSION_RSP,
				rsp_info.sender_teid,
				rsp_info.seq);

		 if(cause_value == GTPV2C_CAUSE_MANDATORY_IE_MISSING ){
			set_ie_header(&cs_resp.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
			           sizeof(struct cause_ie));
			 cs_resp.cause.offend_ie_type = rsp_info.offending;
			 cs_resp.cause.offend_ie_len = 0;
			      }
		   else{
			set_ie_header(&cs_resp.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
		                  sizeof(struct cause_ie_hdr_t));
		   }
		   cs_resp.cause.cause_value = cause_value;
		   cs_resp.cause.pce = 0;
		   cs_resp.cause.bce = 0;
		   cs_resp.cause.spareinstance = 0;
		   if(cp_config->cp_type != SGWC || cp_config->cp_type !=SAEGWC )
		         cs_resp.cause.cs = 1;
		   else
		         cs_resp.cause.cs = 0;



		set_ie_header(&cs_resp.bearer_contexts_created.header, GTP_IE_BEARER_CONTEXT,
					IE_INSTANCE_ZERO, 0);


		set_ebi(&cs_resp.bearer_contexts_created.eps_bearer_id, IE_INSTANCE_ZERO,
					rsp_info.ebi_index);

		cs_resp.bearer_contexts_created.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

		set_cause_error_value(&cs_resp.bearer_contexts_created.cause, IE_INSTANCE_ZERO, cause_value);

		cs_resp.bearer_contexts_created.header.len += sizeof(struct cause_ie_hdr_t) + IE_HEADER_SIZE;

		uint16_t msg_len = 0;
		msg_len = encode_create_sess_rsp(&cs_resp,(uint8_t *)gtpv2c_tx);
		gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

		payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

		ret = clean_up_while_error(rsp_info.ebi_index,
						rsp_info.teid,&msg->gtpc_msg.csr.imsi.imsi_number_digits,
						msg->gtpc_msg.csr.imsi.header.len, rsp_info.seq);
		if(iface == S11_IFACE){
			gtpv2c_send(s11_fd, tx_buf, payload_length,
					(struct sockaddr *) &s11_mme_sockaddr, s11_mme_sockaddr_len);
			update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S11);
		}else{
			gtpv2c_send(s5s8_fd, tx_buf, payload_length,
					 (struct sockaddr *)&s5s8_recv_sockaddr,s5s8_sockaddr_len);

			struct sockaddr_in *s5s8_ip = (struct sockaddr_in *)&s5s8_recv_sockaddr;

			update_cli_stats(s5s8_ip->sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S5S8);

		}

        printf("%s %d - msg type = %d \n",__FUNCTION__, __LINE__, msg->msg_type);
        if(msg->msg_type != PFCP_ASSOCIATION_SETUP_RESPONSE)
            break;

	    if(upf_context != NULL && (msg->msg_type == PFCP_ASSOCIATION_SETUP_RESPONSE)
	    	&& (msg->pfcp_msg.pfcp_ass_resp.cause.cause_value != REQUESTACCEPTED)){
            if(LIST_EMPTY(&upf_context->pendingCSRs))
                break;
        }
	}
	RTE_SET_USED(ret);
}


void mbr_error_response(msg_info *msg, uint8_t cause_value, int iface)
{

	err_rsp_info rsp_info = {0};

	get_error_rsp_info(msg, &rsp_info);

	bzero(&tx_buf, sizeof(tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) tx_buf;

	mod_bearer_rsp_t mb_resp = {0};
	set_gtpv2c_teid_header(&mb_resp.header,
			GTP_MODIFY_BEARER_RSP,
			rsp_info.sender_teid,
			rsp_info.seq);

	set_cause_error_value(&mb_resp.cause, IE_INSTANCE_ZERO, cause_value);

	set_ie_header(&mb_resp.bearer_contexts_modified.header,
			GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO, 0);

	set_cause_error_value(&mb_resp.bearer_contexts_modified.cause,
			IE_INSTANCE_ZERO,cause_value);


	mb_resp.bearer_contexts_modified.header.len += sizeof(struct cause_ie_hdr_t) +
													IE_HEADER_SIZE;

	set_ebi(&mb_resp.bearer_contexts_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			rsp_info.ebi_index);

	 mb_resp.bearer_contexts_modified.header.len += sizeof(uint8_t)+ IE_HEADER_SIZE;

	ue_context_t *context = NULL;

	if (get_ue_context(rsp_info.teid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
	}

	uint16_t msg_len = 0;
	msg_len = encode_mod_bearer_rsp(&mb_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

	if(iface == S11_IFACE){
		gtpv2c_send(s11_fd, tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr, s11_mme_sockaddr_len);

		update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S11);
	}else{
		gtpv2c_send(s5s8_fd, tx_buf, payload_length,
				(struct sockaddr *)&s5s8_recv_sockaddr, s5s8_sockaddr_len);

		struct sockaddr_in *s5s8_ip = (struct sockaddr_in *)&s5s8_recv_sockaddr;

		update_cli_stats(s5s8_ip->sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S5S8);
	}
}


void ds_error_response(msg_info *msg, uint8_t cause_value, int iface)
{
	int ret = 0;
	uint8_t eps_bearer_id = 0;
	pdn_connection_t *pdn = NULL;
	ue_context_t *context = NULL;
	err_rsp_info rsp_info = {0};

	get_error_rsp_info(msg, &rsp_info);

	if(get_ue_context_while_error(rsp_info.teid, &context) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
		return;
	}
	if(context != NULL ) {
		for(int8_t idx = 0 ; idx < MAX_BEARERS; idx++) {
			pdn = context->pdns[idx];
			if(pdn == NULL) {
				continue;
			} else {
				eps_bearer_id = UE_BEAR_ID(pdn->seid);
			}
		}
	}
	if(eps_bearer_id == 0 && rsp_info.ebi_index != 0) {
		eps_bearer_id = rsp_info.ebi_index;
	} else {
		if(eps_bearer_id == 0)
			return;
	}

#ifdef GX_BUILD
	if (cp_config->cp_type != SGWC) {
		send_ccr_t_req(msg, eps_bearer_id, rsp_info.teid);
		struct sockaddr_in saddr_in;
		saddr_in.sin_family = AF_INET;
		inet_aton("127.0.0.1", &(saddr_in.sin_addr));
		update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_TERMINATE, SENT, GX);
	}
#endif
	bzero(&tx_buf, sizeof(tx_buf));

	bzero(&tx_buf, sizeof(tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) tx_buf;

	del_sess_rsp_t ds_resp = {0};

	set_gtpv2c_teid_header(&ds_resp.header,
								   GTP_DELETE_SESSION_RSP,
								   rsp_info.sender_teid,
								   rsp_info.seq);

	set_cause_error_value(&ds_resp.cause, IE_INSTANCE_ZERO, cause_value);

	uint16_t msg_len = 0;
	msg_len = encode_del_sess_rsp(&ds_resp,(uint8_t *)gtpv2c_tx);
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;
	header->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
	ret = clean_up_while_error(eps_bearer_id, rsp_info.teid, &context->imsi, context->imsi_len, rsp_info.seq);
	if( ret < 0 ) {
		return;
	}
	if(iface == S11_IFACE){
		gtpv2c_send(s11_fd, tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr, s11_mme_sockaddr_len);
		update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S11);
	}else{
		gtpv2c_send(s5s8_fd, tx_buf, payload_length,
				(struct sockaddr *)&s5s8_recv_sockaddr, s5s8_sockaddr_len);

		struct sockaddr_in *s5s8_ip = (struct sockaddr_in *)&s5s8_recv_sockaddr;
		update_cli_stats(s5s8_ip->sin_addr.s_addr,gtpv2c_tx->gtpc.message_type,REJ,S5S8);
	}

}


void ubr_error_response(msg_info *msg, uint8_t cause_value, int iface)
{

	int ret = 0;
	err_rsp_info rsp_info = {0};
	int ebi_index = 0;
	get_error_rsp_info(msg, &rsp_info);
	bzero(&tx_buf, sizeof(tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)tx_buf;

	if(iface == S5S8_IFACE){
		upd_bearer_rsp_t ubr_rsp = {0};

		set_gtpv2c_teid_header(&ubr_rsp.header,
									GTP_UPDATE_BEARER_RSP,
									rsp_info.sender_teid,
									rsp_info.seq);
		set_cause_error_value(&ubr_rsp.cause, IE_INSTANCE_ZERO,
													cause_value);
		ubr_rsp.bearer_context_count = rsp_info.bearer_count;
		for(int i = 0; i < rsp_info.bearer_count; i++){

			set_ie_header(&ubr_rsp.bearer_contexts[i].header, GTP_IE_BEARER_CONTEXT,
				IE_INSTANCE_ZERO, 0);

			set_ebi(&ubr_rsp.bearer_contexts[i].eps_bearer_id, IE_INSTANCE_ZERO,
															rsp_info.bearer_id[i]);
			ubr_rsp.bearer_contexts[i].header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

			set_cause_error_value(&ubr_rsp.bearer_contexts[i].cause, IE_INSTANCE_ZERO,
																			cause_value);
			ubr_rsp.bearer_contexts[i].header.len += sizeof(uint16_t) + IE_HEADER_SIZE;
		}

		uint16_t msg_len = 0;
		msg_len = encode_upd_bearer_rsp(&ubr_rsp, (uint8_t *)gtpv2c_tx);
		gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
		payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
			//send S5S8 interface update bearer response.
		gtpv2c_send(s5s8_fd, tx_buf, payload_length,
		   	  		(struct sockaddr *) &s5s8_recv_sockaddr,
					s5s8_sockaddr_len);
	}else{
		ebi_index = rsp_info.bearer_id[0] - 5;
		ue_context_t *context = NULL;
		pdn_connection_t *pdn_cntxt = NULL;

		if(msg->msg_type == GTP_UPDATE_BEARER_REQ){
			ret = get_ue_context_by_sgw_s5s8_teid(rsp_info.teid, &context);
		}else{
			ret = get_ue_context(rsp_info.teid, &context);
		}

		if (ret) {
			clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
			return;
		}
		pdn_cntxt = context->eps_bearers[ebi_index]->pdn;
#ifdef GX_BUILD
		gen_reauth_error_response(pdn_cntxt, DIAMETER_UNABLE_TO_COMPLY);
#else
		RTE_SET_USED(pdn_cntxt);
#endif /* GX_BUILD */
	}

}

/* Function to Fill and Send  Version not supported response to peer node */
void send_version_not_supported(int iface, uint32_t seq)
{

	bzero(&tx_buf, sizeof(tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) tx_buf;
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;

	set_gtpv2c_header(header, 0, GTP_VERSION_NOT_SUPPORTED_IND, 0, seq);


	uint16_t msg_len = 0;
	msg_len = encode_gtpv2c_header_t(header, (uint8_t *)gtpv2c_tx);
	header->gtpc.message_len = htons(msg_len - 4);

	payload_length = msg_len;
	if(iface == S11_IFACE){
		gtpv2c_send(s11_fd, tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr, s11_mme_sockaddr_len);

	}else{
		gtpv2c_send(s5s8_fd, tx_buf, payload_length,
				(struct sockaddr *)&s5s8_recv_sockaddr, s5s8_sockaddr_len);

	}
	return;
}
