// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "gx_interface.h"
#include "clogger.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "rte_hash.h"
#include "pfcp_cp_set_ie.h" // ajay - included for Gx context. need cleanup  
#include "pfcp.h"
#include "sm_structs_api.h"
#include "tables/tables.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "spgw_cpp_wrapper.h"

static 
void dispatch_rar(msg_info_t *msg)
{
	if (cp_config->cp_type == SAEGWC){
        switch(msg->proc)
        {
            case DED_BER_ACTIVATION_PROC:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg, NULL);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg, NULL);
                }
                break;
            }
            case PDN_GW_INIT_BEARER_DEACTIVATION:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg, NULL);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg, NULL);
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
                    process_rar_request_handler(msg, NULL);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg, NULL);
                }
                break;
            }
            case PDN_GW_INIT_BEARER_DEACTIVATION:
            {
                if(msg->state == CONNECTED_STATE) {
                    process_rar_request_handler(msg, NULL);
                } else if (msg->state == IDEL_STATE) {
                    process_rar_request_handler(msg, NULL);
                }
                break;
            }
        }
    }
    return;
}

int handle_rar_msg(msg_info_t *msg)
{
    int ret;
    uint32_t call_id;
	gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_RAR,saddr_in.sin_addr.s_addr);
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
    ret = get_gx_context((uint8_t *)msg->gx_msg.rar.session_id.val,&gx_context);
    if (ret < 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
                msg->gx_msg.rar.session_id.val);
        return -1;
    }
    /* Reteive the rqst ptr for RAA */
    uint32_t buflen = gx_rar_calc_length (&msg->gx_msg.rar);
    RTE_SET_USED(buflen);
#ifdef BADCODE
    //gx_context->rqst_ptr = (uint64_t *)(((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + buflen));
    memcpy( &gx_context->rqst_ptr ,((unsigned char *)msg->gx_msg.rar + sizeof(msg->gx_msg.msg_type) + buflen), sizeof(unsigned long));
#endif

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

    return 0;
}
int
process_rar_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = parse_gx_rar_msg(&msg->gx_msg.rar);
	if (ret) {
		if(ret != -1){
			uint32_t call_id = 0;
			pdn_connection_t *pdn_cntxt = NULL;
			ret = retrieve_call_id((char *)&msg->gx_msg.rar.session_id.val, &call_id);
			if (ret < 0) {
	        		clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                        msg->gx_msg.rar.session_id.val);
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
			gen_reauth_error_response(pdn_cntxt, ret);
		}
		clLog(sxlogger, eCLSeverityCritical, "%s:%s:%d Error: %d \n",
				__FILE__, __func__, __LINE__, ret);
		return -1;
	}
	RTE_SET_USED(unused_param);
	return 0;
}

