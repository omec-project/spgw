#ifdef FUTURE_NEED
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

int8_t
process_upd_pdn_conn_set_req_t(upd_pdn_conn_set_req_t *upd_pdn_req)
{
	RTE_SET_USED(upd_pdn_req);
	return 0;
}

int8_t
process_upd_pdn_conn_set_rsp_t(upd_pdn_conn_set_rsp_t *upd_pdn_rsp)
{
	RTE_SET_USED(upd_pdn_rsp);
	return 0;
}

#endif
