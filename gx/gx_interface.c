// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdlib.h>
#include "rte_errno.h"
#include "gx_interface.h"
#include "ipc_api.h"
#include "pfcp.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "ue.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "gen_utils.h"
#include "tables/tables.h"
#include "cp_config.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "cp_events.h"

static uint32_t cc_request_number = 0;
int g_cp_sock ;
int ret ;

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
		subs_id->list = malloc(sizeof( GxSubscriptionId));
		if(subs_id->list == NULL){
			clLog(clSystemLog, eCLSeverityCritical,"[%s]:[%s]:[%d] Memory allocation fails\n",
					__file__, __func__, __LINE__);
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

		subs_id->list = malloc(sizeof( GxSubscriptionId));
		if(subs_id->list == NULL){
			clLog(clSystemLog, eCLSeverityCritical,"[%s]:[%s]:[%d] Memory allocation fails\n",
					__file__, __func__, __LINE__);
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
	uint16_t len = 0;
	char apn[MAX_APN_LEN] = {0};

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
				clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %s \n", __func__,
						strerror(errno));
				return -1;
		}
	}

	/* VS: TODO */
	/* TODO: Need to Discuss with Varun and Himanshu for make following AVP's are generic or
	 * to be based on MSG TYPE OR condition basis */

	/* VS: Fill the APN Vaule */
	if ((pdn->apn_in_use)->apn_name_length != 0) {
		ccr->presence.called_station_id = PRESENT;

		for(int i=0; i < MAX_APN_LEN; ){

			len = (pdn->apn_in_use)->apn_name[i];
			if((pdn->apn_in_use)->apn_name[i] != '\0'){
				strncat(apn,(const char *) &((pdn->apn_in_use)->apn_name[i + 1]), len);
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
    printf("Starting gx message handler thread \n");
    RTE_SET_USED(data);

    /* Make a connection between control-plane and gx_app */
    if(cp_config->cp_type != SGWC) {
        printf("Opening up gx-app socket \n");
        start_cp_app();
    }

    while(1) {
        if(my_sock.gx_app_sock > 0) {
            break;
        }
        printf("\n error - why app started but gx_app_sock is < 0 ??\n");
        sleep(1);
    }

    printf("Gx app connected \n");
    while(1) {
        int bytes_rx = 0;
        msg_info_t *msg = calloc(1, sizeof(msg_info_t)); 
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
        msg->msg_type = gx_rx->msg_type;
        msg->source_interface = GX_IFACE;
        msg->raw_buf = calloc(1, bytes_rx);
        memcpy(msg->raw_buf, recv_buf, bytes_rx);
#if 1
        if(GX_RAR_MSG == msg->msg_type) {
            printf("Ignoring RAR coming from PCRF\n");
            free(msg);
            continue;
        }
#endif
        queue_stack_unwind_event(GX_MSG_RECEIVED, (void *)msg, process_gx_msg);
    }
    printf("exiting gx message handler thread \n");
    return NULL;
}

void
process_gx_msg(void *data, uint16_t event)
{
    assert(event == GX_MSG_RECEIVED );
    msg_info_t *msg = (msg_info_t *)data;
    gx_msg *gx_rx = (gx_msg *)msg->raw_buf;
    msg->rar_seq_num = gx_rx->seq_num;
    printf("Sequence number = %d \n", gx_rx->seq_num);
    switch(msg->msg_type) {
        case GX_CCA_MSG: {
            printf("\n Received CCA \n");
            if (gx_cca_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + sizeof(gx_rx->seq_num),
                        &msg->gx_msg.cca) <= 0) {
                printf("\n unpack failed \n");
                return;
            }
            printf("\n Received CCA session id  %s \n", msg->gx_msg.cca.session_id.val);
            if(msg->gx_msg.cca.cc_request_type == INITIAL_REQUEST) {
                printf("\n Received CCA-initial \n");
                handle_cca_initial_msg(&msg);
            } else if (msg->gx_msg.cca.cc_request_type == UPDATE_REQUEST) {
                printf("\n Received CCA-update\n");
                handle_cca_update_msg(&msg); 
            } else if (msg->gx_msg.cca.cc_request_type == TERMINATION_REQUEST) {
                printf("\n Received CCA-terminate \n");
                handle_ccr_terminate_msg(&msg);
            } else {
                printf("\n Received unknown CCA...treating initial..worst \n");
                handle_cca_initial_msg(&msg);
            }
            break;
        }
        case GX_RAR_MSG: {
            printf("\n Received RAR with sequence number %d \n", gx_rx->seq_num);
            if (gx_rar_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + sizeof(gx_rx->seq_num),
                        &msg->gx_msg.rar) <= 0) {
                return;
            }
            handle_rar_msg(&msg);
            break;
        }
        default: {
            clLog(clSystemLog, eCLSeverityCritical, "%s::process_msgs-"
                    "\n\tcase: SAEGWC::spgw_cfg= %d;"
                    "\n\tReceived Gx Message : "
                    "%d not supported... Discarding\n", __func__,
                    cp_config->cp_type, gx_rx->msg_type);
            free(msg);
            return;
        }
    }
    free(msg->raw_buf);
    // nobody took ownership...
    if(msg->refCnt == 0)
        free(msg);
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
	int ret = 0;
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
	gx_context_t *gx_context = NULL;

	ret = get_gx_context((uint8_t *)pdn->gx_sess_id, &gx_context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
				pdn->gx_sess_id);
	return -1;
	}
	/* VS: Set the Msg header type for CCR */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* VS: Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = UPDATE_REQUEST ;

	/* VG: Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = TERMINATION;

	/* VS:TODO: Need to check the bearer identifier value */
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
		ccr_request.data.ccr.subscription_id.list  = rte_malloc_socket(NULL,
				(sizeof(GxSubscriptionId)*1),
				RTE_CACHE_LINE_SIZE, rte_socket_id());
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
	ccr_request.data.ccr.charging_rule_report.list = rte_malloc_socket(NULL,
			(sizeof(GxChargingRuleReportList)*1),
			RTE_CACHE_LINE_SIZE, rte_socket_id());

	ccr_request.data.ccr.charging_rule_report.list[idx].presence.charging_rule_name = PRESENT;
	ccr_request.data.ccr.charging_rule_report.list[idx].charging_rule_name.list = rte_malloc_socket(NULL,
			(sizeof(GxChargingRuleNameOctetString)*1),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
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
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
		return -1;
	}

    struct sockaddr_in saddr_in;
    saddr_in.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_U, saddr_in.sin_addr.s_addr);


	/* Update UE State */
	pdn->state = CCRU_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCRU_SNT_STATE;
	//gx_context->proc = pdn->proc;
	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_ccr_calc_length(&ccr_request.data.ccr);
	buffer = rte_zmalloc_socket(NULL, msg_len + sizeof(ccr_request.msg_type),
	    RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (buffer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return -1;
	}

	/* VS: Fill the CCR header values */
	memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

	if (gx_ccr_pack(&(ccr_request.data.ccr),
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num)), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
	return 0;
}
