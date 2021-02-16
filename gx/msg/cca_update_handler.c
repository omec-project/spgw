// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include "gx_interface.h"
#include "cp_log.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "sm_struct.h"
#include "sm_hand.h"
#include "rte_hash.h"
#include "pfcp_cp_set_ie.h" // ajay - included for Gx context. need cleanup  
#include "pfcp.h"
#include "sm_structs_api.h"
#include "tables/tables.h"
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

int handle_cca_update_msg(msg_info_t **msg_p)
{
    msg_info_t *msg = *msg_p;
    gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_U, saddr_in.sin_addr.s_addr);

    pdn_connection_t *pdn_cntxt = NULL;
    /* Retrive Gx_context based on Sess ID. */
    int ret = get_gx_context((uint8_t*)msg->gx_msg.cca.session_id.val,&gx_context);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]",
                msg->gx_msg.cca.session_id.val);
        return -1;
    }

    if(msg->gx_msg.cca.presence.result_code &&
            msg->gx_msg.cca.result_code != 2001){
        LOG_MSG(LOG_ERROR, "Received CCA with DIAMETER Failure [%d]",
                msg->gx_msg.cca.result_code);
        return -1;
    }

    uint32_t call_id;
    /* Extract the call id from session id */
    ret = retrieve_call_id((char *)msg->gx_msg.cca.session_id.val, &call_id);
    if (ret < 0) {
        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s",
                msg->gx_msg.cca.session_id.val);
        return -1;
    }
    /* Retrieve PDN context based on call id */
    pdn_cntxt = get_pdn_conn_entry(call_id);
    if (pdn_cntxt == NULL) {
        LOG_MSG(LOG_ERROR, "No valid pdn cntxt found for CALL_ID:%u", call_id);
        return -1;
    }
    /* Retrive the Session state and set the event */
    proc_context_t *proc_context = (proc_context_t *)gx_context->proc_context;
    msg->event = CCA_RCVD_EVNT;
    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:%s[%u], Session Id:%s, "
            "State:%s, Event:%s",
            gx_type_str(msg->msg_type), msg->msg_type,
            msg->gx_msg.cca.session_id.val,
            get_state_string(msg->state), get_event_string(msg->event));

    SET_PROC_MSG(proc_context,msg);
    msg->proc_context = proc_context;
    proc_context->handler(proc_context, msg);
    // NOTE : this is important so that caller does not free msg pointer 
    *msg_p = NULL;
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

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&msg->gx_msg.cca.session_id.val, &call_id);
	if (ret < 0) {
	        LOG_MSG(LOG_ERROR, "No Call Id found from session id:%s",
	                       (char*) &msg->gx_msg.cca.session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn = get_pdn_conn_entry(call_id);
	if (pdn == NULL)
	{
	      LOG_MSG(LOG_ERROR, "No valid pdn cntxt found for CALL_ID:%u",
	                          call_id);
	      return -1;
	}

	ebi_index = pdn->default_bearer_id - 5; 

	if (!(pdn->context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet");
		return -EPERM;
	}


#ifdef FUTURE_NEED
	eps_bearer_t *bearer = NULL;
	bearer = pdn->eps_bearers[ebi_index];
	ret = send_pfcp_sess_mod_req_handover(pdn, bearer, &resp->gtpc_msg.mbr);
	 if (ret) {
	        LOG_MSG(LOG_ERROR, "Error: %d ", ret);
	         return ret;
	}
#endif
    LOG_MSG(LOG_NEVER, "unused = %p ", unused);
	return 0;
}

