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
#include "pfcp_association_setup_proc.h"
#include "gtpv2_error_rsp.h"
#include "spgw_cpp_wrapper.h"


static 
void dispatch_cca(msg_info_t *msg)
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
            default:
            {
                assert(0);
            }
        }
    }
#ifdef FUTURE_NEED
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
            default:
            {
                assert(0);
            }
        }
    }
#endif
    return;
}
int handle_cca_initial_msg(msg_info_t *msg)
{
	gx_context_t *gx_context = NULL;
    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));


    increment_gx_peer_stats(MSG_RX_DIAMETER_GX_CCA_I, saddr_in.sin_addr.s_addr);

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
This function Handles the msgs received from PCEF
*/
int
cca_msg_handler(void *data, void *unused_param)
{
    int ret = 0;
	int8_t ebi_index = 0;
	upf_context_t *upf_context = NULL;
	pdn_connection_t *pdn = NULL;

	msg_info_t *msg = (msg_info_t *)data;

	RTE_SET_USED(msg);

	/* Handle the CCR-T Message */
	if (msg->gx_msg.cca.cc_request_type == TERMINATION_REQUEST) {
		clLog(gxlogger, eCLSeverityDebug, FORMAT"Received GX CCR-T Response..!! \n",
				ERR_MSG);
		return 0;
	}

	/* VS: Retrive the ebi index */
	ret = parse_gx_cca_msg(&msg->gx_msg.cca, &pdn);
	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "Failed to establish session on PGWU, Send Failed CSResp back to SGWC\n");
		return ret;
	}

	ebi_index = pdn->default_bearer_id - 5;
    RTE_SET_USED(ebi_index);
	/* VS: Send the Association setup request */
	ret = process_pfcp_assoication_request(pdn->context);
	if (ret) {
		if(ret != -1){
			cs_error_response(msg, ret, cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			process_error_occured_handler_new(data, unused_param);
		}
		clLog(sxlogger, eCLSeverityCritical, "%s:%s:%d Error: %d \n",
				__FILE__, __func__, __LINE__, ret);
		return -1;
	}
	/* Retrive association state based on UPF IP. */
	ret = upf_context_entry_lookup((pdn->upf_ipv4.s_addr), &(upf_context));
#if 0
	/* send error response in case of pfcp est. fail using this data */
	if(upf_context->state == PFCP_ASSOC_RESP_RCVD_STATE) {
                ret = get_sess_entry_seid(pdn->seid, &resp);
                if(ret != -1 && resp != NULL){
                        if(cp_config->cp_type == PGWC) {
                                resp->gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key = pdn->s5s8_sgw_gtpc_teid;
                        }
                        if(cp_config->cp_type == SAEGWC) {
                                resp->gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key = pdn->context->s11_mme_gtpc_teid;
                        }
                        resp->gtpc_msg.csr.header.teid.has_teid.seq = pdn->context->sequence;
                        resp->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi = ebi_index + 5;
                        if (cp_config->cp_type == PGWC) {
                                /* : we need teid for send ccr-T to PCRF  */
                                resp->gtpc_msg.csr.header.teid.has_teid.teid = pdn->s5s8_pgw_gtpc_teid;
                        }
                        if(cp_config->cp_type == SAEGWC) {
                                 resp->gtpc_msg.csr.header.teid.has_teid.teid = pdn->context->s11_sgw_gtpc_teid;
                        }
                }
        }
#endif

	RTE_SET_USED(unused_param);
	return 0;
}


