// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "proc_rar.h"
#include "proc_struct.h"
#include "ue.h"
#include "gx_error_rsp.h"
#include "proc_bearer_create.h"
#include "proc_nw_init_detach.h"
#include "gx_interface.h"
#include "cp_log.h"
#include "pfcp_cp_session.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "assert.h"
#include "pfcp.h"
#include "proc.h"


proc_context_t*
alloc_rar_proc(msg_info_t *msg)
{
    proc_context_t *rar_proc;

    rar_proc = (proc_context_t *)calloc(1, sizeof(proc_context_t));
    strcpy(rar_proc->proc_name, "RAR");
    rar_proc->proc_type = msg->proc; 
    rar_proc->ue_context = (void *)msg->ue_context;
    rar_proc->pdn_context = (void *)msg->pdn_context; 
    rar_proc->bearer_context = (void *)msg->bearer_context;
    rar_proc->handler = rar_req_event_handler;
    rar_proc->rar_seq_num = msg->rar_seq_num; 
    msg->proc_context = rar_proc;
    SET_PROC_MSG(rar_proc, msg);
    rar_proc->child_procs_cnt = 0;
    rar_proc->child_proc_add = add_rar_child_proc;
    rar_proc->child_proc_done = done_rar_child_proc;
 
    return rar_proc;
}

void
add_rar_child_proc(void *parent, void *child)
{
    proc_context_t *proc = (proc_context_t *)parent; 
    proc->child_procs[proc->child_procs_cnt] = child;
    proc->child_procs_cnt++;
    return;
}

void
done_rar_child_proc(void *child_proc)
{
    int i;
    proc_context_t *p_proc = (proc_context_t *)(((proc_context_t *)child_proc)->parent_proc);
    for(i=0; i<MAX_CHILD_PROC; i++) {
        if(p_proc->child_procs[i] == child_proc) {
            p_proc->child_procs_cnt--;
            p_proc->child_procs[i] = NULL;
            break;
        }
    }
    if(p_proc->child_procs_cnt == 0) {
        LOG_MSG(LOG_DEBUG,"All child procs complete, resume parent procedure");
        msg_info_t *msg = (msg_info_t*)p_proc->msg_info;
        msg->event = SEND_RE_AUTH_RSP_EVNT;
        /* We should use collective result of all child procedures */
        p_proc->result = ((proc_context_t *)child_proc)->result;
        p_proc->handler(p_proc, p_proc->msg_info);
    }
    return;
}

void
rar_req_event_handler(void *proc, void *msg_info)
{
    msg_info_t *msg = (msg_info_t *)msg_info;
    uint8_t event = msg->event;
    switch(event) {
        case RE_AUTH_REQ_RCVD_EVNT: {
            // outcome can be - PDN GW initiated bearer creation, deletion 
            process_rar_request_handler(msg);
            break;
        }
        case SEND_RE_AUTH_RSP_EVNT: {
            send_raa(msg);
            break;
        }
        default: {
            assert(0); // wrong event 
        }
    }
    LOG_MSG(LOG_NEVER, "proc = %p", proc);
}

void
proc_rar_failed(msg_info_t *msg, uint8_t cause )
{
    proc_context_t *proc_context = (proc_context_t *)msg->proc_context;
    proc_context->result = PROC_RESULT_FAILURE;
    increment_stat(PROCEDURES_SPGW_RAR_PROC_FAILURE);
    proc_rar_complete(proc_context);
    LOG_MSG(LOG_NEVER, "RAR failed with cause = %d ", cause);
}

void 
proc_rar_complete(proc_context_t *proc_context)
{
    end_procedure(proc_context);
}


void 
send_raa(msg_info_t *msg)
{
	char *send_buf =  NULL;
	uint32_t buflen;
    proc_context_t *proc = (proc_context_t*)msg->proc_context;

	gx_msg *resp = (gx_msg *)calloc(1,sizeof(gx_msg));

	/* Filling Header value of RAA */
	resp->msg_type = GX_RAA_MSG ;
	//create_bearer_resp_t cbresp = {0};

	//fill_raa_msg( &(resp->data.cp_raa), &cbresp );
    pdn_connection_t *pdn = (pdn_connection_t *)proc->pdn_context;
    resp->data.cp_raa.presence.session_id = PRESENT;
	resp->data.cp_raa.session_id.len = strlen(pdn->gx_sess_id);
	memcpy(resp->data.cp_raa.session_id.val, pdn->gx_sess_id, resp->data.cp_raa.session_id.len);

	/* Result code */
    if(proc->result  == PROC_RESULT_SUCCESS) {
	    resp->data.cp_raa.result_code = 2001;
	    resp->data.cp_raa.presence.result_code = PRESENT;
    } else {
	    resp->data.cp_raa.presence.experimental_result = PRESENT;
	    resp->data.cp_raa.experimental_result.presence.vendor_id = PRESENT;
	    resp->data.cp_raa.experimental_result.vendor_id = 10415;
	    resp->data.cp_raa.experimental_result.presence.experimental_result_code = PRESENT;
	    resp->data.cp_raa.experimental_result.experimental_result_code = 5001;
    }

	/* Cal the length of buffer needed */
	buflen = gx_raa_calc_length (&resp->data.cp_raa);

	send_buf = (char *)malloc( buflen + sizeof(resp->msg_type)+sizeof(resp->seq_num));
	memset(send_buf, 0, buflen + sizeof(resp->msg_type)+sizeof(resp->seq_num));

	/* encoding the raa header value to buffer */
	memcpy(send_buf, &resp->msg_type, sizeof(resp->msg_type));
    memcpy(send_buf+sizeof(resp->msg_type), &proc->rar_seq_num, sizeof(resp->seq_num));
    
	if ( gx_raa_pack(&(resp->data.cp_raa), (unsigned char *)(send_buf + sizeof(resp->msg_type) + sizeof(resp->seq_num)), buflen ) == 0 )
    {
		LOG_MSG(LOG_DEBUG,"RAA Packing failure ");
    }
    LOG_MSG(LOG_DEBUG, "RAA successfully sent ");

    gx_send(my_sock.gx_app_sock, send_buf, buflen + sizeof(resp->msg_type) + sizeof(resp->seq_num));
    if(proc->result  == PROC_RESULT_SUCCESS) {
        increment_stat(PROCEDURES_SPGW_RAR_PROC_SUCCESS);
    } else {
        increment_stat(PROCEDURES_SPGW_RAR_PROC_FAILURE);
    }
    proc_rar_complete(proc);
}

