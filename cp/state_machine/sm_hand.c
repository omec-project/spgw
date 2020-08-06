// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "pfcp.h"
#include "gx_interface.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_session.h"
#include "cp_config.h"
#include "clogger.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "trans_struct.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_association_setup_proc.h"
#include "tables/tables.h"


extern udp_sock_t my_sock;

extern socklen_t s5s8_sockaddr_len;
extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];
extern struct rte_hash *bearer_by_fteid_hash;
extern struct cp_stats_t cp_stats;


#ifdef FUTURE_NEED
int
gx_setup_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;

	ret = process_create_sess_req(&msg->gtpc_msg.csr, &context, &pdn, msg);
	if (ret != 0 && ret != -2) {
		if (ret != -1){

			cs_error_response(msg, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
			process_error_occured_handler_new(data, unused_param);
		}
		clLog(sxlogger, eCLSeverityCritical, "[%s]:[%s]:[%d] Error: %d \n",
				__file__, __func__, __LINE__, ret);
		return -1;
	}

	RTE_SET_USED(unused_param);
	return ret;
}
#endif

#ifdef FUTURE_NEED
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

	ret = process_pfcp_sess_est_resp(
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
				s5s8_sockaddr_len);

	update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, SENT,S5S8);

	if (SGWC == cp_config->cp_type) {
		add_gtpv2c_if_timer_entry(
			UE_SESS_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid),
			&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
			UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_est_resp.header.seid_seqno.has_seid.seid) - 5,
			S5S8_IFACE);
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}
#endif

/*
This function Handles the CCA-T received from PCEF
*/
int
cca_t_msg_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;
	gx_context_t *gx_context = NULL;

	RTE_SET_USED(unused_param);

	/* Retrive Gx_context based on Sess ID. */
	ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
			(const void*)(msg->gx_msg.cca.session_id.val),
			(void **)&gx_context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
				msg->gx_msg.cca.session_id.val);
		return -1;
	}

	if(rte_hash_del_key(gx_context_by_sess_id_hash, msg->gx_msg.cca.session_id.val) < 0){
		clLog(clSystemLog, eCLSeverityCritical,
				"%s %s - Error on gx_context_by_sess_id_hash deletion\n",__file__,
				strerror(ret));
	}

	rte_free(gx_context);
	return 0;
}

/*
 * This function handles the message received
 * from PCEF in case of handover.
 * This handler comes when MBR is received
 * from the new SGWC on the PGWC.
 * */
#ifdef FUTURE_NEED
int cca_u_msg_handler_handover(void *data, void *unused)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint32_t call_id = 0;
	pdn_connection_t *pdn = NULL;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer = NULL;

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&msg->gx_msg.cca.session_id.val, &call_id);
	if (ret < 0) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                       (char*) &msg->gx_msg.cca.session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn = get_pdn_conn_entry(call_id);
	if (pdn == NULL)
	{
	      clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
	                          __func__, call_id);
	      return -1;
	}

    proc_context_t *proc_context = pdn->context->current_proc;

	ebi_index = pdn->default_bearer_id - 5; 

	if (!(pdn->context->bearer_bitmap & (1 << ebi_index))) {
		clLog(clSystemLog, eCLSeverityCritical,
				"Received modify bearer on non-existent EBI - "
				"Dropping packet\n");
		return -EPERM;
	}

	bearer = pdn->eps_bearers[ebi_index];

	ret = send_pfcp_sess_mod_req_handover(pdn, bearer, &resp->gtpc_msg.mbr);
	 if (ret) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
	         return ret;
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused);

	return 0;
}
#endif

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
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d:Error"
				"\n%s: (%d) %s\n", __func__, __LINE__,
				gx_type_str(msg->msg_type), ebi_index,
				(ebi_index < 0 ? strerror(-ebi_index) : cause_str(ebi_index)));
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
                ret = get_sess_entry(pdn->seid, &resp);
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


#ifdef FUTURE_NEED
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
		          s5s8_sockaddr_len);

	update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, SENT,S5S8);


	if (SGWC == cp_config->cp_type) {
		add_gtpv2c_if_timer_entry(
			UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
			&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
			UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
			S5S8_IFACE);
	}

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}

