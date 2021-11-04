// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "cp_interface.h"
#include "cp_log.h"
#include "spgw_config_struct.h"
#include "util.h"
#include "pfcp_cp_session.h"
#include "gen_utils.h"
#include "gtpv2_session.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "proc.h"


static void start_procedure_direct(proc_context_t *proc_ctxt);

/* We should queue the event if required */
void
start_procedure(proc_context_t *new_proc_ctxt)
{
    ue_context_t *context = NULL;
    context = (ue_context_t *)new_proc_ctxt->ue_context;

    if(context != NULL) {
        TAILQ_INSERT_TAIL(&context->pending_sub_procs, new_proc_ctxt, next_sub_proc); 
        assert(!TAILQ_EMPTY(&context->pending_sub_procs)); // not empty
        /* Decide if we wish to run procedure now... If yes move on ...*/
        proc_context_t *p_ctxt = TAILQ_FIRST(&context->pending_sub_procs);
        if(p_ctxt != new_proc_ctxt) {
            bool allowed_parellel = false;
            // allow child proc to run if current outstanding proc is parent proc
            proc_context_t *temp;
            TAILQ_FOREACH(temp, &context->pending_sub_procs, next_sub_proc) {
                if(temp == new_proc_ctxt)
                    continue;
                LOG_MSG(LOG_DEBUG4, "outstanding procedures in queue %s ",temp->proc_name);
                if(temp->proc_type == RAR_PROC) {
                    LOG_MSG(LOG_DEBUG4, "current procedure in queue RAR");
                    if((new_proc_ctxt->proc_type == DED_BER_ACTIVATION_PROC) && (new_proc_ctxt->parent_proc == temp)) {
                        LOG_MSG(LOG_DEBUG,"CBREQ, RAR is allowed in parellel");
                        allowed_parellel = true;
                    }
                    if((new_proc_ctxt->proc_type == NW_INIT_DETACH_PROC) && (new_proc_ctxt->parent_proc == temp)) {
                        LOG_MSG(LOG_DEBUG,"DBREQ, RAR is allowed in parellel");
                        allowed_parellel = true;
                    }
                } else {
                    LOG_MSG(LOG_DEBUG4,"Other proecudure is running %s ", temp->proc_name);
                    // even if one running proc does not want new proc to run in parellel then we need to wait
                    allowed_parellel = false; 
                }
            }
            if(allowed_parellel == false) {
                LOG_MSG(LOG_DEBUG4, "Parellel procedure not allowed ");
                return;
            }
        }
    }
    start_procedure_direct(new_proc_ctxt);
    return;
}

static void 
start_procedure_direct(proc_context_t *proc_ctxt)
{
    msg_info_t *msg = (msg_info_t *)proc_ctxt->msg_info;
    assert(proc_ctxt != NULL);

    LOG_MSG(LOG_DEBUG4, "Start direct procedure  %s ",proc_ctxt->proc_name);

    proc_ctxt->flags |= PROC_FLAGS_RUNNING;

    switch(proc_ctxt->proc_type) {
        case INITIAL_PDN_ATTACH_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        } 

        case S1_RELEASE_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_IDLE_PENDING;
            pdn_connection_t *pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_IDLE_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case SERVICE_REQUEST_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case DETACH_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_DETACH_PENDING;
            pdn_connection_t *pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_DETACH_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case NW_INIT_DETACH_PROC: {
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_DETACH_PENDING;
            pdn_connection_t *pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_DETACH_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }

        case USAGE_REPORT_PROC: {
            // no change in procedures
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }
        case PAGING_PROC: { 
            /* Change UE/PDN state if needed and call procedure event */
            ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_PAGING_PENDING;
            pdn_connection_t *pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_PAGING_PENDING;
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }
        case DED_BER_ACTIVATION_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }
        case RAR_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            break;
        }
        default:
            assert("unknown proc");
    }
    return;
}