int
process_rar_request_handler(void *data)
{
    int ret = 0;
    msg_info_t *msg = (msg_info_t *)data;
    proc_context_t *proc_rar = (proc_context_t *)msg->proc_context;
    pdn_connection_t *pdn_cntxt = (pdn_connection_t *)proc_rar->pdn_context;

    LOG_MSG(LOG_DEBUG, " handle events on RAR procedure");
	ret = parse_gx_rar_msg(msg);
	if (ret) {
		if(ret != -1){
			gen_reauth_error_response(pdn_cntxt, ret, proc_rar->rar_seq_num);
		}
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return -1;
	}
    LOG_MSG(LOG_DEBUG,"Lets process RAR request");
    
    GxRAR *rar = &msg->rx_msg.rar;
    if(rar->presence.session_release_cause == PRESENT) {
        msg_info_t *msg1 = (msg_info_t*)calloc(1, sizeof(msg_info_t));
        msg1->event = SEND_PFCP_DEL_SESSION_REQ; 
        msg1->ue_context = proc_rar->ue_context;
        msg1->pdn_context = proc_rar->pdn_context;
        proc_context_t *proc = alloc_nw_init_detach_proc(msg1);
        if(proc != NULL) {
            LOG_MSG(LOG_DEBUG,"started rar triggered pdn delete");
            proc->parent_proc = proc_rar; 
            proc_rar->child_proc_add(proc_rar, proc);
            start_procedure(proc);
            return 0;
        }
    }
	// Now we may have  one/more of the following action,
	// 1. Create dedicated bearer to install rules
	// 1. Delete PDN if all the rules are deleted for default bearer
	// 2. Update default bearer QoS
	// 3. Update default bearer to remove/add some rules
	// 4. Update dedicated bearer to remote/add some rules
    while(1) {
        eps_bearer_t *bearer;
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_cntxt->policy.pending_pcc_rules);
        bool proc_created = false;
        LOG_MSG(LOG_DEBUG,"pcc rule from the list = %p", pcc_rule);
        while (pcc_rule != NULL) {
            LOG_MSG(LOG_DEBUG,"check rule = %s ", pcc_rule->dyn_rule->rule_name);
            pcc_rule_t *next_pcc_rule = TAILQ_NEXT(pcc_rule, next_pcc_rule);
            if(((pcc_rule->flags & PCC_RULE_OP_PENDING) == 0) && pcc_rule->action == RULE_ACTION_ADD) {
                bearer = get_bearer(pdn_cntxt, &pcc_rule->dyn_rule->qos);
                if(bearer == NULL) {
                    // New dedicated Bearer should be created to accomodate-new rule with QCI/ARP 
                    // also mark those rules as pending ??
                    LOG_MSG(LOG_DEBUG,"Bearer not found. Create dedicated Bearer  = %s ", pcc_rule->dyn_rule->rule_name);
                    msg_info_t *msg = (msg_info_t*)calloc(1, sizeof(msg_info_t));
                    msg->event = BEARER_CREATE_EVNT; 
                    msg->ue_context = proc_rar->ue_context;
                    msg->pdn_context = proc_rar->pdn_context;
                    proc_context_t *proc = alloc_bearer_create_proc(msg);
                    proc->parent_proc = proc_rar; 
                    proc_rar->child_proc_add(proc_rar, proc);
                    proc_created = true;
                    start_procedure(proc);
                    break;
                }
            }
            pcc_rule = next_pcc_rule;
        }
        if(proc_created == false) {
            break;
        }
    }