int
process_pfcp_sess_mod_resp_cbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_mod_resp(
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
			/* TODO for cbr
			 * mbr_error_response(&msg->gtpc_msg.mbr, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE); */
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
																			&resp) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if ((SAEGWC != cp_config->cp_type) && ((resp->msg_type == GTP_CREATE_BEARER_RSP) ||
			(resp->msg_type == GX_RAR_MSG))){
	    gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
	            s5s8_sockaddr_len);
		if(resp->msg_type != GTP_CREATE_BEARER_RSP){
			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&(my_sock.s5s8_recv_sockaddr), gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S5S8_IFACE);
		}
		if (resp->msg_type == GTP_CREATE_BEARER_RSP) {

			update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, ACC,S5S8);
		}
		else {

			update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, SENT,S5S8);
		}

	} else {
		if(resp->msg_type != GX_RAA_MSG) {
		    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
		            (struct sockaddr *) &s11_mme_sockaddr,
		            sizeof(struct sockaddr_in));

			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&s11_mme_sockaddr, gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S11_IFACE);

			update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, SENT,S11);
		}
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_cbresp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	RTE_SET_USED(unused_param);
	return 0;
}
#endif

#ifdef FUTURE_NEED
int process_mbr_resp_handover_handler(void *data, void *rx_buf)
{
    int ret = 0;
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;
	ret = process_sgwc_s5s8_modify_bearer_response(&(msg->gtpc_msg.mb_rsp) ,gtpv2c_tx);

	if (ret) {
		mbr_error_response(msg, ret, cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
			(struct sockaddr *) &s11_mme_sockaddr,
			sizeof(struct sockaddr_in));

	update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, ACC,S11);
	update_sys_stat(number_of_users, INCREMENT);
	update_sys_stat(number_of_active_session, INCREMENT);

	RTE_SET_USED(data);
	RTE_SET_USED(rx_buf);

	return 0;
}
#endif

#ifdef FUTURE_NEED
int
process_create_bearer_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_sgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_create_bearer_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_create_bearer_request(&msg->gtpc_msg.cb_req);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

#endif
int
process_rar_request_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = parse_gx_rar_msg(&msg->gx_msg.rar);
	if (ret) {
		if(ret != -1){
			uint32_t call_id = 0;
			pdn_connection_t *pdn_cntxt = NULL;
			ret = retrieve_call_id((char *)&msg->gx_msg.rar.session_id.val, &call_id);
			if (ret < 0) {
	        		clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                        msg->gx_msg.rar.session_id.val);
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
			gen_reauth_error_response(pdn_cntxt, ret);
		}
		clLog(sxlogger, eCLSeverityCritical, "%s:%s:%d Error: %d \n",
				__FILE__, __func__, __LINE__, ret);
		return -1;
	}
	RTE_SET_USED(unused_param);
	return 0;
}

int
pfd_management_handler(void *data, void *unused_param)
{
	clLog(sxlogger, eCLSeverityDebug,
		"Pfcp Pfd Management Response Recived Successfully \n");

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}

#ifdef FUTURE_NEED
int
process_mod_resp_delete_handler(void *data, void *unused_param)
{
    int ret = 0;
	uint16_t payload_length = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_mod_resp(
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret) {
		mbr_error_response(msg, ret, cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (cp_config->cp_type == SGWC) {
		/* Forward s11 delete_session_request on s5s8 */
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
				s5s8_sockaddr_len);

		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
						gtpv2c_tx->gtpc.message_type, SENT,S5S8);
		add_gtpv2c_if_timer_entry(
			UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
			&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
			UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
			S5S8_IFACE);

	} else {
		/*Code should not reach here since this handler is only for SGWC*/
		return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}
#endif



#ifdef FUTURE_NEED
/* dedicated bearer - deactivation PDN initiated deactivation */
int
process_pfcp_sess_mod_resp_dbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;
    ue_context_t  *ue = msg->ue_context; 

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_delete_bearer_pfcp_sess_response(
		msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
		gtpv2c_tx);
	if (ret != 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

    ue_context_t *temp = NULL;
	get_sess_entry(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid, &temp);
    assert(temp == ue_context);

	if ((SAEGWC != cp_config->cp_type) &&
		((resp->msg_type == GTP_DELETE_BEARER_RSP) ||
		(resp->msg_type == GX_RAR_MSG))) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
	            s5s8_sockaddr_len);

		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, SENT, S5S8);

		if (resp->msg_type != GTP_DELETE_BEARER_RSP) {
			add_gtpv2c_if_timer_entry(
				UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
				&my_sock.s5s8_recv_sockaddr, gtp_tx_buf, payload_length,
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
				S5S8_IFACE);
		}

	} else if (resp->msg_type != GX_RAA_MSG) {
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,
				sizeof(struct sockaddr_in));

		add_gtpv2c_if_timer_entry(
				UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
				&s11_mme_sockaddr, gtp_tx_buf, payload_length,
				UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
				S11_IFACE);

		update_cli_stats(s11_mme_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, SENT,
				S11);
	}

	RTE_SET_USED(unused_param);

	return 0;
}
#endif

