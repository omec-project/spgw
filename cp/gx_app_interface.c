// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <stdlib.h>
#include "gx_app_interface.h"
#include "ipc_api.h"
#include "sm_arr.h"
#include "pfcp.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "sm_pcnd.h"
#include "ue.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "gx_event_handlers.h"

static uint32_t cc_request_number = 0;
int g_cp_sock ;
int gx_app_sock  ;
int ret ;

/*
This function Handles the RAR msg received from PCEF
*/
void
handle_gx_rar(unsigned char * recv_buf )
{
	GxRAR *gx_rar = (GxRAR*)malloc(sizeof(*gx_rar));
	memset((void*)gx_rar, 0, sizeof(*gx_rar));

	gx_rar_unpack((unsigned char *)recv_buf, gx_rar );

	clLog(gxlogger, eCLSeverityDebug,"Gx RAR : \n  Session Id [%s]  \n re_auth_request_type[%d]\n",
			gx_rar->session_id.val,
			gx_rar->re_auth_request_type );
}

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

			len = (pdn->apn_in_use)->apn_name_label[i];
			if((pdn->apn_in_use)->apn_name_label[i] != '\0'){
				strncat(apn,(const char *) &((pdn->apn_in_use)->apn_name_label[i + 1]), len);
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

void
process_create_bearer_resp_and_send_raa( int sock )
{
	char *send_buf =  NULL;
	uint32_t buflen ;

	gx_msg *resp = malloc(sizeof(gx_msg));
	memset(resp, 0, sizeof(gx_msg));

	/* Filling Header value of RAA */
	resp->msg_type = GX_RAA_MSG ;
	//create_bearer_resp_t cbresp = {0};

	//fill_raa_msg( &(resp->data.cp_raa), &cbresp );

	/* Cal the length of buffer needed */
	buflen = gx_raa_calc_length (&resp->data.cp_raa);

	send_buf = malloc( buflen );
	memset(send_buf, 0, buflen);

	/* encoding the raa header value to buffer */
	memcpy( send_buf, &resp->msg_type, sizeof(resp->msg_type));

	if ( gx_raa_pack(&(resp->data.cp_raa), (unsigned char *)(send_buf + sizeof(resp->msg_type)), buflen ) == 0 )
		clLog(gxlogger, eCLSeverityDebug,"RAA Packing failure on sock [%d] \n", sock);

	//send_to_ipc_channel( sock, send_buf );
}

int
msg_handler_gx( void )
{
	int bytes_rx = 0;
	msg_info msg = {0};
	gx_msg *gxmsg = NULL;
	char recv_buf[BUFFSIZE] = {0};

	bytes_rx = recv_from_ipc_channel(gx_app_sock, recv_buf);
	if(bytes_rx <= 0 ){
			close_ipc_channel(gx_app_sock);
			/* Greacefull Exit */
			exit(0);
	}

	gxmsg = (gx_msg *)recv_buf;

	if ((ret = gx_pcnd_check(gxmsg, &msg)) != 0)
		return -1;

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
    msg.rx_interface = PGW_GX;
    process_gx_message(gxmsg, &msg);
	return 0;
}

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
	gx_app_sock  = accept_from_ipc_channel( g_cp_sock, gx_app_sockaddr);
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
