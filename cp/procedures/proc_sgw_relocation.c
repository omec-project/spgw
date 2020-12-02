// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "proc_sgw_relocation.h"
#include "spgwStatsPromEnum.h"
#include "sm_struct.h"
#include "cp_proc.h"
#include "spgw_cpp_wrapper.h"
#include "sm_enum.h"
#include "gtpv2_interface.h"
#include "ipc_api.h"
#include "clogger.h"
#include "pfcp_cp_session.h"
#include "gw_adapter.h"
#include "cp_io_poll.h"
#include "spgwStatsPromEnum.h"
#include "cp_config.h"
#include "tables/tables.h"
#include "util.h"
#include "gx.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_set_ie.h"
#include "sm_structs_api.h"
#include "rte_errno.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "cp_transactions.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

int
fill_cgi(uint8_t *buf, cgi_field_t *cgi); 

int
fill_sai(uint8_t *buf, sai_field_t *sai);

int
fill_rai(uint8_t *buf, rai_field_t *rai); 

int
fill_lai(uint8_t *buf, lai_field_t *lai);
int
fill_ecgi(uint8_t *buf, ecgi_field_t *ecgi); 
int
fill_tai(uint8_t *buf, tai_field_t *tai);
void
save_rai(rai_field_t *mb_rai, rai_t *context_rai);
void
save_sai(sai_field_t *mb_sai, sai_t *context_sai);
void
save_cgi(cgi_field_t *mb_cgi, cgi_t *context_cgi);
void
save_tai(tai_field_t *mb_tai, tai_t *context_tai);
int
compare_tai(tai_field_t *mb_tai, tai_t *context_tai);
int
compare_rai(rai_field_t *mb_rai, rai_t *context_rai);
int
compare_sai(sai_field_t *mb_sai, sai_t *context_sai);
int
compare_cgi(cgi_field_t *mb_cgi, cgi_t *context_cgi);
int
compare_ecgi(ecgi_field_t *mb_ecgi, ecgi_t *context_ecgi);
void
save_ecgi(ecgi_field_t *mb_ecgi, ecgi_t *context_ecgi);

proc_context_t* 
alloc_sgw_relocation_proc(msg_info_t *msg)
{
    proc_context_t *sgw_reloc_proc;
    sgw_reloc_proc = calloc(1, sizeof(proc_context_t));
    sgw_reloc_proc->proc_type = msg->proc; 
    sgw_reloc_proc->handler = sgw_relocation_event_handler;

    msg->proc_context = sgw_reloc_proc;
    SET_PROC_MSG(sgw_reloc_proc, msg);
 
    return sgw_reloc_proc;
}

void 
sgw_relocation_event_handler(void *proc, void *msg_info)
{
    proc_context_t *proc_context = (proc_context_t *)proc;
    msg_info_t *msg = (msg_info_t *) msg_info;
    uint8_t event = msg->event;
    RTE_SET_USED(proc_context);

    printf("Received event %d in %s \n",event, __FUNCTION__);
    switch(event) {
        case PFCP_SESS_EST_RESP_RCVD_EVNT: {
            process_sess_est_resp_sgw_reloc_handler(msg, NULL);
            break;
        }
        case MB_REQ_RCVD_EVNT: {
            process_mb_req_sgw_reloc_handler(msg, NULL);
            break;
        }
        case PFCP_SESS_MOD_RESP_RCVD_EVNT: {
            process_sess_mod_resp_sgw_reloc_handler(msg, NULL);
            break;
        }
        case MB_RESP_RCVD_EVNT: {
            process_mbr_resp_handover_handler(msg, NULL);
            break;
        }
        default: {
            assert(0); // unknown event 
        }
    }
    return;
}

