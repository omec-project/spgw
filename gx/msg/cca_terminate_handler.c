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
#include "gen_utils.h"
#include "spgw_cpp_wrapper.h"


static 
void dispatch_cca(msg_info_t *msg)
{
	if (cp_config->cp_type == SAEGWC) {
        switch(msg->proc) {
            case DETACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE)
                {
                    cca_t_msg_handler(msg, NULL);
                }
                break;
            }
            default:
            {
                assert(0);
            }
        }
    }
#ifdef FUTURE_NEED
	else if (cp_config->cp_type == PGWC) {
        switch(msg->proc) {
            case DETACH_PROC:
            {
                if(msg->state == CCR_SNT_STATE)
                {
                    cca_t_msg_handler(msg, NULL);
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

int handle_ccr_terminate_msg(msg_info_t *msg)
{
    int ret;
    gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));

    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_T, saddr_in.sin_addr.s_addr);

    pdn_connection_t *pdn_cntxt = NULL;
    /* Retrive Gx_context based on Sess ID. */
    ret = get_gx_context((uint8_t*)msg->gx_msg.cca.session_id.val,&gx_context);
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
    uint32_t call_id;
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

    return 0;
}

/*
This function Handles the CCA-T received from PCEF
*/
int
cca_t_msg_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	gx_context_t *gx_context = NULL;

	RTE_SET_USED(unused_param);

	/* Retrive Gx_context based on Sess ID. */
	ret = get_gx_context(msg->gx_msg.cca.session_id.val, &gx_context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
				msg->gx_msg.cca.session_id.val);
		return -1;
	}

	if(remove_gx_context((uint8_t*)msg->gx_msg.cca.session_id.val) < 0){
		clLog(clSystemLog, eCLSeverityCritical,
				"%s %s - Error on gx_context_by_sess_id_hash deletion\n",__file__,
				strerror(ret));
	}

	rte_free(gx_context);
	return 0;
}
