// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifdef FUTURE_NEED
/* Function */
int
process_pfcp_sess_set_del_req_t(pfcp_sess_set_del_req_t *del_set_req,
		gtpv2c_header_t *gtpv2c_tx)
{
	RTE_SET_USED(del_set_req);
	RTE_SET_USED(gtpv2c_tx);
	return 0;
}

// ajay - unhandled fsm 
// pgw RESTORATION_RECOVERY_PROC PFCP_SESS_SET_DEL_REQ_RCVD_STATE PFCP_SESS_SET_DEL_REQ_RCVD_EVNT ==> process_pfcp_sess_set_del_req 
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

/* Function */
int
process_pfcp_sess_set_del_rsp_t(pfcp_sess_set_del_rsp_t *del_set_rsp)
{
	if(del_set_rsp->cause.cause_value != REQUESTACCEPTED){
		clLog(clSystemLog, eCLSeverityCritical, FORMAT"ERROR:Cause received Session Set deletion response is %d\n",
				ERR_MSG, del_set_rsp->cause.cause_value);

		/* TODO: Add handling to send association to next upf
		 * for each buffered CSR */
		return -1;
	}
	return 0;
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
		            sizeof(struct sockaddr_in));

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
		        sizeof(struct sockaddr_in));
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
//		        sizeof(struct sockaddr_in));
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

#endif