// handle pfcp_Session est response in case of sgw/
int
process_sess_est_resp_sgw_reloc_handler(void *data, void *unused_param)
{
	/* SGW Relocation
	 * Handle pfcp session establishment response
	 * and send mbr request to PGWC
	 * Update proper state in hash as MBR_REQ_SNT_STATE
	 */

	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_est_resp(msg,
			&msg->pfcp_msg.pfcp_sess_est_resp, gtpv2c_tx);
	//ret = process_pfcp_sess_est_resp(
	//		msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid,
	//		gtpv2c_tx,
	//		msg->pfcp_msg.pfcp_sess_est_resp.up_fseid.seid);

	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return -1;
	}
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

//	if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == PGWC)) 

	gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &(my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));

    increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_MBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);

	if (SGWC == cp_config->cp_type) {
#if 0
		add_gtpv2c_if_timer_entry(
			UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid),
			&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
			UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid) - 5,
			S5S8_IFACE);
#endif
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}

int
process_mb_req_sgw_reloc_handler(void *data, void *unused_param)
{
	/* msg_info_t *msg = (msg_info_t *)data;
	 * Handle MBR for PGWC received from SGWC in case
	 * of SGW Relocation
	*/
	msg_info_t *msg = (msg_info_t *)data;
    int ret = 0;
	ret = process_pfcp_sess_mod_req_handover(&msg->gtpc_msg.mbr);
	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}

int process_pfcp_sess_mod_req_handover(mod_bearer_req_t *mb_req)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	ue_context_t *context = NULL;
	eps_bearer_t *bearer  = NULL;
	pdn_connection_t *pdn =  NULL;
	//pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ret = get_ue_context(mb_req->header.teid.has_teid.teid,
			                              &context);

	if (ret < 0 || !context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	ebi_index = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet\n");
		return -EPERM;
	}

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received modify bearer on non-existent EBI - "
				"Bitmap Inconsistency - Dropping packet\n");
		return -EPERM;
	}

	pdn = bearer->pdn;

	if(pdn->s5s8_sgw_gtpc_ipv4.s_addr != mb_req->sender_fteid_ctl_plane.ipv4_address)
	{
		pdn->old_sgw_addr = pdn->s5s8_sgw_gtpc_ipv4;
		pdn->old_sgw_addr_valid = true;
		pdn->s5s8_sgw_gtpc_ipv4.s_addr = mb_req->sender_fteid_ctl_plane.ipv4_address;
	}
	if(mb_req->ue_time_zone.header.len)
	{
		if((mb_req->ue_time_zone.time_zone != pdn->ue_tz.tz) ||
				(mb_req->ue_time_zone.daylt_svng_time != pdn->ue_tz.dst))
		{
			pdn->old_ue_tz = pdn->ue_tz;
			pdn->old_ue_tz_valid = TRUE;
			pdn->ue_tz.tz = mb_req->ue_time_zone.time_zone;
			pdn->ue_tz.dst = mb_req->ue_time_zone.daylt_svng_time;
		}
	}

	uint8_t flag_check_uli = 0;

	/*The above flag will be set bit wise as
	 * Bit 7| Bit 6 | Bit 5 | Bit 4 | Bit 3|  Bit 2|  Bit 1|  Bit 0 |
	 *---------------------------------------------------------------
	 *|     |       |       | ECGI  | RAI  |  SAI  |  CGI  |  TAI   |
	 ----------------------------------------------------------------
	 */


	if(mb_req->uli.header.len) {

		if(mb_req->uli.tai) {

			ret = compare_tai(&mb_req->uli.tai2, &pdn->context->uli.tai2);
			if(ret == FALSE) {
				flag_check_uli |= (1 << 0 );
				pdn->context->old_uli_valid = TRUE;
				save_tai(&mb_req->uli.tai2, &pdn->context->old_uli.tai2);
			}
		}

		if(mb_req->uli.cgi) {
			ret = compare_cgi(&mb_req->uli.cgi2, &pdn->context->uli.cgi2);
			if(ret == FALSE) {
				flag_check_uli |= ( 1<< 1 );
				pdn->context->old_uli_valid = TRUE;
				save_cgi(&mb_req->uli.cgi2, &pdn->context->old_uli.cgi2);
			}
		}
		if(mb_req->uli.sai) {
			ret = compare_sai(&mb_req->uli.sai2, &pdn->context->uli.sai2);
			if(ret == FALSE) {
				flag_check_uli |= (1 << 2 );
				pdn->context->old_uli_valid = TRUE;
				save_sai(&mb_req->uli.sai2, &pdn->context->old_uli.sai2);
			}
		}
		if(mb_req->uli.rai) {
			ret = compare_rai(&mb_req->uli.rai2, &pdn->context->uli.rai2);
			if(ret == FALSE) {
				flag_check_uli |= ( 1 << 3 );
				pdn->context->old_uli_valid = TRUE;
				save_rai(&mb_req->uli.rai2, &pdn->context->old_uli.rai2);
			}
		}
		if(mb_req->uli.ecgi) {
			ret = compare_ecgi(&mb_req->uli.ecgi2, &pdn->context->uli.ecgi2);
			if(ret == FALSE) {
				flag_check_uli |= (1 << 4);
				pdn->context->old_uli_valid = TRUE;
				save_ecgi(&mb_req->uli.ecgi2, &pdn->context->old_uli.ecgi2);
			}
		}
	}

	/* TODO something with modify_bearer_request.delay if set */
	if(((context->old_uli_valid == TRUE) && (((context->event_trigger & (1 << ULI_EVENT_TRIGGER))) != 0))
		|| ((pdn->old_ue_tz_valid == TRUE) && (((context->event_trigger) & (1 << UE_TIMEZONE_EVT_TRIGGER)) != 0))) {

        if(cp_config->gx_enabled) {
            ret = gen_ccru_request(pdn, bearer, mb_req, flag_check_uli);
        }
		pdn->context->old_uli_valid = FALSE;
		pdn->old_ue_tz_valid = FALSE;

#if 0
		/*Retrive the session information based on session id. */
		if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0){
			clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n", context->pdns[ebi_index]->seid);
			return -1;
		}
	    resp->gtpc_msg.mbr = *mb_req;