void
end_procedure(proc_context_t *proc_ctxt) 
{
    uint64_t imsi64=0;
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

    LOG_MSG(LOG_DEBUG4, "end procedure  %s ",proc_ctxt->proc_name);

    assert(proc_ctxt != NULL);

    if(proc_ctxt->gtpc_trans != NULL) {
        cleanup_gtpc_trans(proc_ctxt->gtpc_trans);
    }

    if(proc_ctxt->pfcp_trans != NULL) {
        cleanup_pfcp_trans(proc_ctxt->pfcp_trans);
    }

    msg_info_t *msg = (msg_info_t *)proc_ctxt->msg_info;
    if(msg != NULL) {
        msg->refCnt--;
        if(msg->refCnt == 1) { // i m the only one using this 
            free(msg->raw_buf);
            free(msg); 
        }
    }

    context = (ue_context_t *)proc_ctxt->ue_context;
    if(context != NULL) {
        imsi64 = context->imsi64;
        TAILQ_REMOVE(&context->pending_sub_procs, proc_ctxt, next_sub_proc);
    }

    switch(proc_ctxt->proc_type) {
        case INITIAL_PDN_ATTACH_PROC: {
            context = (ue_context_t *)proc_ctxt->ue_context;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            if(context != NULL && proc_ctxt->result == PROC_RESULT_SUCCESS) {
                context->state = UE_STATE_ACTIVE;
                assert(pdn != NULL);
                pdn->state = PDN_STATE_ACTIVE;
            } else if (pdn != NULL && context != NULL){
                cleanup_pdn_context(pdn);
                if(context->num_pdns == 0) {
                    cleanup_ue_context(&context);
                }
            }
            if(proc_ctxt->req_pco != NULL) {
                free(proc_ctxt->req_pco);
            }
            if(proc_ctxt->sub_config != NULL) {
                free(proc_ctxt->sub_config);
            }
            free(proc_ctxt);
            proc_ctxt = NULL;
            break;
        }
        case S1_RELEASE_PROC: {
            context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_IDLE;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_IDLE;
            free(proc_ctxt);
            break;
        }
        case SERVICE_REQUEST_PROC: {
            // No state change 
            free(proc_ctxt);
            context = (ue_context_t *)proc_ctxt->ue_context;
            break;
        }
        case DETACH_PROC: {
            context = (ue_context_t *)proc_ctxt->ue_context;
            assert(context  != NULL);
            context->state = UE_STATE_DETACH;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            assert(pdn != NULL);
            pdn->state = PDN_STATE_DETACH;
            free(proc_ctxt);
            cleanup_pdn_context(pdn);
            if(context->num_pdns == 0) {
                cleanup_ue_context(&context);
            }
            break;
        }
        case USAGE_REPORT_PROC:
            //no special action as of now..follow paging proc
        case PAGING_PROC : {
            context = (ue_context_t *)proc_ctxt->ue_context;
            context->state = UE_STATE_ACTIVE;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            pdn->state = PDN_STATE_ACTIVE;
            free(proc_ctxt);
            break;
        }
        case DED_BER_ACTIVATION_PROC: {
            // no change in pdn/context state
            context = (ue_context_t *)proc_ctxt->ue_context;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            if(proc_ctxt->parent_proc != NULL) {
                proc_context_t *p_proc = (proc_context_t *)proc_ctxt->parent_proc;
                p_proc->child_proc_done(proc_ctxt);
            } 
            free(proc_ctxt);
            break;
        }
        case RAR_PROC: {
            // no change in pdn/context state
            context = (ue_context_t *)proc_ctxt->ue_context;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            free(proc_ctxt);
            break;
        }
        case NW_INIT_DETACH_PROC: {
            context = (ue_context_t *)proc_ctxt->ue_context;
            pdn = (pdn_connection_t *)proc_ctxt->pdn_context;
            if(proc_ctxt->parent_proc != NULL) {
                proc_context_t *p_proc = (proc_context_t *)proc_ctxt->parent_proc;
                p_proc->child_proc_done(proc_ctxt);
            } 
            free(proc_ctxt);
            cleanup_pdn_context(pdn);
            if(context->num_pdns == 0) {
                cleanup_ue_context(&context);
            }
            break;
        }
        default:
            assert(0);
    }

    if(context != NULL) {
        // start new procedure if something is pending 
        LOG_MSG(LOG_DEBUG3, "Continue with subscriber %lu ", imsi64);
        proc_context_t *temp;
        TAILQ_FOREACH(temp, &context->pending_sub_procs, next_sub_proc) {
            if(temp->flags & PROC_FLAGS_RUNNING) {
                LOG_MSG(LOG_DEBUG3, "Running procedure %s ", temp->proc_name);
                continue;
            }
            // TODO: can we run this procedure ???
            start_procedure_direct(temp);
        }
    } else {
        LOG_MSG(LOG_DEBUG3, "Released subscriber %lu ", imsi64);
    }
    // result_success - schedule next proc
    // result_fail_call_del - abort all outstanding procs and delete the call
    // result_fail_keep_call - continue with call as is and schedule new proc if any  
    return;
}

