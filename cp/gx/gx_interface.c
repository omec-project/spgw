// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdlib.h>
#include "gx_interface.h"
#include "ipc_api.h"
#include "pfcp.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "ue.h"
#include "gen_utils.h"
#include "spgw_config_struct.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "cp_events.h"
#include "cp_test.h"
#include "cp_log.h"

#define TIMESTAMP_LEN 14
/* VS: Need to decide the base value of call id */
/* const uint32_t call_id_base_value = 0xFFFFFFFF; */
const uint32_t call_id_base_value = 0x00000000;
static uint32_t call_id_offset;

static uint32_t cc_request_number = 0;
int g_cp_sock ;

void
fill_rat_type_ie( int32_t *ccr_rat_type, uint8_t csr_rat_type )
{
	if ( csr_rat_type == EUTRAN_ ) {
		*ccr_rat_type = GX_EUTRAN;
	}else if ( csr_rat_type == UTRAN ){
		*ccr_rat_type = GX_UTRAN;
	}else if ( csr_rat_type == GERAN ){
		*ccr_rat_type = GX_GERAN;
	}else if ( csr_rat_type == WLAN ){
		*ccr_rat_type = GX_WLAN;
	}else if ( csr_rat_type == VIRTUAL ){
		*ccr_rat_type = GX_VIRTUAL;
	}else if ( csr_rat_type == GAN ){
		*ccr_rat_type = GX_GAN;
	}else if ( csr_rat_type == HSPA_EVOLUTION ){
		*ccr_rat_type = GX_HSPA_EVOLUTION;
	}
}

/**
 * @brief  : Fill qos information
 * @param  : ccr_qos_info, structure to be filled
 * @param  : bearer, bearer information
 * @param  : apn_ambr, ambr details
 * @return : Returns nothing
 */
static void
fill_qos_info( GxQosInformation *ccr_qos_info,
		eps_bearer_t *bearer, ambr_ie *apn_ambr)
{

	/* VS:TODO: Need to check the bearer identifier value */
	ccr_qos_info->presence.bearer_identifier = PRESENT ;
	ccr_qos_info->bearer_identifier.len =
		int_to_str((char *)ccr_qos_info->bearer_identifier.val,
				bearer->eps_bearer_id);

	ccr_qos_info->presence.apn_aggregate_max_bitrate_ul = PRESENT;
	ccr_qos_info->presence.apn_aggregate_max_bitrate_dl = PRESENT;
	ccr_qos_info->apn_aggregate_max_bitrate_ul =
		apn_ambr->ambr_uplink;
	ccr_qos_info->apn_aggregate_max_bitrate_dl =
		apn_ambr->ambr_downlink;

	ccr_qos_info->presence.max_requested_bandwidth_ul = PRESENT;
	ccr_qos_info->presence.max_requested_bandwidth_dl = PRESENT;
	ccr_qos_info->max_requested_bandwidth_ul =
		bearer->qos.ul_mbr;
	ccr_qos_info->max_requested_bandwidth_dl =
		bearer->qos.dl_mbr;

	ccr_qos_info->presence.guaranteed_bitrate_ul = PRESENT;
	ccr_qos_info->presence.guaranteed_bitrate_dl = PRESENT;
	ccr_qos_info->guaranteed_bitrate_ul =
		bearer->qos.ul_gbr;
	ccr_qos_info->guaranteed_bitrate_dl =
		bearer->qos.dl_gbr;
}

/**
 * @brief  : fill default eps bearer qos
 * @param  : ccr_default_eps_bearer_qos, structure to be filled
 * @param  : bearer, bearer data
 * @return : Returns Nothing
 */