#endif

		return ret;
	}
	ret = send_pfcp_sess_mod_req_handover(pdn, bearer, mb_req);

	return 0;
}

// only for pgw 
int
process_sess_mod_resp_sgw_reloc_handler(void *data, void *unused_param)
{

	/* Use below function for reference
	 * This function is used in SGWU
	 * Create similar function to handle pfcp mod resp on PGWC
	 */

	uint16_t payload_length = 0;
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_mod_resp_handover(
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret) {
		if(ret != -1)
			mbr_error_response(msg, ret, cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
		       (struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));

    increment_mme_peer_stats(MSG_TX_GTPV2_S5S8_MBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);

	if (SGWC == cp_config->cp_type) {
#if 0
		add_gtpv2c_if_timer_entry(
			UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
			&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
			UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
			S5S8_IFACE);
#endif
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}

uint8_t
process_pfcp_sess_mod_resp_handover(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);

#ifdef TEMP
	/* Retrive the session information based on session id. */
	if (get_sess_entry_seid(sess_id, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n", sess_id);
		return -1;
	}

	/* Update the session state */
	resp->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	//ebi_index = resp->eps_bearer_id - 5;
	ebi_index = resp->eps_bearer_id ;
#endif
	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
	         clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
	                 __func__, __LINE__,
	                 teid);
	}

#if 0
	/* Update the UE state */
	ret = update_ue_state(context->pdns[ebi_index]->s5s8_pgw_gtpc_teid,
			PFCP_SESS_MOD_RESP_RCVD_STATE ,ebi_index);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State for teid: %u\n", __func__,
				context->pdns[ebi_index]->s5s8_pgw_gtpc_teid);
	}
#endif

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet\n");
		return -EPERM;
	}
	/* Fill the modify bearer response */

    uint32_t sequence=0;
	set_modify_bearer_response_handover(gtpv2c_tx,
			sequence, context, bearer);

	bearer->pdn->state = CONNECTED_STATE;