#ifdef SUPPORT_DEDICATED_BEARER
int
process_pfcp_sess_del_resp_dbr_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_delete_bearer_pfcp_sess_response(
		msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
		gtpv2c_tx);
	if (ret != 0) {
		clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (get_sess_entry(
		msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid,
		&resp) != 0) {
		clLog(sxlogger, eCLSeverityCritical,
			"%s:%d NO Session Entry Found for sess ID:%lu\n",
			__func__, __LINE__,
			msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);

		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if ((SAEGWC != cp_config->cp_type) &&
		((resp->msg_type == GTP_DELETE_BEARER_RSP))) {
			gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
		            (struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
	        	    s5s8_sockaddr_len);

		if (resp->msg_type == GTP_DELETE_BEARER_RSP) {
			update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, SENT, S5S8);
		}
	}

	RTE_SET_USED(unused_param);

	return 0;
}
#endif

/*DELETE bearer commaand deactivation*/

/*
 * This handler is called when CCA-U is received on PGWC.
 * and PGWC will send session modification to PGWU.
 * On Combined, SAEGWC will send this to SAEGWU.
 *
*/

#ifdef FUTURE_NEED
int del_bearer_cmd_ccau_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint32_t call_id = 0;
	pdn_connection_t *pdn = NULL;

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&msg->gx_msg.cca.session_id.val, &call_id);
	if (ret < 0) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                       (char*) &msg->gx_msg.cca.session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn = get_pdn_conn_entry(call_id);
	if (pdn == NULL)
	{
	      clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
	                          __func__, call_id);
	      return -1;
	}


	ret = process_sess_mod_req_del_cmd(pdn);
	if (ret != 0) {
		clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}
	RTE_SET_USED(unused_param);
	return 0;
}
#endif


/* This handler is called when SGWC-PGWC-SAEWC receives
 * delete bearer response from MME-SGWC-MME
 *
 */

#ifdef FUTURE_NEED
int
process_delete_bearer_response_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	ret = process_delete_bearer_resp(&msg->gtpc_msg.db_rsp, 1);
	if (ret != 0) {
		clLog(s11logger, eCLSeverityCritical, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}
	RTE_SET_USED(unused_param);
	return 0;
}
#endif

/*
 * This handler will be called when PFCP MOD is received from
 * PGWU on PGWC
 * On combined it will be recieved on SAEGWC
 * */

#ifdef FUTURE_NEED
int
del_bearer_cmd_mbr_resp_handler(void *data, void *unused_param)
{
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;
	uint8_t flag = -1;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;
	ret = process_pfcp_sess_mod_resp_del_cmd
			(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			 gtpv2c_tx ,&flag);

	if (ret) {
		clLog(sxlogger, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return ret;
	}
	if(flag == 0){
		return 0;
	}
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);


	if (PGWC == cp_config->cp_type ) {
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
				s5s8_sockaddr_len);
		update_cli_stats(my_sock.s5s8_recv_sockaddr.sin_addr.s_addr,
				gtpv2c_tx->gtpc.message_type, SENT,
				S5S8);
	} else if ((SGWC == cp_config->cp_type) ||
				(SAEGWC == cp_config->cp_type)) {

		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,  /* need change - future */
				sizeof(struct sockaddr_in));
	}

	RTE_SET_USED(unused_param);
	return 0;
}
#endif


