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
#include "sm_pcnd.h"
#include "ue.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "gx_event_handlers.h"
#include "gen_utils.h"

static uint32_t cc_request_number = 0;
extern udp_sock_t my_sock;
int g_cp_sock ;
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
	msg_info_t msg = {0};
	gx_msg *gxmsg = NULL;
	char recv_buf[BUFFSIZE] = {0};

	bytes_rx = recv_from_ipc_channel(my_sock.gx_app_sock, recv_buf);
	if(bytes_rx <= 0 ){
			close_ipc_channel(my_sock.gx_app_sock);
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
	my_sock.gx_app_sock  = accept_from_ipc_channel( g_cp_sock, gx_app_sockaddr);
	if((my_sock.gx_app_sock > my_sock.sock_fd_pfcp) &&
	   (my_sock.gx_app_sock > my_sock.sock_fd_s11) &&
	   (my_sock.gx_app_sock > my_sock.sock_fd_s5s8)) {
        my_sock.select_max_fd = my_sock.gx_app_sock + 1;
    }
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

int
gen_ccr_request(ue_context_t *context, uint8_t ebi_index, create_sess_req_t *csr)
{
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
	gx_context_t *gx_context = NULL;

	/* VS: Generate unique call id per PDN connection */
	context->pdns[ebi_index]->call_id = generate_call_id();

	/** Allocate the memory for Gx Context
	 */
	gx_context = rte_malloc_socket(NULL,
					sizeof(gx_context_t),
					RTE_CACHE_LINE_SIZE, rte_socket_id());

	/* VS: Generate unique session id for communicate over the Gx interface */
	if (gen_sess_id_for_ccr(gx_context->gx_sess_id,
				context->pdns[ebi_index]->call_id)) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error: %s \n", __func__, __LINE__,
				strerror(errno));
		return -1;
	}

	/* Maintain the gx session id in context */
	memcpy(context->pdns[ebi_index]->gx_sess_id,
			gx_context->gx_sess_id , strlen(gx_context->gx_sess_id));

	/* VS: Maintain the PDN mapping with call id */
	if (add_pdn_conn_entry(context->pdns[ebi_index]->call_id,
				context->pdns[ebi_index]) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to add pdn entry with call id\n", __func__, __LINE__);
		return -1;
	}

	/* VS: Set the Msg header type for CCR */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* VS: Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = INITIAL_REQUEST ;

	/* VG: Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = ESTABLISHMENT ;

	/* VS:TODO: Need to check the bearer identifier value */
	ccr_request.data.ccr.presence.bearer_identifier = PRESENT ;
	ccr_request.data.ccr.bearer_identifier.len =
		int_to_str((char *)ccr_request.data.ccr.bearer_identifier.val,
				(context->eps_bearers[ebi_index])->eps_bearer_id);

	/* Subscription-Id */
	if(csr->imsi.header.len  || csr->msisdn.header.len)
	{
		uint8_t idx = 0;
		ccr_request.data.ccr.presence.subscription_id = PRESENT;
		ccr_request.data.ccr.subscription_id.count = 1; // IMSI & MSISDN
		ccr_request.data.ccr.subscription_id.list  = rte_malloc_socket(NULL,
				(sizeof(GxSubscriptionId)*1),
				RTE_CACHE_LINE_SIZE, rte_socket_id());

		/* Fill IMSI */
		if(csr->imsi.header.len != 0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].presence.subscription_id_type = PRESENT;
			ccr_request.data.ccr.subscription_id.list[idx].presence.subscription_id_data = PRESENT;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_IMSI;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len = csr->imsi.header.len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&csr->imsi.imsi_number_digits,
					csr->imsi.header.len);
			idx++;
		}

#if 0
		/* Fill MSISDN */
		if(csr->msisdn.header.len !=0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_E164;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len = csr->msisdn.header.len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&csr->msisdn.msisdn_number_digits,
					csr->msisdn.header.len);
		}
