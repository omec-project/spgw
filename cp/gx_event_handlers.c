// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gx_event_handlers.h"
#include "clogger.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "rte_hash.h"

extern struct rte_hash *gx_context_by_sess_id_hash;

static 
void dispatch_cca(msg_info *msg)
{
	if (cp_config->cp_type == SAEGWC) {
        switch(msg->proc) {
            case INITIAL_PDN_ATTACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE) 
                {
                    cca_msg_handler(msg, NULL);
                }
                break;
            }
            case DETACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE)
                {
                    cca_t_msg_handler(msg, NULL);
                }
                break;
            }
            case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC: 
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    del_bearer_cmd_ccau_handler(msg);
                }
                break;
            }
            default:
            {
                assert(0);
            }
        }
    }
	else if (cp_config->cp_type == PGWC) {
        switch(msg->proc) {
            case INITIAL_PDN_ATTACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE) 
                {
                    cca_msg_handler(msg, NULL);
                }
                break;
            }
            case SGW_RELOCATION_PROC:
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    cca_u_msg_handler_handover(msg);
                }
                break;
            }
            case DETACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE)
                {
                    cca_t_msg_handler(msg, NULL);
                }
                break;
            }
            case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC: 
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    del_bearer_cmd_ccau_handler(msg);
                }
                break;
            }
            default:
            {
                assert(0);
            }
        }
    }
    return;
}

static 
void dispatch_rar(msg_info *msg)
{
	if (cp_config->cp_type == SAEGWC){
        switch(msg->proc)
        {
            case DED_BER_ACTIVATION_PROC:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg);
                }
                break;
            }
            case PDN_GW_INIT_BEARER_DEACTIVATION:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg);
                }
                break;
            }
            /* TODO - RAR triggered bearer update missing*/
        }
    } 
	else if (cp_config->cp_type == PGWC) {
        switch(msg->proc)
        {
            case DED_BER_ACTIVATION_PROC:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg);
                }
                break;
            }
            case PDN_GW_INIT_BEARER_DEACTIVATION:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg);
                }
                break;
            }
        }
    }
    return;
}

int process_gx_message(gx_msg *gx_msg, msg_info *msg)
{
    int ret = 0;
	gx_context_t *gx_context = NULL;

    switch(msg->msg_type) 
    {
		case GX_CCA_MSG: 
        {
			/* Retrive Gx_context based on Sess ID. */
			ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
					(const void*)(msg->gx_msg.cca.session_id.val),
					(void **)&gx_context);
			if (ret < 0) {
			    clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
						msg->gx_msg.cca.session_id.val);
			    return -1;
			}

			if(msg->gx_msg.cca.presence.result_code &&
					msg->gx_msg.cca.result_code != 2001){
				clLog(clSystemLog, eCLSeverityCritical, "%s:Received CCA with DIAMETER Failure [%d]\n", __func__,
						msg->gx_msg.cca.result_code);
				return -1;
			}

			/* Extract the call id from session id */
			ret = retrieve_call_id((char *)msg->gx_msg.cca.session_id.val, &call_id);
			if (ret < 0) {
			        clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
			                        msg->gx_msg.cca.session_id.val);
			        return -1;
			}
			/* Retrieve PDN context based on call id */
			pdn_cntxt = get_pdn_conn_entry(call_id);
			if (pdn_cntxt == NULL)
			{
			      clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
			                          __func__, call_id);
			      return -1;
			}
			/* Retrive the Session state and set the event */
			msg->state = gx_context->state;
			msg->event = CCA_RCVD_EVNT;
			msg->proc = gx_context->proc;
			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Session Id:%s, "
					"State:%s, Event:%s\n",
					__func__, gx_type_str(msg->msg_type), msg->msg_type,
					msg->gx_msg.cca.session_id.val,
					get_state_string(msg->state), get_event_string(msg->event));

            dispatch_cca(msg); 
           break;
        }
        case GX_RAR_MSG:
        {
			ret = retrieve_call_id((char *)&msg->gx_msg.rar.session_id.val, &call_id);
			if (ret < 0) {
	        	clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	        	                msg->gx_msg.rar.session_id.val);
	       	 	return -1;
			}
			pdn_connection_t *pdn_cntxt = NULL;
			/* Retrieve PDN context based on call id */
			pdn_cntxt = get_pdn_conn_entry(call_id);
			if (pdn_cntxt == NULL)
			{
	     		 clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
	                								          __func__, call_id);
	     		 return -1;
			}
			/* Retrive Gx_context based on Sess ID. */
			ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
					(const void*)(msg->gx_msg.rar.session_id.val),
					(void **)&gx_context);
			if (ret < 0) {
			    clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
						msg->gx_msg.rar.session_id.val);
			    return -1;
			}
			/* Reteive the rqst ptr for RAA */
			buflen = gx_rar_calc_length (&msg->gx_msg.rar);
			//gx_context->rqst_ptr = (uint64_t *)(((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + buflen));
			memcpy( &gx_context->rqst_ptr ,((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + buflen),
					sizeof(unsigned long));

			pdn_cntxt->rqst_ptr = gx_context->rqst_ptr;
			/* Retrive the Session state and set the event */
			msg->state = CONNECTED_STATE;
			msg->event = RE_AUTH_REQ_RCVD_EVNT;
            /* RAR may trigger update bearer, delete bearer or create bearer... */
			msg->proc = DED_BER_ACTIVATION_PROC; 

			clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
					"Msg_Type:%s[%u], Session Id:%s, "
					"State:%s, Event:%s\n",
					__func__, gx_type_str(msg->msg_type), msg->msg_type,
					msg->gx_msg.cca.session_id.val,
					get_state_string(msg->state), get_event_string(msg->event));

            dispatch_rar(msg);
        }
    }
    return ret;
}
