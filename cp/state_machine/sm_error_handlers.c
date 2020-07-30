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
#include "upf_struct.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"

extern struct rte_hash *bearer_by_fteid_hash;
extern udp_sock_t my_sock;

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
    assert(context == msg->ue_context); 

    ebi_index = info_resp.ebi_index;
    pdn = GET_PDN(context ,ebi_index);

    del_sess_entry(pdn->seid); 

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

    printf("Delete all bearers \n");
    // if all bearers released then release pdn context 
    if(pdn->num_bearer == 0) {
        rte_hash_del_key(pdn_by_fteid_hash, (const void*) &teid);
        if(pdn->s5s8_sgw_gtpc_teid != 0) {
            rte_hash_del_key(bearer_by_fteid_hash, (const void *)
                    &(pdn->s5s8_sgw_gtpc_teid));
        }
        rte_free(pdn);
        context->num_pdns --;
    }

    printf("Delete all PDNs \n");
    proc_context_t *proc = context->current_proc;
    upf_context_t *upf_context = context->upf_context;

    // if all PDNs released then release user context 
    if (context->num_pdns == 0) {
        rte_hash_del_key(ue_context_by_imsi_hash,(const void *) &(*context).imsi);
        rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid);
        rte_free(context);
        context = NULL;
    }

    if(upf_context != NULL) {
	    pending_proc_key_t *key = NULL;
        LIST_FOREACH(key, &upf_context->pendingProcs, procentries) {
            if(key != NULL && (key->proc_context == (void *)proc) ) {
                LIST_REMOVE(key, procentries);
                rte_free(key);
                break;
            }
        }
    }

    printf("Delete Proc if any \n");
    if(proc != NULL) {
        if(proc->gtpc_trans != NULL) {
            printf("Delete gtpc procs \n");
            /* Only MME initiated transactions as of now */
            uint16_t port_num = proc->gtpc_trans->peer_sockaddr.sin_port; 
            uint32_t sender_addr = proc->gtpc_trans->peer_sockaddr.sin_addr.s_addr; 
            uint32_t seq_num = proc->gtpc_trans->sequence; 
            transData_t *gtpc_trans = delete_gtp_transaction(sender_addr, port_num, seq_num);
            assert(gtpc_trans == proc->gtpc_trans);
            stop_transaction_timer(proc->gtpc_trans);
            free(proc->gtpc_trans);
            proc->gtpc_trans = NULL;
        }
        if(proc->pfcp_trans != NULL) {
            // only self initiated transactions as of now 
            printf("Delete pfcp procs \n");
            uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
            uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
            uint32_t seq_num = proc->pfcp_trans->sequence; 
            transData_t *pfcp_trans = delete_pfcp_transaction(local_addr, port_num, seq_num);
            assert(pfcp_trans != NULL);
            assert(pfcp_trans == proc->pfcp_trans);
            stop_transaction_timer(proc->pfcp_trans);
            free(proc->pfcp_trans);
            proc->pfcp_trans = NULL;
        }
        free(proc);
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
                rte_hash_del_key(upf_context_by_ip_hash, (const void *) &pdn->upf_ipv4.s_addr);
                pending_proc_key_t *key;
                key = LIST_FIRST(&upf_ctx->pendingProcs);
                while (key != NULL) {
                    LIST_REMOVE(key, procentries);
                    rte_free(key);
                    key = LIST_FIRST(&upf_ctx->pendingProcs);
                }
                rte_free(upf_ctx);
                upf_ctx = NULL;
            }
        }

        del_sess_entry(pdn->seid); 

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
            rte_hash_del_key(pdn_by_fteid_hash, (const void*) &teid);
            if(pdn->s5s8_sgw_gtpc_teid != 0) {
                rte_hash_del_key(bearer_by_fteid_hash, (const void *)
                        &(pdn->s5s8_sgw_gtpc_teid));
            }
            rte_free(pdn);
            context->num_pdns --;
        }

        if (context->num_pdns == 0) {
            rte_hash_del_key(ue_context_by_imsi_hash,(const void *) &(*context).imsi);
            rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid);
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

							if(pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg,encoded, &context->upf_context->upf_sockaddr) < 0) {
								fprintf(stderr , " %s:%s:%u Error sending: %i\n",
										__FILE__, __func__, __LINE__, errno);
							}else {
                            transData_t *trans_entry;
							trans_entry = start_pfcp_session_timer(context, pfcp_msg, encoded, clean_up_while_error_pfcp_timeout);
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
                                            LIST_FOREACH(key, &upf_context->pendingProcs, procentries) {
												if(key != NULL && key->teid == context->s11_sgw_gtpc_teid ) {
                                                    LIST_REMOVE(key, procentries);
                                                    rte_free(key);
                                                    break;
                                                }
                                            }
                                            if(LIST_EMPTY(&upf_context->pendingProcs)) {
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
		rte_hash_del_key(ue_context_by_fteid_hash,(const void *) &teid); // TODO - teid should be taken from context 
		rte_free(context);
		context = NULL;
	}
	return 0;
	RTE_SET_USED(seq);
}
#endif