#endif
	}

	ccr_request.data.ccr.presence.network_request_support = PRESENT;
	ccr_request.data.ccr.network_request_support = NETWORK_REQUEST_SUPPORTED;

	/* TODO: Removing this ie as it is not require.
	 * It's showing padding in pcap
	ccr_request.data.ccr.presence.framed_ip_address = PRESENT;

	ccr_request.data.ccr.framed_ip_address.len = inet_ntoa(ccr_request.data.ccr.framed_ip_address.val,
			                                               context->pdns[ebi_index]);

	char *temp = inet_ntoa(context->pdns[ebi_index]->ipv4);
	memcpy(ccr_request.data.ccr.framed_ip_address.val, &temp, strlen(temp)); */

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
	if(fill_ccr_request(&ccr_request.data.ccr, context, ebi_index, gx_context->gx_sess_id) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
		return -1;
	}

	struct sockaddr_in saddr_in;
    	saddr_in.sin_family = AF_INET;
    	inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    	update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_INITIAL, SENT, GX);


	/* Update UE State */
	context->pdns[ebi_index]->state = CCR_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCR_SNT_STATE;
	gx_context->proc = context->pdns[ebi_index]->proc;

	/* VS: Maintain the Gx context mapping with Gx Session id */
	if (gx_context_entry_add(gx_context->gx_sess_id, gx_context) < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Error: %s \n", __func__, __LINE__,
				strerror(errno));
		return -1;
	}

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
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type));
	return 0;
}

static int
fill_tai(uint8_t *buf, tai_field_t *tai) 
{

	int index = 0;
	buf[index++] = ((tai->tai_mcc_digit_2 << 4) | (tai->tai_mcc_digit_1)) & 0xff;
	buf[index++] = ((tai->tai_mnc_digit_3 << 4 )| (tai->tai_mcc_digit_3)) & 0xff;
	buf[index++] = ((tai->tai_mnc_digit_2 << 4 ) | (tai->tai_mnc_digit_1)) & 0xff;
	buf[index++] = ((tai->tai_tac >>8) & 0xff);
	buf[index++] =  (tai->tai_tac) &0xff;

	return sizeof(tai_field_t);
}

static int
fill_ecgi(uint8_t *buf, ecgi_field_t *ecgi) 
{

	int index = 0;
	buf[index++] = ((ecgi->ecgi_mcc_digit_2 << 4 ) | (ecgi->ecgi_mcc_digit_1)) & 0xff;
	buf[index++] = ((ecgi->ecgi_mnc_digit_3 << 4 ) | (ecgi->ecgi_mcc_digit_3)) & 0xff;
	buf[index++] = ((ecgi->ecgi_mnc_digit_2 << 4 ) | (ecgi->ecgi_mnc_digit_1)) & 0xff;
	buf[index++] = (((ecgi->ecgi_spare) | (ecgi->eci >> 24 )) & 0xff);
	buf[index++] = (((ecgi->eci >> 16 )) & 0xff);
	buf[index++] = (((ecgi->eci >> 8 )) & 0xff);
	buf[index++] = (ecgi->eci & 0xff);

	return sizeof(ecgi_field_t);
}

static int
fill_lai(uint8_t *buf, lai_field_t *lai) 
{

	int index = 0;
	buf[index++] = ((lai->lai_mcc_digit_2 << 4) | (lai->lai_mcc_digit_1)) & 0xff;
	buf[index++] = ((lai->lai_mnc_digit_3 << 4 )| (lai->lai_mcc_digit_3)) & 0xff;
	buf[index++] = ((lai->lai_mnc_digit_2 << 4 ) | (lai->lai_mnc_digit_1)) & 0xff;
	buf[index++] = ((lai->lai_lac >>8) & 0xff);
	buf[index++] =  (lai->lai_lac) &0xff;
	return sizeof(lai_field_t);
}

static int
fill_rai(uint8_t *buf, rai_field_t *rai) 
{

	int index = 0;
	buf[index++] = ((rai->ria_mcc_digit_2 << 4) | (rai->ria_mcc_digit_1)) & 0xff;
	buf[index++] = ((rai->ria_mnc_digit_3 << 4 )| (rai->ria_mcc_digit_3)) & 0xff;
	buf[index++] = ((rai->ria_mnc_digit_2 << 4 ) | (rai->ria_mnc_digit_1)) & 0xff;
	buf[index++] = ((rai->ria_lac >>8) & 0xff);
	buf[index++] =  (rai->ria_lac) &0xff;
	buf[index++] = ((rai->ria_rac >>8) & 0xff);
	buf[index++] =  (rai->ria_rac) &0xff;

	return sizeof(rai_field_t);
}