/**
 * @brief  : It return procedure name from enum
 * @param  : value, procedure
 * @return : Returns procedure string
 */
// TODO : performance ... 
const char * get_proc_string(int value)
{
	switch(value) {
		case NONE_PROC:
			return "NONE_PROC";
		case INITIAL_PDN_ATTACH_PROC:
			return "INITIAL_PDN_ATTACH_PROC";
		case SERVICE_REQUEST_PROC:
			return "SERVICE_REQUEST_PROC";
		case SGW_RELOCATION_PROC:
			return "SGW_RELOCATION_PROC";
		case CONN_SUSPEND_PROC:
			return "CONN_SUSPEND_PROC";
		case DETACH_PROC:
			return "DETACH_PROC";
		case DED_BER_ACTIVATION_PROC:
			return "DED_BER_ACTIVATION_PROC";
		case PDN_GW_INIT_BEARER_DEACTIVATION:
			return "PDN_GW_INIT_BEARER_DEACTIVATION";
		case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC:
			return "MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC";
		case UPDATE_BEARER_PROC:
			return "UPDATE_BEARER_PROC";
		case RESTORATION_RECOVERY_PROC:
			return "RESTORATION_RECOVERY_PROC";
		case END_PROC:
			return "END_PROC";
        case RAR_PROC:
            return "RAR_PROC";
        case PAGING_PROC:
            return "PAGING_PROC";
        case USAGE_REPORT_PROC:
            return "USAGE_REPORT_PROC";
        case S1_RELEASE_PROC:
            return "S1_RELEASE_PROC";
        case PFCP_ASSOC_SETUP_PROC:
            return "PFCP_ASSOC_SETUP_PROC";
        case SGW_RELOCATION_DETACH_PROC:
            return "SGW_RELOCATION_DETACH_PROC";
        case NW_INIT_DETACH_PROC:
            return "NW_INIT_DETACH_PROC";
		default:
			return "UNDEFINED_PROC";
	}
	return "UNDEFINED_PROC";
}

/**
 * @brief  : It return state name from enum
 * @param  : value, state
 * @return : Returns state string
 */