#ifdef TEMP
	/* Update the session state */
	resp->state = CONNECTED_STATE;
	/* Update the UE state */
	ret = update_ue_state(context->s11_sgw_gtpc_teid,
			CONNECTED_STATE,ebi_index);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State.\n", __func__);
	}
#endif

	my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
		htonl(bearer->pdn->s5s8_sgw_gtpc_ipv4.s_addr);

#if 0
	clLog(sxlogger, eCLSeverityDebug, "%s: s11_mme_sockaddr.sin_addr.s_addr :%s\n", __func__,
			inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));
#endif
	return 0;
}


int
process_sgwc_s5s8_modify_bearer_response(mod_bearer_rsp_t *mb_rsp, gtpv2c_header_t *gtpv2c_s11_tx)
{
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	eps_bearer_t *bearer = NULL;
	int ret = 0;

	uint8_t ebi_index =
		mb_rsp->bearer_contexts_modified.eps_bearer_id.ebi_ebi - 5;

	/* s11_sgw_gtpc_teid= s5s8_sgw_gtpc_teid =
	 * key->ue_context_by_fteid_hash */

	 ret = get_ue_context_by_sgw_s5s8_teid(mb_rsp->header.teid.has_teid.teid, &context);
	 if (ret < 0 || !context)
	         return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	bearer = context->eps_bearers[ebi_index];
	pdn = bearer->pdn;

	set_create_session_response(
			gtpv2c_s11_tx, mb_rsp->header.teid.has_teid.seq,
			context, pdn, bearer);

	 pdn->state =  CONNECTED_STATE;
	 //pdn->proc = INITIAL_PDN_ATTACH_PROC;

	return 0;
}

int process_mbr_resp_handover_handler(void *data, void *rx_buf)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;
	ret = process_sgwc_s5s8_modify_bearer_response(&(msg->gtpc_msg.mb_rsp) ,gtpv2c_tx);

	if (ret) {
		mbr_error_response(msg, ret, cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}


#ifdef TEMP
	uint16_t payload_length = 0;
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);
	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
			(struct sockaddr *) &s11_mme_sockaddr,
			sizeof(struct sockaddr_in));

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_CSRSP, s11_mme_sockaddr.sin_addr.s_addr);
#endif
	increment_stat(NUM_UE_SGW_ACTIVE_SUBSCRIBERS);

	RTE_SET_USED(data);
	RTE_SET_USED(rx_buf);

	return 0;
}

int 
process_sgwc_delete_handover(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
	uint16_t msg_len = 0;
	int ret = 0;
	ue_context_t *context = NULL;
	del_sess_rsp_t del_resp = {0};

	uint32_t teid = UE_SESS_ID(sess_id);
	//gtpv2c_header gtpv2c_rx;

#ifdef TEMP
	if (get_sess_entry_seid(sess_id, &resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n", sess_id);
		return -1;
	}
#endif

	ret = get_ue_context(teid, &context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to update UE State for teid: %u\n",
				__func__, __LINE__,
				teid);
	}

    uint32_t sequence = 0; // TODO...
	set_gtpv2c_teid_header((gtpv2c_header_t *) &del_resp, GTP_DELETE_SESSION_RSP,
			context->s11_mme_gtpc_teid, sequence);

	set_cause_accepted_ie((gtpv2c_header_t *) &del_resp, IE_INSTANCE_ZERO);

	del_resp.cause.header.len = ntohs(del_resp.cause.header.len);

	/*Encode the S11 delete session response message. */
	msg_len = encode_del_sess_rsp(&del_resp, (uint8_t *)gtpv2c_tx);

	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

#ifdef TEMP
	s11_mme_sockaddr.sin_addr.s_addr =
		htonl(context->s11_mme_gtpc_ipv4.s_addr);

	clLog(s11logger, eCLSeverityDebug, "SAEGWC:%s:"
			"s11_mme_sockaddr.sin_addr.s_addr :%s\n", __func__,
			inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));
#endif

	/* Delete entry from session entry */
	if (del_sess_entry_seid(sess_id) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for Key sess ID:%lu\n", sess_id);
		return -1;
	}
	return 0;
}