static int
fill_sai(uint8_t *buf, sai_field_t *sai) 
{

	int index = 0;
	buf[index++] = ((sai->sai_mcc_digit_2 << 4) | (sai->sai_mcc_digit_1)) & 0xff;
	buf[index++] = ((sai->sai_mnc_digit_3 << 4 )| (sai->sai_mcc_digit_3)) & 0xff;
	buf[index++] = ((sai->sai_mnc_digit_2 << 4 ) | (sai->sai_mnc_digit_1)) & 0xff;
	buf[index++] = ((sai->sai_lac >>8) & 0xff);
	buf[index++] =  (sai->sai_lac) &0xff;
	buf[index++] = ((sai->sai_sac >>8) & 0xff);
	buf[index++] =  (sai->sai_sac) &0xff;
	return sizeof(sai_field_t);
}

static int
fill_cgi(uint8_t *buf, cgi_field_t *cgi) 
{

	int index = 0;
	buf[index++] = ((cgi->cgi_mcc_digit_2 << 4) | (cgi->cgi_mcc_digit_1)) & 0xff;
	buf[index++] = ((cgi->cgi_mnc_digit_3 << 4 )| (cgi->cgi_mcc_digit_3)) & 0xff;
	buf[index++] = ((cgi->cgi_mnc_digit_2 << 4 ) | (cgi->cgi_mnc_digit_1)) & 0xff;
	buf[index++] = ((cgi->cgi_lac >>8) & 0xff);
	buf[index++] =  (cgi->cgi_lac) &0xff;
	buf[index++] = ((cgi->cgi_ci >>8) & 0xff);
	buf[index++] =  (cgi->cgi_ci) &0xff;
	return sizeof(cgi_field_t);
}