static void
fill_default_eps_bearer_qos( GxDefaultEpsBearerQos *ccr_default_eps_bearer_qos,
		eps_bearer_t *bearer)
{
	if(( QCI_1 <= (bearer->qos.qci)  && (bearer->qos.qci) <= QCI_9 ) ||
			QCI_65 == (bearer->qos.qci) ||
			QCI_66 == (bearer->qos.qci) ||
			QCI_69 == (bearer->qos.qci) ||
			QCI_70 == (bearer->qos.qci))
	{
		ccr_default_eps_bearer_qos->presence.qos_class_identifier = PRESENT;
		ccr_default_eps_bearer_qos->qos_class_identifier = bearer->qos.qci;
	} else {
		/* TODO :Revisit to handler other values of Qci e.g 0 */
	}

	ccr_default_eps_bearer_qos->presence.allocation_retention_priority = PRESENT;

	ccr_default_eps_bearer_qos->allocation_retention_priority.presence.priority_level = PRESENT;
	ccr_default_eps_bearer_qos->allocation_retention_priority.priority_level =
		bearer->qos.arp.priority_level;

	ccr_default_eps_bearer_qos->allocation_retention_priority.presence.pre_emption_capability = PRESENT;
	ccr_default_eps_bearer_qos->allocation_retention_priority.pre_emption_capability =
		bearer->qos.arp.preemption_capability;
	ccr_default_eps_bearer_qos->allocation_retention_priority.presence.pre_emption_vulnerability = PRESENT;
	ccr_default_eps_bearer_qos->allocation_retention_priority.pre_emption_vulnerability =
		bearer->qos.arp.preemption_vulnerability;
}

/**
 * @brief  : convert binary value to string
 *           Binary value is stored in 8 bytes, each nibble representing each char.
 *           char binary stroes each char in 1 byte.
 * @param  : [in] b_val : Binary val
 * @param  : [out] s_val : Converted string val
 * @return : void
 */
void
bin_to_str(unsigned char *b_val, char *s_val, int b_len, int s_len)
{
	if(NULL == b_val || NULL == s_val) return;

	memset(s_val, 0, s_len);

	/* Byte 'AB' in b_val, is converted to two bytes 'A', 'B' in s_val*/
	s_val[0] = '0' + (b_val[0] & 0x0F);
	s_val[1] = '0' + ((b_val[0]>>4) & 0x0F);

	for(int i=1; i < b_len; ++i) {
		s_val[(i*2)] = '0' + (b_val[i] & 0x0F);
		s_val[(i*2) + 1] = '0' + ((b_val[i]>>4) & 0x0F);
	}
	s_val[(b_len*2)-1] = '\0';
}


void
fill_subscription_id( GxSubscriptionIdList *subs_id, uint64_t imsi, uint64_t msisdn )
{
	subs_id->count = 0;

	if( imsi != 0 ) {
		subs_id->list = (GxSubscriptionId *)malloc(sizeof( GxSubscriptionId));
		if(subs_id->list == NULL){
			LOG_MSG(LOG_ERROR,"Memory allocation fails");
		}

		subs_id->list[subs_id->count].presence.subscription_id_type = PRESENT;
		subs_id->list[subs_id->count].presence.subscription_id_data = PRESENT;
		subs_id->list[subs_id->count].subscription_id_type = END_USER_IMSI;
		subs_id->list[subs_id->count].subscription_id_data.len = STR_IMSI_LEN -1 ;
		bin_to_str((unsigned char*) (&imsi),
				(char *)(subs_id->list[subs_id->count].subscription_id_data.val),
				BINARY_IMSI_LEN, STR_IMSI_LEN);

		subs_id->count++;
	} else 	if( msisdn != 0 ) {

		subs_id->list = (GxSubscriptionId *)malloc(sizeof( GxSubscriptionId));
		if(subs_id->list == NULL){
			LOG_MSG(LOG_ERROR,"Memory allocation fails");
		}

		subs_id->list[subs_id->count].presence.subscription_id_type = PRESENT;
		subs_id->list[subs_id->count].presence.subscription_id_data = PRESENT;
		subs_id->list[subs_id->count].subscription_id_type = END_USER_E164;
		subs_id->list[subs_id->count].subscription_id_data.len = STR_MSISDN_LEN;
		bin_to_str((unsigned char*) (&msisdn),
				(char *)(subs_id->list[subs_id->count].subscription_id_data.val),
				BINARY_MSISDN_LEN, STR_MSISDN_LEN);
		subs_id->count++;
	}
}

