// Copyright (c) 2019 Sprint
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <rte_hash_crc.h>
#include <rte_errno.h>

#include "ue.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "gen_utils.h"
#include "cp_log.h"
#include "sm_structs_api.h"
#include "tables/tables.h"



char proc_name [64];
char state_name[64];
char event_name[64];



uint8_t
update_ue_state(uint32_t teid_key, uint8_t state,  uint8_t ebi_index)
{
	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	ret = get_ue_context(teid_key, &context);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:Failed to update UE State for Teid:%x...\n", __func__,
				teid_key);
		return -1;
	}
	pdn = GET_PDN(context , ebi_index);
	pdn->state = state;

	LOG_MSG(LOG_DEBUG, "%s: Change UE State for Teid:%u, State:%s\n",
			__func__, teid_key, get_state_string(pdn->state));
	return 0;

}

uint8_t
get_ue_state(uint32_t teid_key, uint8_t ebi_index)
{
	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	ret = get_ue_context(teid_key, &context);

	if ( ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:Entry not found for teid:%x...\n", __func__, teid_key);
		return -1;
	}
	pdn = GET_PDN(context , ebi_index);
	LOG_MSG(LOG_DEBUG, "%s: Teid:%u, State:%s\n",
			__func__, teid_key, get_state_string(pdn->state));
	return pdn->state;
}

int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context_t **context)
{
	int ret = 0;
	eps_bearer_t *bearer = NULL;

	ret = get_bearer_by_teid(teid_key, &bearer);
	if(ret < 0) {
		LOG_MSG(LOG_ERROR, "%s:%d Entry not found for teid:%x...\n", __func__, __LINE__, teid_key);
                return -1;
	}
	if(bearer != NULL && bearer->pdn != NULL && bearer->pdn->context != NULL ) {
		*context = bearer->pdn->context;
		return 0;
	}
	return -1;
}
/* This function use only in clean up while error */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context_t **context)
{
	int ret = 0;
	eps_bearer_t *bearer = NULL;
	/* If teid key is sgwc s11 */
	ret = get_ue_context(teid_key, context);
	if( ret < 0) {
		/* If teid key is sgwc s5s8 */
		ret = get_bearer_by_teid(teid_key, &bearer);
		if(ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Entry not found for teid:%x...\n", __func__, __LINE__, teid_key);
			return -1;
		}

     	   *context = bearer->pdn->context;
	}
	return 0;
}


/**
 * @brief  : It return procedure name from enum
 * @param  : value, procedure
 * @return : Returns procedure string
 */