void
save_ecgi(ecgi_field_t *mb_ecgi, ecgi_t *context_ecgi)
{
		context_ecgi->ecgi_mcc_digit_2 = mb_ecgi->ecgi_mcc_digit_2;
		context_ecgi->ecgi_mcc_digit_1 = mb_ecgi->ecgi_mcc_digit_1;
		context_ecgi->ecgi_mnc_digit_3 = mb_ecgi->ecgi_mnc_digit_3;
		context_ecgi->ecgi_mcc_digit_3 = mb_ecgi->ecgi_mcc_digit_3;
		context_ecgi->ecgi_mnc_digit_2 = mb_ecgi->ecgi_mnc_digit_2;
		context_ecgi->ecgi_mnc_digit_1 = mb_ecgi->ecgi_mnc_digit_1;
		context_ecgi->ecgi_spare = mb_ecgi->ecgi_spare;
		context_ecgi->eci = mb_ecgi->eci;
}

int
compare_ecgi(ecgi_field_t *mb_ecgi, ecgi_t *context_ecgi)
{
		if(mb_ecgi->ecgi_mcc_digit_1 != context_ecgi->ecgi_mcc_digit_1)
			return FALSE;
		if(mb_ecgi->ecgi_mcc_digit_2 != context_ecgi->ecgi_mcc_digit_2)
			return FALSE;
		if(mb_ecgi->ecgi_mcc_digit_3 != context_ecgi->ecgi_mcc_digit_3)
			return FALSE;
		if(mb_ecgi->ecgi_mnc_digit_1 != context_ecgi->ecgi_mnc_digit_1)
			return FALSE;
		if(mb_ecgi->ecgi_mnc_digit_2 != context_ecgi->ecgi_mnc_digit_2)
			return FALSE;
		if(mb_ecgi->ecgi_mnc_digit_3 != context_ecgi->ecgi_mnc_digit_3)
			return FALSE;
		if(mb_ecgi->ecgi_spare != context_ecgi->ecgi_spare)
			return FALSE;
		if(mb_ecgi->eci != context_ecgi->eci)
			return FALSE;

		return TRUE;
}

int
compare_cgi(cgi_field_t *mb_cgi, cgi_t *context_cgi)
{
		if(mb_cgi->cgi_mcc_digit_1 != context_cgi->cgi_mcc_digit_1)
			return FALSE;
		if(mb_cgi->cgi_mcc_digit_2 != context_cgi->cgi_mcc_digit_2)
			return FALSE;
		if(mb_cgi->cgi_mcc_digit_3 != context_cgi->cgi_mcc_digit_3)
			return FALSE;
		if(mb_cgi->cgi_mnc_digit_1 != context_cgi->cgi_mnc_digit_1)
			return FALSE;
		if(mb_cgi->cgi_mnc_digit_2 != context_cgi->cgi_mnc_digit_2)
			return FALSE;
		if(mb_cgi->cgi_mnc_digit_3 != context_cgi->cgi_mnc_digit_3)
			return FALSE;
		if(mb_cgi->cgi_lac != context_cgi->cgi_lac)
			return FALSE;
		if(mb_cgi->cgi_ci != context_cgi->cgi_ci)
			return FALSE;

		return TRUE;
}

int
compare_sai(sai_field_t *mb_sai, sai_t *context_sai)
{
		if(mb_sai->sai_mcc_digit_1 != context_sai->sai_mcc_digit_1)
			return FALSE;
		if(mb_sai->sai_mcc_digit_2 != context_sai->sai_mcc_digit_2)
			return FALSE;
		if(mb_sai->sai_mcc_digit_3 != context_sai->sai_mcc_digit_3)
			return FALSE;
		if(mb_sai->sai_mnc_digit_1 != context_sai->sai_mnc_digit_1)
			return FALSE;
		if(mb_sai->sai_mnc_digit_2 != context_sai->sai_mnc_digit_2)
			return FALSE;
		if(mb_sai->sai_mnc_digit_3 != context_sai->sai_mnc_digit_3)
			return FALSE;
		if(mb_sai->sai_lac != context_sai->sai_lac)
			return FALSE;
		if(mb_sai->sai_sac != context_sai->sai_sac)
			return FALSE;

		return TRUE;
}

