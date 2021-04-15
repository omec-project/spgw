// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "pfcp.h"
#include "gx_interface.h"
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
#include "spgw_config_struct.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "upf_struct.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "util.h"
#include "cp_io_poll.h"
#include "cp_log.h"
#include "pfcp_cp_interface.h"
#include "pfcp_messages_encoder.h"

void
get_info_filled(msg_info_t *msg, err_rsp_info *info_resp)
{
	//pdn_connection_t *pdn = NULL;

	switch(msg->msg_type){
        case GTP_CREATE_SESSION_REQ: {
                info_resp->ebi_index = msg->rx_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                info_resp->teid =  msg->rx_msg.csr.header.teid.has_teid.teid;
                break;
            }

        case GTP_RESERVED: {
                ue_context_t *ue_context = (ue_context_t *)msg->ue_context; 
			    info_resp->teid = ue_context->s11_sgw_gtpc_teid;
                // assumed to be triggered due to CSReq 
                info_resp->ebi_index = msg->rx_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                break;
            }

		case PFCP_ASSOCIATION_SETUP_RESPONSE:{
            ue_context_t  *ue = (ue_context_t *)msg->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t *)msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			// info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
            ue_context_t  *ue = (ue_context_t *)msg->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t*)msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			//info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{

			if(msg->rx_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				info_resp->ebi_index = msg->rx_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
			info_resp->teid = msg->rx_msg.cs_rsp.header.teid.has_teid.teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {

			info_resp->teid = UE_SESS_ID(msg->rx_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			info_resp->ebi_index = UE_BEAR_ID(msg->rx_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid) - 5;

			break;
		}

	}
}
// case1 : pfcp association setup failure
int
process_error_occured_handler_new(void *data, void *unused_param)
{
    msg_info_t *msg = (msg_info_t *)data;
    err_rsp_info info_resp = {0};
    ue_context_t *context;
    pdn_connection_t *pdn;
    uint8_t ebi_index = 0;

    get_info_filled(msg, &info_resp);
    uint32_t teid = info_resp.teid;

    if (get_ue_context_while_error(teid, &context) < 0) 
    {
        LOG_MSG(LOG_ERROR, "SM_ERROR: Error handler UE_Proc: %u event : %u and Message_Type:%s",
                msg->proc, msg->event,
                gtp_type_str(msg->msg_type));

        LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
        return -1;
    }

    ebi_index = info_resp.ebi_index;
    pdn = GET_PDN(context ,ebi_index);

    //send delete pfcp message to UPF 
    if(pdn->dp_seid != 0) {
	    uint8_t pfcp_msg[512]={0};
	    pfcp_sess_del_req_t pfcp_sess_del_req = {0};
	    fill_pfcp_sess_del_req(&pfcp_sess_del_req);
	    pfcp_sess_del_req.header.seid_seqno.has_seid.seid = pdn->dp_seid;
	    int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	    header->message_len = htons(encoded - 4);
        if(context->upf_context != NULL) {
	        pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);
        }
    }

    do {
	    if ((cp_config->gx_enabled) && (cp_config->cp_type != SGWC)) {
	    	gx_context_t *gx_context = NULL;
	        gx_msg ccr_request = {0};
            uint16_t msglen;

	    	/* Retrive Gx_context based on Sess ID. */
	    	ue_context_t *temp_context  = (ue_context_t *)get_gx_context((uint8_t *)pdn->gx_sess_id); 
	    	if (temp_context == NULL) {
	    		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
                break;
	    	}
            assert(temp_context == context);
            gx_context = (gx_context_t *)temp_context->gx_context;

	    	/* VS: Set the Msg header type for CCR-T */
	    	ccr_request.msg_type = GX_CCR_MSG ;

	    	/* VS: Set Credit Control Request type */
	    	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	    	ccr_request.data.ccr.cc_request_type = TERMINATION_REQUEST ;

	    	/* VG: Set Credit Control Bearer opertaion type */
	    	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	    	ccr_request.data.ccr.bearer_operation = TERMINATION ;

	    	/* VS: Fill the Credit Crontrol Request to send PCRF */
	    	if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, pdn->gx_sess_id) != 0) {
	    		LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
                break;
	    	}

	    	/* VS: Set the Gx State for events */
	    	gx_context->state = CCR_SNT_STATE;

	    	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	    	msglen = gx_ccr_calc_length(&ccr_request.data.ccr);

            char *buffer = (char *)calloc(1, msglen + sizeof(ccr_request.msg_type));
            if (buffer == NULL) {
                LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
                break;
            }

            memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

            if (gx_ccr_pack(&(ccr_request.data.ccr),
                        (unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msglen) == 0) {
                LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
                break;
            }
	    	gx_send(my_sock.gx_app_sock, buffer,
	    			msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
            struct sockaddr_in saddr_in;
            saddr_in.sin_family = AF_INET;
            inet_aton("127.0.0.1", &(saddr_in.sin_addr));
            increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T, saddr_in.sin_addr.s_addr);
			if(remove_gx_context((uint8_t*)pdn->gx_sess_id) < 0){
				LOG_MSG(LOG_ERROR, " Error on gx_context_by_sess_id_hash deletion");
			}
            free(gx_context); 
            temp_context->gx_context = NULL;
            del_pdn_conn_entry(pdn->call_id);
        }
    }while(0);

    for(int8_t idx = 0; idx < MAX_BEARERS; idx++) {
        if(context->eps_bearers[idx] != NULL) {
            free(pdn->eps_bearers[idx]);
            pdn->eps_bearers[idx] = NULL;
            context->eps_bearers[idx] = NULL;
            if(pdn->num_bearer != 0) {
                pdn->num_bearer--;
            }
        }
    }

    LOG_MSG(LOG_DEBUG3, "Delete all bearers of %lu ", context->imsi64);
    // if all bearers released then release pdn context 
    if(pdn->num_bearer == 0) {
        if(pdn->s5s8_sgw_gtpc_teid != 0) {
            bearer_context_delete_entry_teidKey(pdn->s5s8_sgw_gtpc_teid);
        }
        cleanup_pdn_context(pdn);
    }

    LOG_MSG(LOG_DEBUG3, "Delete all PDNs of %lu ", context->imsi64);
    upf_context_t *upf_context = context->upf_context;

    proc_context_t *proc = TAILQ_FIRST(&context->pending_sub_procs);
    while(proc != NULL) {
      TAILQ_REMOVE(&context->pending_sub_procs, proc, next_sub_proc);
      if(upf_context != NULL) {
	      pending_proc_key_t *key = NULL;
          LIST_FOREACH(key, &upf_context->pending_sub_procs, procentries) {
              if(key != NULL && (key->proc_context == (void *)proc) ) {
                  LIST_REMOVE(key, procentries);
                  free(key);
                  break;
              }
          }
      }

      if(proc != NULL) {
          if(proc->gtpc_trans != NULL) {
              LOG_MSG(LOG_DEBUG, "Delete gtpc procs ");
              cleanup_gtpc_trans(proc->gtpc_trans);
          }
          if(proc->pfcp_trans != NULL) {
              LOG_MSG(LOG_DEBUG, "Delete pfcp procs ");
              cleanup_pfcp_trans(proc->pfcp_trans);
          }
          free(proc);
      }
      proc = TAILQ_FIRST(&context->pending_sub_procs);
    }
    // if all PDNs released then release user context 
    if (context->num_pdns == 0) {
        ue_context_delete_entry_imsiKey((*context).imsi);
        ue_context_delete_entry_teidKey(teid);
        free(context);
        context = NULL;
    }
    return 0;
}