const char * get_proc_string(int value)
{
	switch(value) {
		case NONE_PROC:
			strcpy(proc_name, "NONE_PROC");
			break;
		case INITIAL_PDN_ATTACH_PROC:
			strcpy(proc_name, "INITIAL_PDN_ATTACH_PROC");
			break;
		case SERVICE_REQUEST_PROC:
			strcpy(proc_name, "SERVICE_REQUEST_PROC");
			break;
		case SGW_RELOCATION_PROC:
			strcpy(proc_name, "SGW_RELOCATION_PROC");
			break;
		case CONN_SUSPEND_PROC:
			strcpy(proc_name, "CONN_SUSPEND_PROC");
			break;
		case DETACH_PROC:
			strcpy(proc_name, "DETACH_PROC");
			break;
		case DED_BER_ACTIVATION_PROC:
			strcpy(proc_name, "DED_BER_ACTIVATION_PROC");
			break;
		case PDN_GW_INIT_BEARER_DEACTIVATION:
			strcpy(proc_name, "PDN_GW_INIT_BEARER_DEACTIVATION");
			break;
		case MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC:
			strcpy(proc_name, "MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC");
			break;
		case UPDATE_BEARER_PROC:
			strcpy(proc_name, "UPDATE_BEARER_PROC");
			break;
		case RESTORATION_RECOVERY_PROC:
			strcpy(proc_name, "RESTORATION_RECOVERY_PROC");
			break;
		case END_PROC:
			strcpy(proc_name, "END_PROC");
			break;
		default:
			strcpy(proc_name, "UNDEFINED PROC");
			break;
	}
	return proc_name;
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
            strcpy(state_name, "SGWC_NONE_STATE");
            break;
        case PFCP_ASSOC_REQ_SNT_STATE:
            strcpy(state_name, "PFCP_ASSOC_REQ_SNT_STATE");
            break;
        case PFCP_ASSOC_RESP_RCVD_STATE:
            strcpy(state_name, "PFCP_ASSOC_RESP_RCVD_STATE");
            break;
        case PFCP_SESS_EST_REQ_SNT_STATE:
            strcpy(state_name, "PFCP_SESS_EST_REQ_SNT_STATE");
            break;
        case PFCP_SESS_EST_RESP_RCVD_STATE:
            strcpy(state_name, "PFCP_SESS_EST_RESP_RCVD_STATE");
            break;
        case CONNECTED_STATE:
            strcpy(state_name, "CONNECTED_STATE");
            break;
        case IDEL_STATE:
            strcpy(state_name, "IDEL_STATE");
            break;
        case CS_REQ_SNT_STATE:
            strcpy(state_name, "CS_REQ_SNT_STATE");
            break;
        case CS_RESP_RCVD_STATE:
            strcpy(state_name, "CS_RESP_RCVD_STATE");
            break;
        case PFCP_SESS_MOD_REQ_SNT_STATE:
            strcpy(state_name, "PFCP_SESS_MOD_REQ_SNT_STATE");
            break;
        case PFCP_SESS_MOD_RESP_RCVD_STATE:
            strcpy(state_name, "PFCP_SESS_MOD_RESP_RCVD_STATE");
            break;
        case PFCP_SESS_DEL_REQ_SNT_STATE:
            strcpy(state_name, "PFCP_SESS_DEL_REQ_SNT_STATE");
            break;
        case PFCP_SESS_DEL_RESP_RCVD_STATE:
            strcpy(state_name, "PFCP_SESS_DEL_RESP_RCVD_STATE");
            break;
        case DS_REQ_SNT_STATE:
            strcpy(state_name, "DS_REQ_SNT_STATE");
            break;
        case DS_RESP_RCVD_STATE:
            strcpy(state_name, "DS_RESP_RCVD_STATE");
            break;
        case DDN_REQ_SNT_STATE:
            strcpy(state_name, "DDN_REQ_SNT_STATE");
            break;
        case DDN_ACK_RCVD_STATE:
            strcpy(state_name, "DDN_ACK_RCVD_STATE");
            break;
		case MBR_REQ_SNT_STATE:
			strcpy(state_name, "MBR_REQ_SNT_STATE");
			break;
		case MBR_RESP_RCVD_STATE:
			strcpy(state_name, "MBR_RESP_RCVD_STATE");
			break;
		case CREATE_BER_REQ_SNT_STATE:
			strcpy(state_name, "CREATE_BER_REQ_SNT_STATE");
			break;
		case RE_AUTH_ANS_SNT_STATE:
			 strcpy(state_name, "RE_AUTH_ANS_SNT_STATE");
			break;
		case PGWC_NONE_STATE:
		        strcpy(state_name, "PGWC_NONE_STATE");
		        break;
		case CCR_SNT_STATE:
		        strcpy(state_name, "CCR_SNT_STATE");
		        break;
		case CREATE_BER_RESP_SNT_STATE:
		        strcpy(state_name, "CREATE_BER_RESP_SNT_STATE");
		        break;
		case PFCP_PFD_MGMT_RESP_RCVD_STATE:
		        strcpy(state_name, "PFCP_PFD_MGMT_RESP_RCVD_STATE");
		        break;
		case ERROR_OCCURED_STATE:
		        strcpy(state_name, "ERROR_OCCURED_STATE");
				break;
		case UPDATE_BEARER_REQ_SNT_STATE:
		        strcpy(state_name, "UPDATE_BEARER_REQ_SNT_STATE");
				break;
		case UPDATE_BEARER_RESP_SNT_STATE:
		        strcpy(state_name, "UPDATE_BEARER_RESP_SNT_STATE");
				break;
		case DELETE_BER_REQ_SNT_STATE:
		    strcpy(state_name, "DELETE_BER_REQ_SNT_STATE");
			break;
		case CCRU_SNT_STATE:
		    strcpy(state_name, "CCRU_SNT_STATE");
			break;
		case PGW_RSTRT_NOTIF_REQ_SNT_STATE:
		    strcpy(state_name, "PGW_RSTRT_NOTIF_REQ_SNT_STATE");
			break;
		case UPD_PDN_CONN_SET_REQ_SNT_STATE:
		    strcpy(state_name, "UPD_PDN_CONN_SET_REQ_SNT_STATE");
			break;
		case DEL_PDN_CONN_SET_REQ_SNT_STATE:
		    strcpy(state_name, "DEL_PDN_CONN_SET_REQ_SNT_STATE");
			break;
		case DEL_PDN_CONN_SET_REQ_RCVD_STATE:
		    strcpy(state_name, "DEL_PDN_CONN_SET_REQ_RCVD_STATE");
			break;
		case PFCP_SESS_SET_DEL_REQ_SNT_STATE:
		    strcpy(state_name, "PFCP_SESS_SET_DEL_REQ_SNT_STATE");
			break;
		case PFCP_SESS_SET_DEL_REQ_RCVD_STATE:
		    strcpy(state_name, "PFCP_SESS_SET_DEL_REQ_RCVD_STATE");
			break;
		case END_STATE:
		    strcpy(state_name, "END_STATE");
			break;
		default:
		    strcpy(state_name, "UNDEFINED STATE");
		    break;
    }
    return state_name;
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
            strcpy(event_name, "NONE_EVNT");
            break;
        case CS_REQ_RCVD_EVNT:
            strcpy(event_name, "CS_REQ_RCVD_EVNT");
            break;
        case PFCP_ASSOC_SETUP_SNT_EVNT:
            strcpy(event_name, "PFCP_ASSOC_SETUP_SNT_EVNT");
            break;
        case PFCP_ASSOC_SETUP_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_ASSOC_SETUP_RESP_RCVD_EVNT");
            break;
        case PFCP_SESS_EST_REQ_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_EST_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_EST_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_EST_RESP_RCVD_EVNT");
            break;
        case CS_RESP_RCVD_EVNT:
            strcpy(event_name, "CS_RESP_RCVD_EVNT");
            break;
        case MB_REQ_RCVD_EVNT:
            strcpy(event_name,"MB_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_MOD_REQ_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_MOD_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_MOD_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_MOD_RESP_RCVD_EVNT");
            break;
        case MB_RESP_RCVD_EVNT:
            strcpy(event_name,"MB_RESP_RCVD_EVNT");
            break;
        case REL_ACC_BER_REQ_RCVD_EVNT:
            strcpy(event_name, "REL_ACC_BER_REQ_RCVD_EVNT");
            break;
        case DS_REQ_RCVD_EVNT:
            strcpy(event_name, "DS_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_DEL_REQ_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_DEL_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_DEL_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_DEL_RESP_RCVD_EVNT");
            break;
        case DS_RESP_RCVD_EVNT:
            strcpy(event_name, "DS_RESP_RCVD_EVNT");
            break;
        case ECHO_REQ_RCVD_EVNT:
            strcpy(event_name, "DDN_ACK_RCVD_EVNT");
            break;
        case ECHO_RESP_RCVD_EVNT:
            strcpy(event_name, "ECHO_RESP_RCVD_EVNT");
            break;
        case DDN_ACK_RESP_RCVD_EVNT:
            strcpy(event_name, "DDN_ACK_RESP_RCVD_EVNT");
            break;
        case PFCP_SESS_RPT_REQ_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_RPT_REQ_RCVD_EVNT");
            break;
	case RE_AUTH_REQ_RCVD_EVNT:
            strcpy(event_name, "RE_AUTH_REQ_RCVD_EVNT");
			break;
	case CREATE_BER_RESP_RCVD_EVNT:
            strcpy(event_name, "CREATE_BER_RESP_RCVD_EVNT");
			break;
	case CCA_RCVD_EVNT:
            strcpy(event_name, "CCA_RCVD_EVNT");
			break;
	case CREATE_BER_REQ_RCVD_EVNT:
            strcpy(event_name, "CREATE_BER_REQ_RCVD_EVNT");
			break;
	case PFCP_PFD_MGMT_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_PFD_MGMT_RESP_RCVD_EVNT");
			break;
	case ERROR_OCCURED_EVNT:
            strcpy(event_name, "ERROR_OCCURED_EVNT");
            break;
	case UPDATE_BEARER_REQ_RCVD_EVNT:
            strcpy(event_name, "UPDATE_BEARER_REQ_RCVD_EVNT");
            break;
	case UPDATE_BEARER_RSP_RCVD_EVNT:
            strcpy(event_name, "UPDATE_BEARER_RSP_RCVD_EVNT");
            break;
	case DELETE_BER_REQ_RCVD_EVNT:
            strcpy(event_name, "DELETE_BER_REQ_RCVD_EVNT");
            break;
	case DELETE_BER_RESP_RCVD_EVNT:
            strcpy(event_name, "DELETE_BER_RESP_RCVD_EVNT");
            break;
	case DELETE_BER_CMD_RCVD_EVNT:
            strcpy(event_name, "DELETE_BER_CMD_RCVD_EVNT");
            break;
	case CCAU_RCVD_EVNT:
            strcpy(event_name, "CCAU_RCVD_EVNT");
            break;
        case PFCP_SESS_SET_DEL_REQ_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_SET_DEL_REQ_RCVD_EVNT");
            break;
        case PFCP_SESS_SET_DEL_RESP_RCVD_EVNT:
            strcpy(event_name, "PFCP_SESS_SET_DEL_RSEP_RCVD_EVNT");
            break;
		case PGW_RSTRT_NOTIF_ACK_RCVD_EVNT:
		    strcpy(event_name, "PGW_RSTRT_NOTIF_ACK_RCVD_EVNT");
		    break;
		case UPD_PDN_CONN_SET_REQ_RCVD_EVNT:
		    strcpy(event_name, "UPD_PDN_CONN_SET_REQ_RCVD_EVNT");
		    break;
		case UPD_PDN_CONN_SET_RESP_RCVD_EVNT:
		    strcpy(event_name, "UPD_PDN_CONN_SET_RESP_RCVD_EVNT");
		    break;
		case DEL_PDN_CONN_SET_REQ_RCVD_EVNT:
		    strcpy(event_name, "DEL_PDN_CONN_SET_REQ_RCVD_EVNT");
		    break;
		case DEL_PDN_CONN_SET_RESP_RCVD_EVNT:
		    strcpy(event_name, "DEL_PDN_CONN_SET_RESP_RCVD_EVNT");
		    break;
        case END_EVNT:
            strcpy(event_name, "END_EVNT");
            break;
        default:
            strcpy(event_name, "UNDEFINED EVNT");
            break;
    }
    return event_name;
}