int
compare_rai(rai_field_t *mb_rai, rai_t *context_rai)
{
		if(mb_rai->ria_mcc_digit_1 != context_rai->ria_mcc_digit_1)
			return FALSE;
		if(mb_rai->ria_mcc_digit_2 != context_rai->ria_mcc_digit_2)
			return FALSE;
		if(mb_rai->ria_mcc_digit_3 != context_rai->ria_mcc_digit_3)
			return FALSE;
		if(mb_rai->ria_mnc_digit_1 != context_rai->ria_mnc_digit_1)
			return FALSE;
		if(mb_rai->ria_mnc_digit_2 != context_rai->ria_mnc_digit_2)
			return FALSE;
		if(mb_rai->ria_mnc_digit_3 != context_rai->ria_mnc_digit_3)
			return FALSE;
		if(mb_rai->ria_lac != context_rai->ria_lac)
			return FALSE;
		if(mb_rai->ria_rac != context_rai->ria_rac)
			return FALSE;

		return TRUE;
}

int
compare_tai(tai_field_t *mb_tai, tai_t *context_tai)
{
		if(mb_tai->tai_mcc_digit_1 != context_tai->tai_mcc_digit_1)
			return FALSE;
		if(mb_tai->tai_mcc_digit_2 != context_tai->tai_mcc_digit_2)
			return FALSE;
		if(mb_tai->tai_mcc_digit_3 != context_tai->tai_mcc_digit_3)
			return FALSE;
		if(mb_tai->tai_mnc_digit_1 != context_tai->tai_mnc_digit_1)
			return FALSE;
		if(mb_tai->tai_mnc_digit_2 != context_tai->tai_mnc_digit_2)
			return FALSE;
		if(mb_tai->tai_mnc_digit_3 != context_tai->tai_mnc_digit_3)
			return FALSE;
		if(mb_tai->tai_tac != context_tai->tai_tac)
			return FALSE;

		return TRUE;
}


void
save_tai(tai_field_t *mb_tai, tai_t *context_tai)
{

	//context_tai->uli_old.tai = uli->tai;
	context_tai->tai_mcc_digit_2 = mb_tai->tai_mcc_digit_2;
	context_tai->tai_mcc_digit_1 = mb_tai->tai_mcc_digit_1;
	context_tai->tai_mnc_digit_3 = mb_tai->tai_mnc_digit_3;
	context_tai->tai_mcc_digit_3 = mb_tai->tai_mcc_digit_3;
	context_tai->tai_mnc_digit_2 = mb_tai->tai_mnc_digit_2;
	context_tai->tai_mnc_digit_1 = mb_tai->tai_mnc_digit_1;
	context_tai->tai_tac = mb_tai->tai_tac;

}

void
save_cgi(cgi_field_t *mb_cgi, cgi_t *context_cgi)
{
		context_cgi->cgi_mcc_digit_2 = mb_cgi->cgi_mcc_digit_2;
		context_cgi->cgi_mcc_digit_1 = mb_cgi->cgi_mcc_digit_1;
		context_cgi->cgi_mnc_digit_3 = mb_cgi->cgi_mnc_digit_3;
		context_cgi->cgi_mcc_digit_3 = mb_cgi->cgi_mcc_digit_3;
		context_cgi->cgi_mnc_digit_2 = mb_cgi->cgi_mnc_digit_2;
		context_cgi->cgi_mnc_digit_1 = mb_cgi->cgi_mnc_digit_1;
		context_cgi->cgi_lac = mb_cgi->cgi_lac;
		context_cgi->cgi_ci = mb_cgi->cgi_ci;

}