int
gen_ccru_request(pdn_connection_t *pdn, eps_bearer_t *bearer , mod_bearer_req_t *mb_req, uint8_t flag_check)
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
	gx_context_t *gx_context = NULL;

	int ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
			          (const void*)(pdn->gx_sess_id),
					           (void **)&gx_context);
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
	ccr_request.data.ccr.bearer_operation = MODIFICATION;

	/* VS:TODO: Need to check the bearer identifier value */
	ccr_request.data.ccr.presence.bearer_identifier = PRESENT ;
	ccr_request.data.ccr.bearer_identifier.len =
		int_to_str((char *)ccr_request.data.ccr.bearer_identifier.val,
				bearer->eps_bearer_id);

	/* Subscription-Id */
	if(pdn->context->imsi  || pdn->context->msisdn)
	{
		uint8_t idx = 0;
		ccr_request.data.ccr.presence.subscription_id = PRESENT;
		ccr_request.data.ccr.subscription_id.count = 2; // IMSI & MSISDN
		ccr_request.data.ccr.subscription_id.list  = rte_malloc_socket(NULL,
				(sizeof(GxSubscriptionId)*2),
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

		/* Fill MSISDN */
		if(pdn->context->msisdn !=0)
		{
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_type = END_USER_E164;
			ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.len =  pdn->context->msisdn_len;
			memcpy(ccr_request.data.ccr.subscription_id.list[idx].subscription_id_data.val,
					&pdn->context->msisdn,
					pdn->context->msisdn_len);
		}
	}

	ccr_request.data.ccr.presence.network_request_support = PRESENT;
	ccr_request.data.ccr.network_request_support = NETWORK_REQUEST_SUPPORTED;

	/*
	 * nEED TO ADd following to Complete CCR_I, these are all mandatory IEs
	 * AN-GW Addr (SGW)
	 * User Eqip info (IMEI)
	 * 3GPP-ULI
	 * calling station id (APN)
	 * Access n/w charging addr (PGW addr)
	 * Charging Id
	 */

	int index = 0;
	int len = 0;


	if(pdn->context->old_uli_valid == TRUE) {

		if(flag_check  == ECGI_AND_TAI_PRESENT) {
			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_ECGI_AND_TAI_TYPE;
			ccr_request.data.ccr.tgpp_user_location_info.len =index ;

			len = fill_tai(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.tai2));

			ccr_request.data.ccr.tgpp_user_location_info.len += len;

			len  = fill_ecgi(&(ccr_request.data.ccr.tgpp_user_location_info.val[len + 1]), &(mb_req->uli.ecgi2));
			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1<< 0)) == TAI_PRESENT) ) {

			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_TAI_TYPE;

			ccr_request.data.ccr.tgpp_user_location_info.len = index ;

			len = fill_tai(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.tai2));

			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1 << 4)) == ECGI_PRESENT)) {

			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_ECGI_TYPE;
			ccr_request.data.ccr.tgpp_user_location_info.len = index ;
			len  = fill_ecgi(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.ecgi2));
			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1 << 2)) == SAI_PRESENT)) {

			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_SAI_TYPE;
			ccr_request.data.ccr.tgpp_user_location_info.len = index ;
			len  = fill_sai(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.sai2));
			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1 << 3)) == RAI_PRESENT)) {
			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_RAI_TYPE;
			ccr_request.data.ccr.tgpp_user_location_info.len = index ;
			len  = fill_rai(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.rai2));
			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1 << 1)) == CGI_PRESENT)) {

			ccr_request.data.ccr.presence.tgpp_user_location_info = PRESENT;
			ccr_request.data.ccr.tgpp_user_location_info.val[index++] = GX_CGI_TYPE;
			ccr_request.data.ccr.tgpp_user_location_info.len = index ;
			len  = fill_cgi(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.cgi2));
			ccr_request.data.ccr.tgpp_user_location_info.len += len;

		} else if (((flag_check & (1 << 6)) == 1)) {
			len = fill_lai(&(ccr_request.data.ccr.tgpp_user_location_info.val[index]), &(mb_req->uli.lai2));
		}

	}

	if( pdn->old_ue_tz_valid == TRUE ) {

		index = 0;
		ccr_request.data.ccr.presence.tgpp_ms_timezone = PRESENT;
		ccr_request.data.ccr.tgpp_ms_timezone.val[index++] = GX_UE_TIMEZONE_TYPE;
		ccr_request.data.ccr.tgpp_ms_timezone.val[index++] = ((pdn->ue_tz.tz) & 0xff);
		ccr_request.data.ccr.tgpp_ms_timezone.val[index++] = ((pdn->ue_tz.dst) & 0xff);

		ccr_request.data.ccr.tgpp_ms_timezone.len = index;

	}


	/* VS: Fill the Credit Crontrol Request to send PCRF */
	if(fill_ccr_request(&ccr_request.data.ccr, pdn->context, (bearer->eps_bearer_id - 5), pdn->gx_sess_id) != 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed CCR request filling process\n", __func__, __LINE__);
		return -1;
	}

	struct sockaddr_in saddr_in;
	saddr_in.sin_family = AF_INET;
	inet_aton("127.0.0.1", &(saddr_in.sin_addr));
	update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_INITIAL, SENT, GX);


	/* Update UE State */
	pdn->state = CCRU_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCRU_SNT_STATE;
	gx_context->proc = pdn->proc;

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
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type));
	return 0;
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

	ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
			(const void*)(pdn->gx_sess_id),
			(void **)&gx_context);
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
    	update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCR_INITIAL, SENT, GX);


	/* Update UE State */
	pdn->state = CCRU_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCRU_SNT_STATE;
	gx_context->proc = pdn->proc;
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
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type));
	return 0;
}