void
fill_user_equipment_info( GxUserEquipmentInfo *ccr_user_eq_info, uint64_t csr_imei )
{
	ccr_user_eq_info->presence.user_equipment_info_type = PRESENT;
	ccr_user_eq_info->presence.user_equipment_info_value = PRESENT;
	ccr_user_eq_info->user_equipment_info_type = IMEISV ;
	ccr_user_eq_info->user_equipment_info_value.len = sizeof(uint64_t);
	memcpy( ccr_user_eq_info->user_equipment_info_value.val,  &(csr_imei),
			ccr_user_eq_info->user_equipment_info_value.len);
}
void
fill_3gpp_ue_timezone( Gx3gppMsTimezoneOctetString *ccr_tgpp_ms_timezone,
		gtp_ue_time_zone_ie_t csr_ue_timezone )
{
	ccr_tgpp_ms_timezone->len = csr_ue_timezone.header.len;
	memcpy( ccr_tgpp_ms_timezone->val, &(csr_ue_timezone.time_zone), ccr_tgpp_ms_timezone->len);
}

/* VS: Fill the Credit Crontrol Request to send PCRF */
int
fill_ccr_request(GxCCR *ccr, ue_context_t *context,
		uint8_t ebi_index, char *sess_id)
{
	eps_bearer_t *bearer = NULL;
	pdn_connection_t *pdn = NULL;

	bearer = context->eps_bearers[ebi_index];
	pdn = context->eps_bearers[ebi_index]->pdn;

	/* VS: Assign the Session ID in the request */
	if (sess_id != NULL) {
		ccr->presence.session_id = PRESENT;
		ccr->session_id.len = strlen(sess_id);
		memcpy(ccr->session_id.val, sess_id, ccr->session_id.len);
	}

	/* RFC 4006 section 8.2 */
	/* ============================================== */
	/*  Cc Request Type      |    Cc Request number   */
	/* ============================================== */
	/*   Initial Request     --     0                 */
	/*   Event   Request     --     0                 */
	/*   Update  Request_1   --     1                 */
	/*   Update  Request_2   --     2                 */
	/*   Update  Request_n   --     n                 */
	/*   Termination Request --     n + 1             */

	/* VS: Handle the Multiple Msg type request */
	if (ccr->presence.cc_request_type == PRESENT) {
		switch(ccr->cc_request_type) {
			case INITIAL_REQUEST: {
				ccr->presence.cc_request_number = PRESENT;
				/* Make this number generic */
				ccr->cc_request_number = 0 ;

				/* VS: TODO: Need to Check condition handling */
				ccr->presence.ip_can_type = PRESENT;
				ccr->ip_can_type = TGPP_GPRS;

				break;
			}
			case UPDATE_REQUEST:
				ccr->presence.cc_request_number = PRESENT;
				ccr->cc_request_number = ++cc_request_number ;
				break;

			case TERMINATION_REQUEST:
				ccr->presence.cc_request_number = PRESENT;
				/* Make this number generic */
				ccr->cc_request_number =  ++cc_request_number ;

				/* VS: TODO: Need to Check condition handling */
				ccr->presence.ip_can_type = PRESENT;
				ccr->ip_can_type = TGPP_GPRS;
				break;

			default:
				LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
				return -1;
		}
	}

	/* VS: TODO */
	/* TODO: Need to Discuss with Varun and Himanshu for make following AVP's are generic or
	 * to be based on MSG TYPE OR condition basis */
	uint16_t len = 0;
	char apn[MAX_APN_LEN] = {0};
	/* VS: Fill the APN Vaule */
	if (pdn->apn_len != 0) {
		ccr->presence.called_station_id = PRESENT;

		for(int i=0; i < MAX_APN_LEN; ){

			len = pdn->apn[i];
			if(pdn->apn[i] != '\0'){
				strncat(apn,(const char *) &(pdn->apn[i + 1]), len);
				apn[len] = '.';
				i += len+1;
			} else {
				apn[i-1] = '\0';
				break;
			}
		}

		ccr->called_station_id.len = strlen(apn);
		memcpy(ccr->called_station_id.val, apn, ccr->called_station_id.len);

	 }

	/* VS: Fill the RAT type in CCR */
	if( context->rat_type.len != 0 ){
		ccr->presence.rat_type = PRESENT;
		fill_rat_type_ie(&ccr->rat_type, context->rat_type.rat_type);
	}

	/* VS: Set the bearer eps qos values received in CSR */
	ccr->presence.default_eps_bearer_qos = PRESENT;
	fill_default_eps_bearer_qos( &(ccr->default_eps_bearer_qos),
			bearer);

	/* VS: Set the bearer apn ambr and Uplink/Downlink MBR/GBR values received in CSR */
	ccr->presence.qos_information = PRESENT;
	fill_qos_info(&(ccr->qos_information), bearer, &pdn->apn_ambr);


	/* Need to Handle IMSI and MSISDN */
	if( context->imsi != 0 || context->msisdn != 0 )
	{
		ccr->presence.subscription_id = PRESENT;
		fill_subscription_id( &ccr->subscription_id, context->imsi, context->msisdn );
	}

	///* VS: TODO Need to check later on */
	if(context->mei != 0)
	{
		ccr->presence.user_equipment_info = PRESENT;
		fill_user_equipment_info( &(ccr->user_equipment_info), context->mei );
	}

	return 0;
}