const char * get_state_string(int value)
{
    switch(value) {
        case SGWC_NONE_STATE:
            return "SGWC_NONE_STATE";
        case PFCP_ASSOC_REQ_SNT_STATE:
            return "PFCP_ASSOC_REQ_SNT_STATE";
        case PFCP_ASSOC_RESP_RCVD_STATE:
            return "PFCP_ASSOC_RESP_RCVD_STATE";
        case PFCP_SESS_EST_REQ_SNT_STATE:
            return "PFCP_SESS_EST_REQ_SNT_STATE";
        case PFCP_SESS_EST_RESP_RCVD_STATE:
            return "PFCP_SESS_EST_RESP_RCVD_STATE";
        case CONNECTED_STATE:
            return "CONNECTED_STATE";
        case IDEL_STATE:
            return "IDEL_STATE";
        case CS_REQ_SNT_STATE:
            return "CS_REQ_SNT_STATE";
        case CS_RESP_RCVD_STATE:
            return "CS_RESP_RCVD_STATE";
        case PFCP_SESS_MOD_REQ_SNT_STATE:
            return "PFCP_SESS_MOD_REQ_SNT_STATE";
        case PFCP_SESS_MOD_RESP_RCVD_STATE:
            return "PFCP_SESS_MOD_RESP_RCVD_STATE";
        case PFCP_SESS_DEL_REQ_SNT_STATE:
            return "PFCP_SESS_DEL_REQ_SNT_STATE";
        case PFCP_SESS_DEL_RESP_RCVD_STATE:
            return "PFCP_SESS_DEL_RESP_RCVD_STATE";
        case DS_REQ_SNT_STATE:
            return "DS_REQ_SNT_STATE";
        case DS_RESP_RCVD_STATE:
            return "DS_RESP_RCVD_STATE";
        case DDN_REQ_SNT_STATE:
            return "DDN_REQ_SNT_STATE";
        case DDN_ACK_RCVD_STATE:
            return "DDN_ACK_RCVD_STATE";
        case MBR_REQ_SNT_STATE:
            return "MBR_REQ_SNT_STATE";
        case MBR_RESP_RCVD_STATE:
            return "MBR_RESP_RCVD_STATE";
        case CREATE_BER_REQ_SNT_STATE:
            return "CREATE_BER_REQ_SNT_STATE";
        case RE_AUTH_ANS_SNT_STATE:
            return "RE_AUTH_ANS_SNT_STATE";
        case PGWC_NONE_STATE:
            return "PGWC_NONE_STATE";
        case CCR_SNT_STATE:
            return "CCR_SNT_STATE";
        case CREATE_BER_RESP_SNT_STATE:
            return "CREATE_BER_RESP_SNT_STATE";
        case PFCP_PFD_MGMT_RESP_RCVD_STATE:
            return "PFCP_PFD_MGMT_RESP_RCVD_STATE";
        case ERROR_OCCURED_STATE:
            return "ERROR_OCCURED_STATE";
        case UPDATE_BEARER_REQ_SNT_STATE:
            return "UPDATE_BEARER_REQ_SNT_STATE";
        case UPDATE_BEARER_RESP_SNT_STATE:
            return "UPDATE_BEARER_RESP_SNT_STATE";
        case DELETE_BER_REQ_SNT_STATE:
            return "DELETE_BER_REQ_SNT_STATE";
        case CCRU_SNT_STATE:
            return "CCRU_SNT_STATE";
        case PGW_RSTRT_NOTIF_REQ_SNT_STATE:
            return "PGW_RSTRT_NOTIF_REQ_SNT_STATE";
        case UPD_PDN_CONN_SET_REQ_SNT_STATE:
            return "UPD_PDN_CONN_SET_REQ_SNT_STATE";
        case DEL_PDN_CONN_SET_REQ_SNT_STATE:
            return "DEL_PDN_CONN_SET_REQ_SNT_STATE";
        case DEL_PDN_CONN_SET_REQ_RCVD_STATE:
            return "DEL_PDN_CONN_SET_REQ_RCVD_STATE";
        case PFCP_SESS_SET_DEL_REQ_SNT_STATE:
            return "PFCP_SESS_SET_DEL_REQ_SNT_STATE";
        case PFCP_SESS_SET_DEL_REQ_RCVD_STATE:
            return "PFCP_SESS_SET_DEL_REQ_RCVD_STATE";
        case UPF_SETUP_FAILED:
            return "UPF_SETUP_FAILED";
        case PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE:
            return "PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE";
        case END_STATE:
            return "END_STATE";
        default:
            return "UNDEFINED_STATE";
    }
    return "UNDEFINED_STATE";
}