void
get_info_filled(msg_info_t *msg, err_rsp_info *info_resp)
{
	//pdn_connection_t *pdn = NULL;

	switch(msg->msg_type){
        case GTP_CREATE_SESSION_REQ: {
                info_resp->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                info_resp->teid =  msg->gtpc_msg.csr.header.teid.has_teid.teid;
                break;
            }

        case GTP_RESERVED: {
                ue_context_t *ue_context = msg->ue_context; 
			    info_resp->teid = ue_context->s11_sgw_gtpc_teid;
                // assumed to be triggered due to CSReq 
                info_resp->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi - 5;
                break;
            }

		case PFCP_ASSOCIATION_SETUP_RESPONSE:{
            ue_context_t  *ue = msg->ue_context; 
            pdn_connection_t *pdn = msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			// info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
            ue_context_t  *ue = msg->ue_context; 
            pdn_connection_t *pdn = msg->pdn_context;
			info_resp->sender_teid = ue->s11_mme_gtpc_teid;
            // TODO - Need more thought 
			//info_resp->seq = ue->sequence;
			info_resp->ebi_index = pdn->default_bearer_id - 5;
			info_resp->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{

			if(msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				info_resp->ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi - 5;
			info_resp->teid = msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {

			info_resp->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid);
			info_resp->ebi_index = UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_del_resp.header.seid_seqno.has_seid.seid) - 5;

			break;
		}

	}
}

/* Function */
int
process_del_pdn_conn_set_req(void *data, void *unused_param)
{
#ifdef USE_CSID
	int ret = 0;
	uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_del_pdn_conn_set_req_t(&msg->gtpc_msg.del_pdn_req,
			gtpv2c_tx);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (msg->gtpc_msg.del_pdn_req.pgw_fqcsid.number_of_csids) {
		/* Send the delete PDN set request to MME */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,
				sizeof(struct sockaddr_in));

		memset(gtpv2c_tx, 0, sizeof(gtpv2c_header_t));
	}

	if (msg->gtpc_msg.del_pdn_req.mme_fqcsid.number_of_csids) {
		/* Send the delete PDN set request to PGW */
		if (cp_config->cp_type == SGWC ) {
			gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
					(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
					s5s8_sockaddr_len);

		}
		memset(gtpv2c_tx, 0, sizeof(gtpv2c_header_t));
	}
	/* Send Response back to peer node */
	ret = fill_gtpc_del_set_pdn_conn_rsp(gtpv2c_tx,
			msg->gtpc_msg.del_pdn_req.header.teid.has_teid.seq,
			GTPV2C_CAUSE_REQUEST_ACCEPTED);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if (msg->gtpc_msg.del_pdn_req.pgw_fqcsid.number_of_csids) {
		/* Send response to PGW */
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
				s5s8_sockaddr_len);
	}

	if (msg->gtpc_msg.del_pdn_req.mme_fqcsid.number_of_csids) {
		/* Send the delete PDN set request to MME */
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &s11_mme_sockaddr,
				sizeof(struct sockaddr_in));
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */

	RTE_SET_USED(unused_param);
	return 0;
}

/* Function */
//int
//process_s5s8_del_pdn_conn_set_req(void *data, void *unused_param)
//{
//#ifdef USE_CSID
//	uint16_t payload_length = 0;
//	msg_info_t *msg = (msg_info_t *)data;
//
//	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
//	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;
//
//	ret = process_del_pdn_conn_set_req_t(&msg->gtpc_msg.del_pdn_req,
//			gtpv2c_tx);
//	if (ret) {
//			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
//					ERR_MSG, ret);
//			return -1;
//	}
//
//	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
//		+ sizeof(gtpv2c_tx->gtpc);
//
//	/* Send the delete PDN set request to MME */
//	if (cp_config->cp_type == SGWC ) {
//		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
//				(struct sockaddr *) &s11_mme_sockaddr,
//				s11_mme_sockaddr_len);
//	}
//
//	/* Send Response back to peer node */
//	ret = fill_gtpc_del_set_pdn_conn_rsp(gtpv2c_tx,
//			msg->gtpc_msg.del_pdn_req.header.teid.has_teid.seq,
//			GTPV2C_CAUSE_REQUEST_ACCEPTED);
//	if (ret) {
//			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
//					ERR_MSG, ret);
//			return -1;
//	}
//
//	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
//		+ sizeof(gtpv2c_tx->gtpc);
//
//	/* Send response to PGW */
//	gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
//			(struct sockaddr *) &s5s8_recv_sockaddr,
//			s5s8_sockaddr_len);
//#else
//	RTE_SET_USED(data);
//#endif /* USE_CSID */
//
//	RTE_SET_USED(unused_param);
//	return 0;
//}
/* Function */
int
process_del_pdn_conn_set_rsp(void *data, void *unused_param)
{
#ifdef USE_CSID
	//uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;

	ret = process_del_pdn_conn_set_rsp_t(&msg->gtpc_msg.del_pdn_rsp);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */

	RTE_SET_USED(unused_param);
	return 0;
}