void*
msg_handler_gx(void *data)
{
    LOG_MSG(LOG_INIT,"Starting gx message handler thread ");

    /* Make a connection between control-plane and gx_app */
    if(cp_config->cp_type != SGWC) {
        LOG_MSG(LOG_INIT, "Opening up gx-app socket ");
        start_cp_app();
    }

    while(1) {
        if(my_sock.gx_app_sock > 0) {
            break;
        }
        LOG_MSG(LOG_ERROR,"App started but gx_app_sock is < 0 ? Retry ");
        sleep(1);
    }

    LOG_MSG(LOG_INIT, "Gx app connected ");
    while(1) {
        int bytes_rx = 0;
        gx_msg *gx_rx = NULL;
        char recv_buf[BUFFSIZE] = {0};

        bytes_rx = recv_from_ipc_channel(my_sock.gx_app_sock, recv_buf);
        if(bytes_rx <= 0 ){
            close_ipc_channel(my_sock.gx_app_sock);
            /* Greacefull Exit */
            exit(0);
            return NULL;
        }

        gx_rx = (gx_msg *)recv_buf;
        struct sockaddr_in saddr_in;
        saddr_in.sin_family = AF_INET;
        inet_aton("127.0.0.1", &(saddr_in.sin_addr));
        msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t)); 
        msg->magic_head = MSG_MAGIC;
        msg->magic_tail = MSG_MAGIC;
        msg->msg_type = gx_rx->msg_type;
        msg->source_interface = GX_IFACE;
        msg->raw_buf = calloc(1, bytes_rx);
        memcpy(msg->raw_buf, recv_buf, bytes_rx);
		if(gx_in_mock_handler[msg->msg_type] != NULL) {
			gx_in_mock_handler[msg->msg_type](msg, GX_MSG_RECEIVED);
		} else {
        	queue_stack_unwind_event(GX_MSG_RECEIVED, (void *)msg, process_gx_msg);
		}
    }
    LOG_MSG(LOG_ERROR,"exiting gx message handler thread data = %p ", data);
    return NULL;
}

