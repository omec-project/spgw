// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
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
#include "upf_struct.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"

void
get_info_filled(msg_info_t *msg, err_rsp_info *info_resp)
{
	//pdn_connection_t *pdn = NULL;

	switch(msg->msg_type){
        case GTP_CREATE_SESSION_REQ: {
                info_resp->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                info_resp->teid =  msg->gtpc_msg.csr.header.teid.has_teid.teid;
                break;
            }

        case GTP_RESERVED: {
                ue_context_t *ue_context = msg->ue_context; 
			    info_resp->teid = ue_context->s11_sgw_gtpc_teid;
                // assumed to be triggered due to CSReq 
                info_resp->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                break;
            }

		case PFCP_ASSOCIATION_SETUP_RESPONSE:{
            ue_context_t  *ue = msg->ue_context; 
            pdn_connection_t *pdn = msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			// info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
            ue_context_t  *ue = msg->ue_context; 
            pdn_connection_t *pdn = msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			//info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{

			if(msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				info_resp->ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
			info_resp->teid = msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {

			info_resp->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			info_resp->ebi_index = UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid) - 5;

			break;
		}

	}
}
// case1 : pfcp association setup failure
int
process_error_occured_handler_new(void *data, void *unused_param)
{
    int ret = 0;
    msg_info_t *msg = (msg_info_t *)data;
    err_rsp_info info_resp = {0};
    ue_context_t *context;
    pdn_connection_t *pdn;
    uint8_t ebi_index = 0;

    get_info_filled(msg, &info_resp);
    uint32_t teid = info_resp.teid;

    if (get_ue_context_while_error(teid, &context) < 0) 
    {
        clLog(clSystemLog, eCLSeverityCritical, "%s:%d:SM_ERROR: Error handler UE_Proc: %u UE_State: %u "
                "%u and Message_Type:%s\n", __func__, __LINE__,
                msg->proc, msg->state,msg->event,
                gtp_type_str(msg->msg_type));

        RTE_SET_USED(unused_param);
        RTE_SET_USED(ret);
        return -1;
    }

    ebi_index = info_resp.ebi_index;
    pdn = GET_PDN(context ,ebi_index);

    del_sess_entry_seid(pdn->seid); 

    for(int8_t idx = 0; idx < MAX_BEARERS; idx++) {
        if(context->eps_bearers[idx] != NULL){
            rte_free(pdn->eps_bearers[idx]);
            pdn->eps_bearers[idx] = NULL;
            context->eps_bearers[idx] = NULL;
            if(pdn->num_bearer != 0) {
                pdn->num_bearer--;
            }
        }
    }

    pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
    while (pcc_rule != NULL) {
        TAILQ_REMOVE(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
        free(pcc_rule->dyn_rule);
        free(pcc_rule);
        pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
    }


    printf("Delete all bearers \n");
    // if all bearers released then release pdn context 
    if(pdn->num_bearer == 0) {
        pdn_context_delete_entry_teidKey(teid);
        if(pdn->s5s8_sgw_gtpc_teid != 0) {
            bearer_context_delete_entry_teidKey(pdn->s5s8_sgw_gtpc_teid);
        }
        rte_free(pdn);
        context->num_pdns --;
    }

    printf("Delete all PDNs \n");
    upf_context_t *upf_context = context->upf_context;

    proc_context_t *proc = TAILQ_FIRST(&context->pending_sub_procs);
    while(proc != NULL) {
      TAILQ_REMOVE(&context->pending_sub_procs, proc, next_sub_proc);
      if(upf_context != NULL) {
	      pending_proc_key_t *key = NULL;
          LIST_FOREACH(key, &upf_context->pending_sub_procs, procentries) {
              if(key != NULL && (key->proc_context == (void *)proc) ) {
                  LIST_REMOVE(key, procentries);
                  rte_free(key);
                  break;
              }
          }
      }

      if(proc != NULL) {
          if(proc->gtpc_trans != NULL) {
              printf("Delete gtpc procs \n");
              cleanup_gtpc_trans(proc->gtpc_trans);
          }
          if(proc->pfcp_trans != NULL) {
              printf("Delete pfcp procs \n");
              cleanup_gtpc_trans(proc->pfcp_trans);
          }
          free(proc);
      }
      proc = TAILQ_FIRST(&context->pending_sub_procs);
    }
    // if all PDNs released then release user context 
    if (context->num_pdns == 0) {
        ue_context_delete_entry_imsiKey((*context).imsi);
        ue_context_delete_entry_teidKey(teid);
        rte_free(context);
        context = NULL;
    }
    return 0;
}

#if 0
int
process_error_occured_handler(void *data, void *unused_param)
{
    int ret = 0;
    msg_info_t *msg = (msg_info_t *)data;
    err_rsp_info info_resp = {0};
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

    get_info_filled(msg, &info_resp);
    uint8_t ebi_index = info_resp.ebi_index;
    uint32_t teid = info_resp.teid;

    if (get_ue_context_while_error(teid, &context) == 0) {
        pdn = GET_PDN(context ,ebi_index);
        if ((upf_context_entry_lookup(pdn->upf_ipv4.s_addr,&upf_ctx)) ==  0) {
            if(upf_ctx->state < PFCP_ASSOC_RESP_RCVD_STATE){
                upf_context_delete_entry(&pdn->upf_ipv4.s_addr);
                pending_proc_key_t *key;
                key = LIST_FIRST(&upf_ctx->pending_sub_procs);
                while (key != NULL) {
                    LIST_REMOVE(key, procentries);
                    rte_free(key);
                    key = LIST_FIRST(&upf_ctx->pending_sub_procs);
                }
                rte_free(upf_ctx);
                upf_ctx = NULL;
            }
        }

        del_sess_entry_seid(pdn->seid); 

        for(int8_t idx = 0; idx < MAX_BEARERS; idx++) {
            if(context->eps_bearers[idx] != NULL){
                rte_free(pdn->eps_bearers[idx]);
                pdn->eps_bearers[idx] = NULL;
                context->eps_bearers[idx] = NULL;
                if(pdn->num_bearer != 0) {
                    pdn->num_bearer--;
                }
            }
        }

        if(pdn->num_bearer == 0) {
            pdn_context_delete_entry_teidKey(teid);
            if(pdn->s5s8_sgw_gtpc_teid != 0) {
                bearer_context_delete_entry_teidKey(pdn->s5s8_sgw_gtpc_teid);
            }
            rte_free(pdn);
            context->num_pdns --;
        }

        if (context->num_pdns == 0) {
            ue_context_delete_entry_imsiKey((*context).imsi);
            ue_context_delete_entry_teidKey(teid);
            rte_free(context);
            context = NULL;
        }
    }
    clLog(clSystemLog, eCLSeverityCritical, "%s:%d:SM_ERROR: Error handler UE_Proc: %u UE_State: %u "
            "%u and Message_Type:%s\n", __func__, __LINE__,
            msg->proc, msg->state,msg->event,
            gtp_type_str(msg->msg_type));

    RTE_SET_USED(unused_param);
    RTE_SET_USED(ret);
    return 0;
}
#endif

#ifdef FUTURE_NEED
int8_t
clean_up_while_error(uint8_t ebi, uint32_t teid, uint64_t *imsi_val, uint16_t imsi_len,uint32_t seq)
{
	uint64_t imsi = UINT64_MAX;
	ue_context_t *context = NULL;
	upf_context_t *upf_context = NULL;
	pdn_connection_t *pdn = NULL;
	pending_proc_key_t *key = NULL;
	uint8_t ebi_index = ebi - 5;
	int ret = 0;
	if(teid != 0) {
		if (get_ue_context_while_error(teid, &context) == 0) {
			//pdn = GET_PDN(context, ebi_index);
			if(context != NULL && context->eps_bearers[ebi_index] != NULL
				&& context->eps_bearers[ebi_index]->pdn != NULL) {
				pdn = context->eps_bearers[ebi_index]->pdn;
				if (pdn){
					if (get_sess_entry_seid(pdn->seid, &resp) == 0) {
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

							if(pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg,encoded, &context->upf_context->upf_sockaddr) < 0) {
								fprintf(stderr , " %s:%s:%u Error sending: %i\n",
										__FILE__, __func__, __LINE__, errno);
							}else {
                            transData_t *trans_entry;
							trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, clean_up_while_error_pfcp_timeout);
                            pdn->trans_entry = trans_entry;
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
							ret = ue_context_entry_lookup_imsiKey(imsi, &context);

							if (ret == -ENOENT){
								return -1;
							}else {
								ue_context_delete_entry_imsiKey(imsi);
								ue_context_delete_entry_teidKey(teid);
								rte_free(context);
								context = NULL;
							}
						}
						else {
							if(pdn != NULL) {
								if ((upf_context_entry_lookup(pdn->upf_ipv4.s_addr,&upf_context)) ==  0) {
									if(upf_context->state < PFCP_ASSOC_RESP_RCVD_STATE){
										if(ret >= 0) {
                                            LIST_FOREACH(key, &upf_context->pending_sub_procs, procentries) {
												if(key != NULL && key->teid == context->s11_sgw_gtpc_teid ) {
                                                    LIST_REMOVE(key, procentries);
                                                    rte_free(key);
                                                    break;
                                                }
                                            }
                                            if(LIST_EMPTY(&upf_context->pending_sub_procs)) {
									        	ret = upf_context_delete_entry(&pdn->upf_ipv4.s_addr);
												rte_free(upf_context);
												upf_context  = NULL;
                                            }
										}
									}
								}
							}
							ret = ue_context_delete_entry_imsiKey(context->imsi);
							if(ret < 0)
								return -1;
							ret = ue_context_delete_entry_teidKey(teid);
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
		ret = ue_context_entry_lookup_imsiKey(imsi,&context);

		if (ret == -ENOENT)
			return -1;

		ue_context_delete_entry_imsiKey(imsi);
		ue_context_delete_entry_teidKey(teid); // TODO - teid should be taken from context 
		rte_free(context);
		context = NULL;
	}
	return 0;
	RTE_SET_USED(seq);
}
#endif