void
save_sai(sai_field_t *mb_sai, sai_t *context_sai)
{
		context_sai->sai_mcc_digit_2 = mb_sai->sai_mcc_digit_2;
		context_sai->sai_mcc_digit_1 = mb_sai->sai_mcc_digit_1;
		context_sai->sai_mnc_digit_3 = mb_sai->sai_mnc_digit_3;
		context_sai->sai_mcc_digit_3 = mb_sai->sai_mcc_digit_3;
		context_sai->sai_mnc_digit_2 = mb_sai->sai_mnc_digit_2;
		context_sai->sai_mnc_digit_1 = mb_sai->sai_mnc_digit_1;
		context_sai->sai_lac         = mb_sai->sai_lac;
		context_sai->sai_sac         = mb_sai->sai_sac;

}

void
save_rai(rai_field_t *mb_rai, rai_t *context_rai)
{
		context_rai->ria_mcc_digit_2 = mb_rai->ria_mcc_digit_2;
		context_rai->ria_mcc_digit_1 = mb_rai->ria_mcc_digit_1;
		context_rai->ria_mnc_digit_3 = mb_rai->ria_mnc_digit_3;
		context_rai->ria_mcc_digit_3 = mb_rai->ria_mcc_digit_3;
		context_rai->ria_mnc_digit_2 = mb_rai->ria_mnc_digit_2;
		context_rai->ria_mnc_digit_1 = mb_rai->ria_mnc_digit_1;
		context_rai->ria_lac = mb_rai->ria_lac;
		context_rai->ria_rac = mb_rai->ria_rac;

}

int
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

int
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

int
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

int
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

int
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

int
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

	int ret = get_gx_context((uint8_t*)pdn->gx_sess_id,&gx_context);
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
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)) + sizeof(ccr_request.seq_num), msg_len) == 0) {
		clLog(clSystemLog, eCLSeverityCritical, "ERROR:%s:%d Packing CCR Buffer... \n", __func__, __LINE__);
		return -1;

	}

	/* VS: Write or Send CCR msg to Gx_App */
	gx_send(my_sock.gx_app_sock, buffer,
			msg_len + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
	return 0;
}

void 
send_pfcp_sess_mod_req_handover_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}


int 
send_pfcp_sess_mod_req_handover(pdn_connection_t *pdn, eps_bearer_t *bearer,
		mod_bearer_req_t *mb_req)
{
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	int ebi_index = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi - 5 ;
	if (!(pdn->context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet\n");
		return -EPERM;
	}
	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];
	pfcp_sess_mod_req.update_far_count = 0;

	if (mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.header.len  != 0){
		/* SGW Relocation */
		bearer->s5s8_sgw_gtpu_ipv4.s_addr =
			mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.ipv4_address;
		bearer->s5s8_sgw_gtpu_teid =
			mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.teid_gre_key;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s5s8_sgw_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s5s8_sgw_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(mb_req->bearer_contexts_to_be_modified.s58_u_sgw_fteid.interface_type);
		if ( cp_config->cp_type != PGWC) {
			update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		}
		pfcp_sess_mod_req.update_far_count++;
	}

	//context->pdns[ebi_index]->seid = SESS_ID(context->s11_sgw_gtpc_teid, bearer->eps_bearer_id);
	pdn->seid = SESS_ID(pdn->context->s11_sgw_gtpc_teid, bearer->eps_bearer_id);

	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &mb_req->header, bearer, pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &pdn->context->upf_context->upf_sockaddr);

	/* Update UE State */
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(pdn->context->upf_context)); 
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(pdn->context, pfcp_msg, encoded, send_pfcp_sess_mod_req_handover_timeout);
    pdn->trans_entry = trans_entry;

	/*Retrive the session information based on session id. */
	//if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0)
	//
	//
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;
#ifdef TEMP
	pdn->proc= SGW_RELOCATION_PROC;//GTP_MODIFY_BEARER_REQ;
	if (get_sess_entry_seid(pdn->seid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n", pdn->seid);
		return -1;
	}

	/* Set create session response */
	resp->eps_bearer_id = mb_req->bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi -5 ;
	resp->msg_type = GTP_MODIFY_BEARER_REQ;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = pdn->proc;
#endif
	return 0;
}