void
process_gx_msg(void *data, uint16_t event)
{
    assert(event == GX_MSG_RECEIVED );
    msg_info_t *msg = (msg_info_t *)data;
    gx_msg *gx_rx = (gx_msg *)msg->raw_buf;
    msg->rar_seq_num = gx_rx->seq_num;
    switch(msg->msg_type) {
        case GX_CCA_MSG: {
    		LOG_MSG(LOG_DEBUG,"Received CCA. Sequence number = %d ", gx_rx->seq_num);
            if (gx_cca_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + sizeof(gx_rx->seq_num),
                        &msg->rx_msg.cca) <= 0) {
                free(msg->raw_buf);
                free(msg);
                LOG_MSG(LOG_ERROR, "Received CCA. Sequence number = %d. Unpacked failed ", gx_rx->seq_num);
                return;
            }
            LOG_MSG(LOG_DEBUG,"Received CCA session id  %s ", msg->rx_msg.cca.session_id.val);
            if(msg->rx_msg.cca.cc_request_type == INITIAL_REQUEST) {
                LOG_MSG(LOG_DEBUG,"Received CCA-initial ");
                handle_cca_initial_msg(&msg);
            } else if (msg->rx_msg.cca.cc_request_type == UPDATE_REQUEST) {
                LOG_MSG(LOG_DEBUG,"Received CCA-update");
                handle_cca_update_msg(&msg); 
            } else if (msg->rx_msg.cca.cc_request_type == TERMINATION_REQUEST) {
                LOG_MSG(LOG_DEBUG,"Received CCA-terminate ");
                handle_ccr_terminate_msg(&msg);
            } else {
                LOG_MSG(LOG_ERROR,"Received unknown CCA...treating initial..worst ");
                handle_cca_initial_msg(&msg);
            }
            break;
        }
        case GX_RAR_MSG: {
            LOG_MSG(LOG_DEBUG,"Received RAR with sequence number %d ", gx_rx->seq_num);
            if (gx_rar_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + sizeof(gx_rx->seq_num),
                        &msg->rx_msg.rar) <= 0) {
                free(msg->raw_buf);
                free(msg);
                return;
            }
            handle_rar_msg(&msg);
            break;
        }
        default: {
            LOG_MSG(LOG_ERROR, "process_msgs-case: SAEGWC::spgw_cfg= %d;"
                    " Received Gx Message : "
                    "%d not supported... Discarding",
                    cp_config->cp_type, gx_rx->msg_type);
            free(msg->raw_buf);
            free(msg);
            return;
        }
    }

    if(msg != NULL) {
        assert(msg->magic_head == MSG_MAGIC);
        assert(msg->magic_tail == MSG_MAGIC);
        if(msg->refCnt == 0) { // no one claimed ownership of this msg 
            free(msg->raw_buf);
            free(msg);
        }
    }
    return;
}

#if 0
    // saegw - INITIAL_PDN_ATTACH_PROC, CCR_SNT_STATE, CCA_RCVD_EVNT - cca_msg_handler 
    // saegw - DETACH_PROC CCR_SNT_STATE CCA_RCVD_EVNT - cca_t_msg_handler
    // saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CCRU_SNT_STATE CCA_RCVD_EVNT => del_bearer_cmd_ccau_handler

    // saegw - DED_BER_ACTIVATION_PROC CONNECTED_STATE RE_AUTH_REQ_RCVD_EVNT => process_rar_request_handler
    // saegw - DED_BER_ACTIVATION_PROC IDEL_STATE RE_AUTH_REQ_RCVD_EVNT => process_rar_request_handler 
    // saegw - PDN_GW_INIT_BEARER_DEACTIVATION CONNECTED_STATE RE_AUTH_REQ_RCVD_EVNT => process_rar_request_handler
    // saegw - PDN_GW_INIT_BEARER_DEACTIVATION IDEL_STATE RE_AUTH_REQ_RCVD_EVNT => process_rar_request_handler 

    // pgw - INITIAL_PDN_ATTACH_PROC CCR_SNT_STATE CCA_RCVD_EVNT => cca_msg_handler
    // pgw - SGW_RELOCATION_PROC CCRU_SNT_STATE CCA_RCVD_EVNT => cca_u_msg_handler_handover
    // pgw - DETACH_PROC CCR_SNT_STATE CCA_RCVD_EVNT => cca_t_msg_handler
    // pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC CCRU_SNT_STATE CCA_RCVD_EVNT del_bearer_cmd_ccau_handler

    // pgw - DED_BER_ACTIVATION_PROC CONNECTED_STATE RE_AUTH_REQ_RCVD_EVNT process_rar_request_handler
    // pgw - DED_BER_ACTIVATION_PROC IDEL_STATE RE_AUTH_REQ_RCVD_EVNT process_rar_request_handler 
    // pgw - PDN_GW_INIT_BEARER_DEACTIVATION CONNECTED_STATE RE_AUTH_REQ_RCVD_EVNT ==> process_rar_request_handler
    // PGW - PDN_GW_INIT_BEARER_DEACTIVATION IDEL_STATE RE_AUTH_REQ_RCVD_EVNT ==> process_rar_request_handler
    process_gx_message(gxmsg, &msg);
