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
#include "spgw_cpp_wrapper.h"


#if 0
static 
void dispatch_cca(msg_info_t *msg)
{
	if (cp_config->cp_type == SAEGWC) {
        switch(msg->proc) {
#ifdef FUTURE_NEED
            case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC: 
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    del_bearer_cmd_ccau_handler(msg, NULL);
                }
                break;
            }
#endif
            default:
            {
                assert(0);
            }
        }
    }
#ifdef FUTURE_NEED
	else if (cp_config->cp_type == PGWC) {
        switch(msg->proc) {
            case SGW_RELOCATION_PROC:
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    cca_u_msg_handler_handover(msg, NULL);
                }
                break;
            }
            case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC: 
            {
                if(msg->state == CCRU_SNT_STATE)
                {
                    del_bearer_cmd_ccau_handler(msg, NULL);
                }
                break;
            }
            default:
            {
                assert(0);
            }
        }
    }
#endif
    return;
}
#endif

int handle_cca_update_msg(msg_info_t *msg)
{
    gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_U, saddr_in.sin_addr.s_addr);

    pdn_connection_t *pdn_cntxt = NULL;
    /* Retrive Gx_context based on Sess ID. */
    int ret = get_gx_context((uint8_t*)msg->gx_msg.cca.session_id.val,&gx_context);
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

    uint32_t call_id;
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
    proc_context_t *proc_context = (proc_context_t *)gx_context->proc_context;
    msg->event = CCA_RCVD_EVNT;
    clLog(sxlogger, eCLSeverityDebug, "%s: Callback called for"
            "Msg_Type:%s[%u], Session Id:%s, "
            "State:%s, Event:%s\n",
            __func__, gx_type_str(msg->msg_type), msg->msg_type,
            msg->gx_msg.cca.session_id.val,
            get_state_string(msg->state), get_event_string(msg->event));

    proc_context->msg_info = msg;
    msg->proc_context = proc_context;
    proc_context->handler(proc_context, msg);
    return 0;

}

/*
 * This function handles the message received
 * from PCEF in case of handover.
 * This handler comes when MBR is received
 * from the new SGWC on the PGWC.
 * */
int cca_u_msg_handler_handover(void *data, void *unused)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint32_t call_id = 0;
	pdn_connection_t *pdn = NULL;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer = NULL;

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&msg->gx_msg.cca.session_id.val, &call_id);
	if (ret < 0) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                       (char*) &msg->gx_msg.cca.session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn = get_pdn_conn_entry(call_id);
	if (pdn == NULL)
	{
	      clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
	                          __func__, call_id);
	      return -1;
	}

#ifdef TEMP
    proc_context_t *proc_context = pdn->context->current_proc;
    RTE_SET_USED(proc_context);
#endif

	ebi_index = pdn->default_bearer_id - 5; 

	if (!(pdn->context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet\n");
		return -EPERM;
	}

	bearer = pdn->eps_bearers[ebi_index];

#ifdef FUTURE_NEED
	ret = send_pfcp_sess_mod_req_handover(pdn, bearer, &resp->gtpc_msg.mbr);
	 if (ret) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
	         return ret;
	}
#endif
    RTE_SET_USED(ret);
    RTE_SET_USED(bearer);
    RTE_SET_USED(pdn);
	RTE_SET_USED(data);
	RTE_SET_USED(unused);

	return 0;
}