#if 0
    // Update Bearer 
    while(1) {
        eps_bearer_t *bearer;
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_cntxt->policy.pending_pcc_rules);
        bool proc_created = false;
        while (pcc_rule != NULL) {
            pcc_rule_t *next_pcc_rule = TAILQ_NEXT(pcc_rule, next_pcc_rule);
            if((pcc_rule->flags & PCC_RULE_OP_PENDING == 0) && (pcc_rule->action == RULE_ACTION_ADD)) {
                bearer = get_bearer(pdn_cntxt, &pcc_rule->dyn_rule->qos);
                if(bearer != NULL) {
                    // Update existing bearer 
                    // also mark those rules as pending ??
                    proc_context_t *proc = alloc_update_bearer_proc(msg, bearer);
                    proc_created = true;
                    start_procedure(proc);
                    break;
                }
            }
            pcc_rule = next_pcc_rule;
        }
        if(proc_created == false) {
            break;
        }
    }

    // update the bearer due to rule modification 
    while(1) {
        eps_bearer_t *bearer;
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_cntxt->policy.pending_pcc_rules);
        bool proc_created = false;
        while (pcc_rule != NULL) {
            pcc_rule_t *next_pcc_rule = TAILQ_NEXT(pcc_rule, next_pcc_rule);
            if(((pcc_rule->flags & PCC_RULE_OP_PENDING) == 0) && (pcc_rule->action == RULE_ACTION_MODIFY)) {
			    rule_name_key_t rule_name = {0};
			    memcpy(&rule_name.rule_name, &(pcc_rule->dyn_rule->rule_name),
			    	   sizeof(pcc_rule->dyn_rule->rule_name));
			    sprintf(rule_name.rule_name, "%s%d",
			    		rule_name.rule_name, pdn_cntxt->call_id);

	            int8_t bearer_id;
			    bearer_id = get_rule_name_entry(rule_name.rule_name);
			    if (bearer_id != -1) {
                    bearer = pdn_cntxt->eps_bearers[bearer_id - 5];
                    if(bearer != NULL) {
                        // Update existing bearer 
                        // also mark those rules as pending ??
                        proc_context_t *proc = alloc_update_bearer_proc(msg, bearer);
                        proc_created = true;
                        start_procedure(proc);
                        break;
                    }
                }
            }
            pcc_rule = next_pcc_rule;
        }
        if(proc_created == false) {
            break;
        }
    }
#endif

    // delete the bearer due to rule deletion...
    // Its expected that all the bearer for which update operation is seen, delete rules
    // are already handled. Now we are left with rules for the bearers which have not 
    // seen update operation  
    LOG_MSG(LOG_DEBUG,"Check for rule delete");
    while(1) {
        eps_bearer_t *bearer;
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn_cntxt->policy.pending_pcc_rules);
        bool proc_created = false;
        while (pcc_rule != NULL) {
            pcc_rule_t *next_pcc_rule = TAILQ_NEXT(pcc_rule, next_pcc_rule);
            LOG_MSG(LOG_DEBUG,"Check for rule delete %s ", pcc_rule->dyn_rule->rule_name);
            if(((pcc_rule->flags & PCC_RULE_OP_PENDING) == 0) && (pcc_rule->action == RULE_ACTION_DELETE)) {
			    rule_name_key_t rule_name = {0};
			    memcpy(&rule_name.rule_name, &(pcc_rule->dyn_rule->rule_name),
			    	   sizeof(pcc_rule->dyn_rule->rule_name));
			    sprintf(rule_name.rule_name, "%s%d",
			    		rule_name.rule_name, pdn_cntxt->call_id);

	            int8_t bearer_id;
			    bearer_id = get_rule_name_entry(rule_name.rule_name);
                LOG_MSG(LOG_DEBUG,"rule delete %s bearer_id = %d ", rule_name.rule_name, bearer_id);
			    if (bearer_id == -1) {
			    	/* TODO: Error handling bearer not found */
                    pcc_rule = next_pcc_rule;
                    continue;
			    }
                bearer = pdn_cntxt->eps_bearers[bearer_id];
                if(bearer != NULL) {
                    msg_info_t *msg1 = (msg_info_t*)calloc(1, sizeof(msg_info_t));
                    msg1->event = SEND_PFCP_DEL_SESSION_REQ; 
                    msg1->ue_context = proc_rar->ue_context;
                    msg1->pdn_context = proc_rar->pdn_context;
                    proc_context_t *proc = alloc_nw_init_detach_proc(msg1);
                    if(proc != NULL) {
                      proc_created = true;
                      proc->parent_proc = proc_rar; 
                      proc_rar->child_proc_add(proc_rar, proc);
                      start_procedure(proc);
                      break;
                    }
                }
            }
            pcc_rule = next_pcc_rule;
        }
        if(proc_created == true) {
            break;
        }
    }

#if 0
	// Update default bearer QoS
    if(pdn_cntxt->policy.default_bearer_qos_valid == true) {
        // check if default bearer qos is changed. If yes
        // create procedure and link that procedure with RAR proc
        // default bearer QoS : QCI, MBR Rate --- Bearer QoS
        // APN AMBR           : APN AMBR UL/DL : part of QoS information AVP
        // return
    }

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

