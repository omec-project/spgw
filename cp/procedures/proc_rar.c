// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "proc_rar.h"
#include "cp_proc.h"
#include "rte_debug.h"
#include "rte_common.h"
#include "ue.h"
#include "gx_error_rsp.h"
#include "proc_bearer_create.h"
#include "gx_interface.h"
#include "cp_log.h"

proc_context_t*
alloc_rar_proc(msg_info_t *msg)
{
    proc_context_t *rar_proc;

    rar_proc = calloc(1, sizeof(proc_context_t));
    rar_proc->proc_type = msg->proc; 
    rar_proc->ue_context = (void *)msg->ue_context;
    rar_proc->pdn_context = (void *)msg->pdn_context; 
    rar_proc->bearer_context = (void *)msg->bearer_context;
    rar_proc->handler = rar_req_event_handler;
    rar_proc->rar_seq_num = msg->rar_seq_num; 
    msg->proc_context = rar_proc;
    SET_PROC_MSG(rar_proc, msg);
 
    return rar_proc;
}

void
rar_req_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    RTE_SET_USED(proc_context);
    msg_info_t *msg = (msg_info_t *)msg_info;
    uint8_t event = msg->event;
    switch(event) {
        case RE_AUTH_REQ_RCVD_EVNT: {
            // outcome can be - PDN GW initiated bearer creation, deletion 
            process_rar_request_handler(msg, NULL);
//#define TEST_RAR
#ifdef TEST_RAR
            LOG_MSG(LOG_DEBUG2,"Send RAA now ?? "); 
            process_create_bearer_resp_and_send_raa(proc_context);
#endif
            break;
        }
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            if(proc_context->cbrsp_received == false) {
                process_pfcp_sess_mod_resp_cbr_handler(msg, proc_context);
            } else {
               LOG_MSG(LOG_DEBUG2, "Send RAA now ?? "); 
               process_create_bearer_resp_and_send_raa(proc_context);
            }
            break;
        }
        case CREATE_BER_RESP_RCVD_EVNT: {
            proc_context->cbrsp_received = true;
            process_sgwc_create_bearer_rsp(proc_context, msg);
            break;
        }
        default: {
            rte_panic("wrong event"); 
        }
    }
}

int
process_rar_request_handler(void *data, void *unused_param)
{
    int ret = 0;
    msg_info_t *msg = (msg_info_t *)data;
    proc_context_t *proc_ctxt = msg->proc_context;
    pdn_connection_t *pdn_cntxt = proc_ctxt->pdn_context;

	ret = parse_gx_rar_msg(msg);
	if (ret) {
		if(ret != -1){
			RTE_SET_USED(unused_param);
			gen_reauth_error_response(pdn_cntxt, ret);
		}
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return -1;
	}
	// Now we may have  one/more of the following action,
	// 1. Delete PDN if all the rules are deleted for default bearer
	// 2. Update default bearer QoS
	// 3. Update default bearer to remove/add some rules
	// 4. Update dedicated bearer to remote/add some rules
	// 5. Create dedicated bearer to install rules

	// Update default bearer QoS
    if(pdn_cntxt->policy.default_bearer_qos_valid == true) {
        // check if default bearer qos is changed. If yes
        // create procedure and link that procedure with RAR proc
        // default bearer QoS : QCI, MBR Rate --- Bearer QoS
        // APN AMBR           : APN AMBR UL/DL : part of QoS information AVP
        // return
    }

#if 0
    // Update default bearer to remove/add some rules
	if(pdn_cntxt->policy.num_charg_rule_modify) {
        /*GXFIX : we are not updating pfcp session ? when should we do it ? */
        /* What if RAR is received which involves modify as well as addition of new rules, we can not
         * just return from here...*/
		return gx_update_bearer_req(pdn_cntxt);
	}
#endif

	return 0;
}