/**
 * @brief  : Generate reauth response
 * @param  : context , pointer to ue context structure
 * @param  : ebi_index, index in array where eps bearer is stored
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
gen_reauth_response(ue_context_t *context, uint8_t ebi_index)
{
	/* VS: Initialize the Gx Parameters */
	uint16_t msg_len = 0;
	char *buffer = NULL;
	gx_msg raa = {0};
	pdn_connection_t *pdn = NULL;
	gx_context_t *gx_context = NULL;
	uint16_t msg_type_ofs = 0;
	uint16_t msg_body_ofs = 0;
	uint16_t rqst_ptr_ofs = 0;
	uint16_t msg_len_total = 0;

	pdn = context->eps_bearers[ebi_index]->pdn;

	/* Clear Policy in PDN */
	pdn->policy.count = 0;
	pdn->policy.num_charg_rule_install = 0;
	pdn->policy.num_charg_rule_modify = 0;
	pdn->policy.num_charg_rule_delete = 0;

	/* Allocate the memory for Gx Context */
	gx_context = rte_malloc_socket(NULL,
			sizeof(gx_context_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());

	//strncpy(gx_context->gx_sess_id, context->pdns[ebi_index]->gx_sess_id, strlen(context->pdns[ebi_index]->gx_sess_id));


	raa.data.cp_raa.session_id.len = strlen(pdn->gx_sess_id);
	memcpy(raa.data.cp_raa.session_id.val, pdn->gx_sess_id, raa.data.cp_raa.session_id.len);

	raa.data.cp_raa.presence.session_id = PRESENT;

	/* VS: Set the Msg header type for CCR */
	raa.msg_type = GX_RAA_MSG;

	/* Result code */
	raa.data.cp_raa.result_code = 2001;
	raa.data.cp_raa.presence.result_code = PRESENT;

	/* Update UE State */
	pdn->state = RE_AUTH_ANS_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = RE_AUTH_ANS_SNT_STATE;

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msg_len = gx_raa_calc_length(&raa.data.cp_raa);
	msg_body_ofs = sizeof(raa.msg_type);
	rqst_ptr_ofs = msg_len + msg_body_ofs;
	msg_len_total = rqst_ptr_ofs + sizeof(pdn->rqst_ptr);

	//buffer = rte_zmalloc_socket(NULL, msg_len + sizeof(uint64_t) + sizeof(raa.msg_type),
	//		RTE_CACHE_LINE_SIZE, rte_socket_id());
	buffer = rte_zmalloc_socket(NULL, msg_len_total,
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (buffer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate CCR Buffer memory"
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return -1;
	}

	memcpy(buffer + msg_type_ofs, &raa.msg_type, sizeof(raa.msg_type));

	//if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + sizeof(raa.msg_type)), msg_len) == 0 )
	if (gx_raa_pack(&(raa.data.cp_raa), (unsigned char *)(buffer + msg_body_ofs), msg_len) == 0 )
		clLog(clSystemLog, eCLSeverityDebug,"RAA Packing failure\n");

	//memcpy((unsigned char *)(buffer + sizeof(raa.msg_type) + msg_len), &(context->eps_bearers[1]->rqst_ptr),
	memcpy((unsigned char *)(buffer + rqst_ptr_ofs), &(pdn->rqst_ptr),
			sizeof(pdn->rqst_ptr));
#if 0
	clLog(clSystemLog, eCLSeverityDebug,"While packing RAA %p %p\n", (void*)(context->eps_bearers[1]->rqst_ptr),
			*(void**)(buffer+rqst_ptr_ofs));

	clLog(clSystemLog, eCLSeverityDebug,"msg_len_total [%d] msg_type_ofs[%d] msg_body_ofs[%d] rqst_ptr_ofs[%d]\n",
			msg_len_total, msg_type_ofs, msg_body_ofs, rqst_ptr_ofs);
#endif
	/* VS: Write or Send CCR msg to Gx_App */
	send_to_ipc_channel(my_sock.gx_app_sock, buffer,
			msg_len_total);
			//msg_len + sizeof(raa.msg_type) + sizeof(unsigned long));

	return 0;
}

int
gx_context_entry_add(char *sess_id, gx_context_t *entry)
{
	int ret = 0;
	ret = rte_hash_add_key_data(gx_context_by_sess_id_hash,
			(const void *)sess_id , (void *)entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return -1;
	}
	return 0;
}

int
gx_context_entry_lookup(char *sess_id, gx_context_t **entry)
{
	int ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
			(const void*) (sess_id), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityDebug, "NO ENTRY FOUND IN UPF HASH [%s]\n", sess_id);
		return -1;
	}

	return 0;
}