#endif

void
start_cp_app(void )
{
	struct sockaddr_un cp_app_sockaddr = {0};
	struct sockaddr_un gx_app_sockaddr = {0};

	/* Socket Creation */
	g_cp_sock = create_ipc_channel();

	/* Bind the socket*/
	bind_ipc_channel(g_cp_sock, cp_app_sockaddr, SERVER_PATH);

	/* Mark the socket fd for listen */
	listen_ipc_channel(g_cp_sock);

	/* Accept incomming connection request receive on socket */
	my_sock.gx_app_sock  = accept_from_ipc_channel( g_cp_sock, gx_app_sockaddr);
}

const char *
gx_type_str(uint8_t type)
{
	/* GX Message Type Values */
	switch (type) {
	case GX_CCR_MSG:
		return "GX_CCR_MSG";
	case GX_CCA_MSG:
		return "GX_CCA_MSG";
	case GX_RAR_MSG:
		return "GX_RAR_MSG";
	case GX_RAA_MSG:
		return "GX_RAA_MSG";
	default:
		return "UNKNOWN";
	}
}



/**
 * @brief  : Generate CCR request
 * @param  : context , pointer to ue context structure
 * @param  : ebi_index, index in array where eps bearer is stored
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
ccru_req_for_bear_termination(pdn_connection_t *pdn, eps_bearer_t *bearer)
{

	/*
	 * TODO:
	 * Passing bearer as parameter is a BAD IDEA
	 * because what if multiple bearer changes?
	 * code SHOULD anchor only on pdn.
	 */
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};

	ue_context_t *ue_context = (ue_context_t *)get_ue_context_from_gxsessid((uint8_t *)pdn->gx_sess_id);
	if (ue_context == NULL) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
	    return -1;
	}
	/* Set the Msg header type for CCR */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = UPDATE_REQUEST ;

	/* Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = TERMINATION;

	/* TODO: Need to check the bearer identifier value */
	ccr_request.data.ccr.presence.bearer_identifier = PRESENT ;
	ccr_request.data.ccr.bearer_identifier.len =
		int_to_str((char *)ccr_request.data.ccr.bearer_identifier.val,
				bearer->eps_bearer_id -5);

	/* Subscription-Id */
	if(pdn->context->imsi  || pdn->context->msisdn)
	{
		uint8_t idx = 0;
		ccr_request.data.ccr.presence.subscription_id = PRESENT;
		ccr_request.data.ccr.subscription_id.count = 1; // IMSI & MSISDN
		ccr_request.data.ccr.subscription_id.list  = (GxSubscriptionId*)calloc(1, sizeof(GxSubscriptionId)*1);
		/* Fill IMSI */
		if(pdn->context->imsi != 0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_IMSI;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len = pdn->context->imsi_len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&pdn->context->imsi,
					pdn->context->imsi_len);
			idx++;
		}

		/* Fill MSISDN
		if(pdn->context->msisdn !=0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_E164;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len =  pdn->context->msisdn_len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&pdn->context->msisdn,
					pdn->context->msisdn_len);
		} */
	}

	ccr_request.data.ccr.presence.network_request_support = PRESENT;
	ccr_request.data.ccr.network_request_support = NETWORK_REQUEST_SUPPORTED;

	/* ccr_request.data.ccr.presence.framed_ip_address = PRESENT;
	ccr_request.data.ccr.framed_ip_address.len = inet_ntoa(ccr_request.data.ccr.framed_ip_address.val);
	                                              bearer->eps_bearer_id -5);*/
	int idx = 0;
	ccr_request.data.ccr.presence.charging_rule_report = PRESENT;
	ccr_request.data.ccr.charging_rule_report.count = 1;
	ccr_request.data.ccr.charging_rule_report.list = (GxChargingRuleReport *)calloc(1, sizeof(GxChargingRuleReport)*1);

	ccr_request.data.ccr.charging_rule_report.list[idx].presence.charging_rule_name = PRESENT;
	ccr_request.data.ccr.charging_rule_report.list[idx].charging_rule_name.list = (GxChargingRuleNameOctetString *)calloc(1, sizeof(GxChargingRuleNameOctetString)*1);
	ccr_request.data.ccr.charging_rule_report.list[idx].charging_rule_name.count = 1;
	ccr_request.data.ccr.charging_rule_report.list[idx].charging_rule_name.list[idx].len = strlen(bearer->dynamic_rules[idx]->rule_name);

	for(uint16_t i = 0 ; i<strlen(bearer->dynamic_rules[idx]->rule_name); i++){
		ccr_request.data.ccr.charging_rule_report.list[idx].charging_rule_name.list[idx].val[i] =
			bearer->dynamic_rules[idx]->rule_name[i];
	}