#if 0
int
process_error_occured_handler(void *data, void *unused_param)
{
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
                    free(key);
                    key = LIST_FIRST(&upf_ctx->pending_sub_procs);
                }
                free(upf_ctx);
                upf_ctx = NULL;
            }
        }

        del_sess_entry_seid(pdn->seid); 

        for(int8_t idx = 0; idx < MAX_BEARERS; idx++) {
            if(context->eps_bearers[idx] != NULL){
                free(pdn->eps_bearers[idx]);
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
            free(pdn);
            context->num_pdns --;
        }

        if (context->num_pdns == 0) {
            ue_context_delete_entry_imsiKey((*context).imsi);
            ue_context_delete_entry_teidKey(teid);
            free(context);
            context = NULL;
        }
    }
    LOG_MSG(LOG_ERROR, "SM_ERROR: Error handler UE_Proc: %u UE_State: %u "
            "%u and Message_Type:%s",
            msg->proc, msg->state,msg->event,
            gtp_type_str(msg->msg_type));

    return 0;
}
#endif

#ifdef FUTURE_NEED
int8_t
clean_up_while_error(uint8_t ebi, uint32_t teid, uint64_t *imsi_val, uint16_t imsi_len,uint32_t seq)
{
	uint64_t imsi = UINT64_MAX;
	ue_context_t *context = NULL;
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
					resp = get_sess_entry_seid(pdn->seid);
					if (resp != NULL) {
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

							pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg,encoded, &context->upf_context->upf_sockaddr);
                            transData_t *trans_entry;
							trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, clean_up_while_error_pfcp_timeout);
                            pdn->trans_entry = trans_entry;
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
							context = (ue_context_t *) ue_context_entry_lookup_imsiKey(imsi);

							if (context == NULL){
								return -1;
							}else {
								ue_context_delete_entry_imsiKey(imsi);
								ue_context_delete_entry_teidKey(teid);
								free(context);
								context = NULL;
							}
						}
						else {
							if(pdn != NULL) {
	                            upf_context_t *upf_context = NULL;
								upf_context = (upf_context_t *)upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
								if (upf_context != NULL) {
									if(upf_context->state < PFCP_ASSOC_RESP_RCVD_STATE){
										if(ret >= 0) {
                                            LIST_FOREACH(key, &upf_context->pending_sub_procs, procentries) {
												if(key != NULL && key->teid == context->s11_sgw_gtpc_teid ) {
                                                    LIST_REMOVE(key, procentries);
                                                    free(key);
                                                    break;
                                                }
                                            }
                                            if(LIST_EMPTY(&upf_context->pending_sub_procs)) {
									        	ret = upf_context_delete_entry(&pdn->upf_ipv4.s_addr);
												free(upf_context);
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
								free(context);
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
		context = (ue_context_t *) ue_context_entry_lookup_imsiKey(imsi);

		if (context == NULL)
			return -1;

		ue_context_delete_entry_imsiKey(imsi);
		ue_context_delete_entry_teidKey(teid); // TODO - teid should be taken from context 
		free(context);
		context = NULL;
	}
	return 0;
}
#endif
