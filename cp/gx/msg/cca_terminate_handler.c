// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "gx_interface.h"
#include "cp_log.h"
#include "spgw_config_struct.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "pfcp_cp_set_ie.h" // ajay - included for Gx context. need cleanup  
#include "pfcp.h"
#include "sm_structs_api.h"
#include "gen_utils.h"
#include "spgw_cpp_wrapper.h"
#include "proc.h"


static 
void dispatch_cca(msg_info_t *msg)
{
	if (cp_config->cp_type == SAEGWC) {
        switch(msg->proc) {
            case DETACH_PROC:
            {
                cca_t_msg_handler(msg, NULL);
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
                cca_t_msg_handler(msg, NULL);
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

int handle_ccr_terminate_msg(msg_info_t **msg_p)
{
    msg_info_t *msg = *msg_p;
    int ret;
    gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));

    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_T, saddr_in.sin_addr.s_addr);

    LOG_MSG(LOG_DEBUG, "find gx context ");

    /* Extract the call id from session id */
    uint32_t call_id;
    ret = retrieve_call_id((char *)msg->rx_msg.cca.session_id.val, &call_id);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s", 
                msg->rx_msg.cca.session_id.val);
        return -1;
    }

    /* Retrieve PDN context based on call id */
    pdn_connection_t *pdn_cntxt = (pdn_connection_t *)get_pdn_conn_entry(call_id);
    if (pdn_cntxt == NULL) {
        LOG_MSG(LOG_ERROR, "No valid pdn cntxt found for CALL_ID:%u", call_id);
        return -1;
    }

    /* Retrive Gx_context based on Sess ID. */
    ue_context_t *temp_ue_context  = (ue_context_t *)get_gx_context((uint8_t*)msg->rx_msg.cca.session_id.val);
    if (temp_ue_context == NULL) {
        LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", msg->rx_msg.cca.session_id.val);
        return -1;
    }
    gx_context = (gx_context_t*)temp_ue_context->gx_context;

    if(msg->rx_msg.cca.presence.result_code &&
            msg->rx_msg.cca.result_code != 2001){
        // TODO : GXMISSING - handle -ve cause codes as well and complete procedure 
        LOG_MSG(LOG_ERROR, "Received CCA-T with DIAMETER Failure [%d]",
                msg->rx_msg.cca.result_code);
        return -1;
    }

    /* Retrive the Session state and set the event */
    msg->event = CCA_RCVD_EVNT;
    msg->proc = gx_context->proc;
    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:%s[%u], Session Id:%s, "
            "Event:%s",
            gx_type_str(msg->msg_type), msg->msg_type,
            msg->rx_msg.cca.session_id.val,
            get_event_string(msg->event));

    dispatch_cca(msg); 
    // NOTE : this is important so that caller does not free msg pointer 
    *msg_p = NULL;

    return 0;
}

/*
This function Handles the CCA-T received from PCEF
*/
int
cca_t_msg_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	gx_context_t *gx_context = NULL;

    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);

	/* Retrive Gx_context based on Sess ID. */
	ue_context_t *temp_ue_context = (ue_context_t *) get_gx_context(msg->rx_msg.cca.session_id.val);
	if (temp_ue_context == NULL) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]",
				msg->rx_msg.cca.session_id.val);
		return -1;
	}

    gx_context = (gx_context_t *)temp_ue_context->gx_context;
	if(remove_gx_context((uint8_t*)msg->rx_msg.cca.session_id.val) < 0) {
		LOG_MSG(LOG_ERROR, "Error on gx_context_by_sess_id_hash deletion");
	}
    LOG_MSG(LOG_DEBUG, "Cleanup - gx session ");
	free(gx_context);
	return 0;
}