//	ccr_request.data.ccr.charging_rule_report.list[idx].presence.bearer_identifier = PRESENT;
//	ccr_request.data.ccr.charging_rule_report.list[idx].bearer_identifier.val[idx] =
//		int_to_str((char *)ccr_request.data.ccr.bearer_identifier.val,
//				bearer->eps_bearer_id - 5);

	ccr_request.data.ccr.charging_rule_report.list[idx].presence.pcc_rule_status = PRESENT;
	ccr_request.data.ccr.charging_rule_report.list[idx].pcc_rule_status = INACTIVE;

	ccr_request.data.ccr.charging_rule_report.list[idx].presence.rule_failure_code = PRESENT;
	ccr_request.data.ccr.charging_rule_report.list[idx].rule_failure_code = NO_BEARER_BOUND;

	//ccr_request.data.ccr.charging_rule_report.list[idx].presence.ran_nas_release_cause = PRESENT;
	//ccr_request.data.ccr.charging_rule_report.list[idx].ran_nas_release_cause =;

	char *temp = inet_ntoa(pdn->ipv4);
	memcpy(ccr_request.data.ccr.framed_ip_address.val, &temp, strlen(temp));

	/*
	 * nEED TO ADd following to Complete CCR_I, these are all mandatory IEs
	 * AN-GW Addr (SGW)
	 * User Eqip info (IMEI)
	 * 3GPP-ULI
	 * calling station id (APN)
	 * Access n/w charging addr (PGW addr)
	 * Charging Id
	 */


	/* VS: Fill the Credit Crontrol Request to send PCRF */
	if(fill_ccr_request(&ccr_request.data.ccr, pdn->context, bearer->eps_bearer_id - 5, pdn->gx_sess_id) != 0) {
		LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
		return -1;
	}

    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_U, saddr_in.sin_addr.s_addr);


	/* Update UE State */
	pdn->state = CCRU_SNT_STATE;

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_ccr_calc_length(&ccr_request.data.ccr);
	buffer = (char *)calloc(1, msg_len + sizeof(ccr_request.msg_type));
	if (buffer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
		return -1;
	}

	/* VS: Fill the CCR header values */
	memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

	if (gx_ccr_pack(&(ccr_request.data.ccr),
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msg_len) == 0) {
		LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	gx_send(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
	return 0;
}

/**
 * @brief  : Generate CCR session id with the combination of timestamp and call id
 * @param  : str_buf is used to store generated session id
 * @param  : timestamp is used to pass timestamp
 * @param  : value is used to pas call id
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
gen_sess_id_string(char *str_buf, char *timestamp , uint32_t value)
{
	char buf[MAX_SESS_ID_LEN] = {0};
	int len = 0;

	if (timestamp == NULL)
	{
		LOG_MSG(LOG_ERROR, "Time stamp is NULL ");
		return -1;
	}

	/* itoa(value, buf, 10);  10 Means base value, i.e. indicate decimal value */
	len = int_to_str(buf, value);

	if(buf[0] == 0)
	{
		LOG_MSG(LOG_ERROR, "Failed coversion of integer to string, len:%d ", len);
		return -1;
	}

	sprintf(str_buf, "%s%s", timestamp, buf);
	return 0;
}