/**
 * @brief  : It return event name from enum
 * @param  : value, state
 * @return : Returns event string
 */
const char * get_event_string(int value)
{
    switch(value) {
        case NONE_EVNT:
            return "NONE_EVNT";
        case CS_REQ_RCVD_EVNT:
            return "CS_REQ_RCVD_EVNT";
        case PFCP_ASSOC_SETUP_SNT_EVNT:
            return "PFCP_ASSOC_SETUP_SNT_EVNT";
        case PFCP_ASSOC_SETUP_RESP_RCVD_EVNT:
            return "PFCP_ASSOC_SETUP_RESP_RCVD_EVNT";
        case PFCP_SESS_EST_REQ_RCVD_EVNT:
            return "PFCP_SESS_EST_REQ_RCVD_EVNT";
        case PFCP_SESS_EST_RESP_RCVD_EVNT:
            return "PFCP_SESS_EST_RESP_RCVD_EVNT";
        case CS_RESP_RCVD_EVNT:
            return "CS_RESP_RCVD_EVNT";
        case MB_REQ_RCVD_EVNT:
            return"MB_REQ_RCVD_EVNT";
        case PFCP_SESS_MOD_REQ_RCVD_EVNT:
            return "PFCP_SESS_MOD_REQ_RCVD_EVNT";
        case PFCP_SESS_MOD_RESP_RCVD_EVNT:
            return "PFCP_SESS_MOD_RESP_RCVD_EVNT";
        case MB_RESP_RCVD_EVNT:
            return"MB_RESP_RCVD_EVNT";
        case REL_ACC_BER_REQ_RCVD_EVNT:
            return "REL_ACC_BER_REQ_RCVD_EVNT";
        case DS_REQ_RCVD_EVNT:
            return "DS_REQ_RCVD_EVNT";
        case PFCP_SESS_DEL_REQ_RCVD_EVNT:
            return "PFCP_SESS_DEL_REQ_RCVD_EVNT";
        case PFCP_SESS_DEL_RESP_RCVD_EVNT:
            return "PFCP_SESS_DEL_RESP_RCVD_EVNT";
        case DS_RESP_RCVD_EVNT:
            return "DS_RESP_RCVD_EVNT";
        case ECHO_REQ_RCVD_EVNT:
            return "DDN_ACK_RCVD_EVNT";
        case ECHO_RESP_RCVD_EVNT:
            return "ECHO_RESP_RCVD_EVNT";
        case DDN_ACK_RESP_RCVD_EVNT:
            return "DDN_ACK_RESP_RCVD_EVNT";
        case PFCP_SESS_RPT_REQ_RCVD_EVNT:
            return "PFCP_SESS_RPT_REQ_RCVD_EVNT";
        case RE_AUTH_REQ_RCVD_EVNT:
            return "RE_AUTH_REQ_RCVD_EVNT";
        case CREATE_BER_RESP_RCVD_EVNT:
            return "CREATE_BER_RESP_RCVD_EVNT";
        case CCA_RCVD_EVNT:
            return "CCA_RCVD_EVNT";
        case CREATE_BER_REQ_RCVD_EVNT:
            return "CREATE_BER_REQ_RCVD_EVNT";
        case PFCP_PFD_MGMT_RESP_RCVD_EVNT:
            return "PFCP_PFD_MGMT_RESP_RCVD_EVNT";
        case ERROR_OCCURED_EVNT:
            return "ERROR_OCCURED_EVNT";
        case UPDATE_BEARER_REQ_RCVD_EVNT:
            return "UPDATE_BEARER_REQ_RCVD_EVNT";
        case UPDATE_BEARER_RSP_RCVD_EVNT:
            return "UPDATE_BEARER_RSP_RCVD_EVNT";
        case DELETE_BER_REQ_RCVD_EVNT:
            return "DELETE_BER_REQ_RCVD_EVNT";
        case DELETE_BER_RESP_RCVD_EVNT:
            return "DELETE_BER_RESP_RCVD_EVNT";
        case DELETE_BER_CMD_RCVD_EVNT:
            return "DELETE_BER_CMD_RCVD_EVNT";
        case CCAU_RCVD_EVNT:
            return "CCAU_RCVD_EVNT";
        case PFCP_SESS_SET_DEL_REQ_RCVD_EVNT:
            return "PFCP_SESS_SET_DEL_REQ_RCVD_EVNT";
        case PFCP_SESS_SET_DEL_RESP_RCVD_EVNT:
            return "PFCP_SESS_SET_DEL_RSEP_RCVD_EVNT";
        case PGW_RSTRT_NOTIF_ACK_RCVD_EVNT:
            return "PGW_RSTRT_NOTIF_ACK_RCVD_EVNT";
        case UPD_PDN_CONN_SET_REQ_RCVD_EVNT:
            return "UPD_PDN_CONN_SET_REQ_RCVD_EVNT";
        case UPD_PDN_CONN_SET_RESP_RCVD_EVNT:
            return "UPD_PDN_CONN_SET_RESP_RCVD_EVNT";
        case DEL_PDN_CONN_SET_REQ_RCVD_EVNT:
            return "DEL_PDN_CONN_SET_REQ_RCVD_EVNT";
        case DEL_PDN_CONN_SET_RESP_RCVD_EVNT:
            return "DEL_PDN_CONN_SET_RESP_RCVD_EVNT";
        case SEND_PFCP_DEL_SESSION_REQ: 
            return "SEND_PFCP_DEL_SESSION_REQ";
        case SEND_GTP_DEL_BEARER_REQ:
            return "SEND_GTP_DEL_BEARER_REQ";
        case PFCP_ASSOCIATION_SETUP:
            return "PFCP_ASSOCIATION_SETUP";
        case PFCP_ASSOCIATION_SETUP_RSP:
            return "PFCP_ASSOCIATION_SETUP_RSP";
        case PFCP_ASSOC_SETUP_SUCCESS:
            return "PFCP_ASSOC_SETUP_SUCCESS";
        case PFCP_ASSOC_SETUP_FAILED:
            return "PFCP_ASSOC_SETUP_FAILED";
        case PFCP_SESS_EST_EVNT:
            return "PFCP_SESS_EST_EVNT";
        case PFCP_SESS_EST_RESP_TIMEOUT_EVNT:
            return "PFCP_SESS_EST_RESP_TIMEOUT_EVNT";
        case GX_CCAI_FAILED:
            return "GX_CCAI_FAILED";
        case DDN_TIMEOUT:
            return "DDN_TIMEOUT";
        case BEARER_CREATE_EVNT:
            return "BEARER_CREATE_EVNT";
        case SEND_RE_AUTH_RSP_EVNT:
            return "SEND_RE_AUTH_RSP_EVNT";
        case DEFAULT_BEARER_DELETE:
            return "DEFAULT_BEARER_DELETE";
        case NW_DETACH_DBREQ_TIMEOUT:
            return "NW_DETACH_DBREQ_TIMEOUT";
        case RCVD_GTP_DEL_BEARER_RSP:
            return "RCVD_GTP_DEL_BEARER_RSP";
        case END_EVNT:
            return "END_EVNT";
        default:
            return "UNDEFINED_EVNT";
    }
    return "UNDEFINED_EVNT";
}

proc_context_t*
get_first_procedure(void *context)
{
    ue_context_t *ue_context = (ue_context_t*)context;

    if(ue_context != NULL) {
        proc_context_t *proc_ctxt = TAILQ_FIRST(&ue_context->pending_sub_procs);
        return proc_ctxt;
    }
    return NULL;
}