/* Function */
// ajay - unhandled fsm - RESTORATION_RECOVERY_PROC CONNECTED_STATE UPD_PDN_CONN_SET_REQ_RCVD_EVNT => process_upd_pdn_conn_set_req
// ajay unhandled fsm - RESTORATION_RECOVERY_PROC CONNECTED_STATE UPD_PDN_CONN_SET_REQ_RCVD_EVNT ==> process_upd_pdn_conn_set_req
// ajay unhandled fsm - RESTORATION_RECOVERY_PROC CONNECTED_STATE UPD_PDN_CONN_SET_REQ_RCVD_EVNT - process_upd_pdn_conn_set_req 
int
process_upd_pdn_conn_set_req(void *data, void *unused_param)
{
#ifdef USE_CSID
	//uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;

	ret = process_upd_pdn_conn_set_req_t(&msg->gtpc_msg.upd_pdn_req);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */

	RTE_SET_USED(unused_param);
	return 0;
}

/* Function */
// ajay - unhandled fsm saegw - RESTORATION_RECOVERY_PROC UPD_PDN_CONN_SET_REQ_SNT_STATE UPD_PDN_CONN_SET_RESP_RCVD_EVNT => process_upd_pdn_conn_set_rsp 
// ajay unhandled fsm  pgw - RESTORATION_RECOVERY_PROC UPD_PDN_CONN_SET_REQ_SNT_STATE UPD_PDN_CONN_SET_RESP_RCVD_EVNT ==> process_upd_pdn_conn_set_rsp
// ajay unhandled fsm sgw - RESTORATION_RECOVERY_PROC UPD_PDN_CONN_SET_REQ_SNT_STATE UPD_PDN_CONN_SET_RESP_RCVD_EVNT process_upd_pdn_conn_set_rsp 
int
process_upd_pdn_conn_set_rsp(void *data, void *unused_param)
{
#ifdef USE_CSID
	//uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;
	int ret = 0;

	ret = process_upd_pdn_conn_set_rsp_t(&msg->gtpc_msg.upd_pdn_rsp);
	if (ret) {
			clLog(s11logger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */

	RTE_SET_USED(unused_param);
	return 0;
}


/* Function */
// ajay - unhandled fsm 
//    pgw RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_RCVD_STATE PFCP_SESS_SET_DEL_REQ_RCVD_EVNT ==> process_pfcp_sess_set_del_req 
// sgw unhandled - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_RCVD_STATE PFCP_SESS_SET_DEL_RESP_RCVD_EVNT - process_pfcp_sess_set_del_req

int process_pfcp_sess_set_del_req(void *data, void *unused_param)
{
#ifdef USE_CSID
	int ret = 0;
	//uint16_t payload_length = 0;
	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_set_del_req_t(&msg->pfcp_msg.pfcp_sess_set_del_req,
			gtpv2c_tx);
	if (ret) {
			clLog(sxlogger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */
	RTE_SET_USED(unused_param);
	return 0;
}

/* Function */
// ajay : unhandled fsm saegw - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_SNT_STATE PFCP_SESS_SET_DEL_RESP_RCVD_EVNT ==> process_pfcp_sess_set_del_rsp
// ajay - unhandled fsm pgw - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_SNT_STATE PFCP_SESS_SET_DEL_RESP_RCVD_EVNT ==> process_pfcp_sess_set_del_rsp 
// ajay unhandled fsm sgw - RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_SNT_STATE PFCP_SESS_SET_DEL_RESP_RCVD_EVNT process_pfcp_sess_set_del_rsp
int process_pfcp_sess_set_del_rsp(void *data, void *unused_param)
{
#ifdef USE_CSID
	int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pfcp_sess_set_del_rsp_t(&msg->pfcp_msg.pfcp_sess_set_del_rsp);
	if (ret) {
			clLog(sxlogger, eCLSeverityCritical, FORMAT"Error: %d \n",
					ERR_MSG, ret);
			return -1;
	}
#else
	RTE_SET_USED(data);
#endif /* USE_CSID */
	RTE_SET_USED(unused_param);
	return 0;
}

int
process_default_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;

	clLog(clSystemLog, eCLSeverityCritical, "%s:%d:SM_ERROR: No handler found for UE_Proc: %u UE_State: %u UE_event"
			"%u and Message_Type:%s\n", __func__, __LINE__,
			msg->proc, msg->state,msg->event,
			gtp_type_str(msg->msg_type));

	RTE_SET_USED(unused_param);
	return 0;
}