/**
 * @brief  : Get the system current timestamp.
 * @param  : timestamp is used for storing system current timestamp
 * @return : Returns 0 in case of success
 */
static uint8_t
get_timestamp(char *timestamp)
{

	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);

	strftime(timestamp, MAX_SESS_ID_LEN, "%Y%m%d%H%M%S", tmp);
	return 0;
}

/**
 * Return the CCR session id.
 */
int8_t
gen_sess_id_for_ccr(char *sess_id, uint32_t call_id)
{
	char timestamp[MAX_SESS_ID_LEN] = {0};

	get_timestamp(timestamp);

	if((gen_sess_id_string(sess_id, timestamp, call_id)) < 0)
	{
		LOG_MSG(LOG_ERROR, "Failed to generate session id for CCR");
		return -1;
	}
	return 0;
}

/**
 * Retrieve the call id from the CCR session id.
 */
int
retrieve_call_id(char *str, uint32_t *call_id)
{
	uint8_t idx = 0, index = 0;
	char buf[MAX_SESS_ID_LEN] = {0};

	if(str == NULL)
	{
		LOG_MSG(LOG_ERROR, "String is NULL");
		return -1;
	}

	idx = TIMESTAMP_LEN; /* TIMESTAMP STANDARD BYTE SIZE */
	for(;str[idx] != '\0'; ++idx)
	{
		buf[index] = str[idx];
		++index;
	}

	*call_id = atoi(buf);
	if (*call_id == 0) {
		LOG_MSG(LOG_ERROR, "Call ID not found");
		return -1;
	}
	return 0;
}

/**
 * Generate the CALL ID
 */
uint32_t
generate_call_id(void)
{
	uint32_t call_id = 0;
	call_id = call_id_base_value + (++call_id_offset);

	return call_id;
}

uint32_t
gx_send(int fd, char *buf, uint16_t len)
{
    static uint32_t seq_num;
    seq_num++;
    if (fd != 0) {
       queue_gx_out_event(fd, (uint8_t *)buf, len);
    }
	return seq_num;
}

/* PERFORAMANCE : Should use conditional variable ?*/
void*
out_handler_gx(void *data)
{
	LOG_MSG(LOG_INIT,"Starting gx out message handler thread");
    while(1) {
        outgoing_pkts_event_t *event = (outgoing_pkts_event_t*)get_gx_out_event();
        if(event != NULL) {
            //Push packet to test chain 
            gx_msg *temp = (gx_msg*)event->payload;
            if(gx_out_mock_handler[temp->msg_type] != NULL) {
                gx_out_mock_handler[temp->msg_type](event);
                free(event->payload);
                free(event);
                continue;
            }

            send_to_ipc_channel(event->fd, (char *)event->payload, event->payload_len);
            free(event->payload);
            free(event);
        }
        //PERFORAMANCE ISSUE - use conditional variable 
        usleep(100); // every pkt 0.1 ms default scheduling delay
    }
	LOG_MSG(LOG_ERROR,"gx out message handler thread exited , data %p ", data);
    return NULL;
}

void init_gx(void)
{
    // thread to read incoming socket messages from udp socket
    pthread_t readerGx_t;
    pthread_attr_t gxattr;
    pthread_attr_init(&gxattr);
    pthread_attr_setdetachstate(&gxattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&readerGx_t, &gxattr, &msg_handler_gx, NULL);
    pthread_attr_destroy(&gxattr);

    // thread to write outgoing gx messages
    pthread_t writerGx_t;
    pthread_attr_t gxattr1;
    pthread_attr_init(&gxattr1);
    pthread_attr_setdetachstate(&gxattr1, PTHREAD_CREATE_DETACHED);
    pthread_create(&writerGx_t, &gxattr1, &out_handler_gx, NULL);
    pthread_attr_destroy(&gxattr1);
}
