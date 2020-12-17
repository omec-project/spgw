// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "proc_bearer_create.h"
#include "sm_struct.h"
#include "gtp_ies.h"
#include "pfcp_cp_session.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_config.h"
#include "gx_interface.h"
#include "gtpv2_interface.h"
#include "spgw_cpp_wrapper.h"
#include "cp_io_poll.h"
#include "gtpv2_set_ie.h"
#include "cp_transactions.h"
#include "gtpv2_interface.h"
#include "byteswap.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "ipc_api.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "cp_log.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

void
process_pfcp_sess_mod_resp_cbr_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_pfcp_sess_mod_resp_cbr_handler(void *data, void *p)
{
    proc_context_t *proc_ctxt = (proc_context_t *)p;
	uint16_t payload_length = 0;
    int ret = 0;

	msg_info_t *msg = (msg_info_t *)data;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	ret = process_pfcp_sess_mod_resp_cbr(
			msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
			gtpv2c_tx);
	if (ret != 0) {
		if(ret != -1)
			/* TODO for cbr
			 * mbr_error_response(&msg->gtpc_msg.mbr, ret,
								cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE); */
		LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n",
				__func__, __LINE__, ret);
		return ret;
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

#if 0
	if (get_sess_entry_seid(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid,
																			&resp) != 0){
		LOG_MSG(LOG_ERROR, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
	if ((SAEGWC != cp_config->cp_type) && ((resp->msg_type == GTP_CREATE_BEARER_RSP) ||
			(resp->msg_type == GX_RAR_MSG))){
	    gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	            (struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));
		if(resp->msg_type != GTP_CREATE_BEARER_RSP){
			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&(my_sock.s5s8_recv_sockaddr), gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S5S8_IFACE);
		}
        // standalone sgw case 
		if (resp->msg_type == GTP_CREATE_BEARER_RSP) {

            increment_mme_peer_stats(MSG_RX_GTPV2_S11_CBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
		}
		else {
            // peer address needs to be corrected 
            increment_mme_peer_stats(MSG_TX_GTPV2_S11_CBRSP, my_sock.s5s8_recv_sockaddr.sin_addr.s_addr);
		}
	} else {
#endif
    ue_context_t *context = proc_ctxt->ue_context;
 	struct sockaddr_in s11_mme_sockaddr = {
		.sin_family = AF_INET,
		.sin_port = htons(GTPC_UDP_PORT),
		.sin_addr.s_addr = htonl(context->s11_mme_gtpc_ipv4.s_addr),
		.sin_zero = {0},
	};

//		if(resp->msg_type != GX_RAA_MSG) {
    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
            (struct sockaddr *) &s11_mme_sockaddr,
            sizeof(struct sockaddr_in));

    int sequence = gtpv2c_tx->teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;
    transData_t *gtpc_trans;
    gtpc_trans = start_response_wait_timer(proc_ctxt, (uint8_t *)gtp_tx_buf, 
                                            payload_length, 
                                            process_pfcp_sess_mod_resp_cbr_timeout);

    gtpc_trans->self_initiated = 1;
    gtpc_trans->proc_context = (void *)proc_ctxt;
    proc_ctxt->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = sequence;
    gtpc_trans->peer_sockaddr = s11_mme_sockaddr;
    add_gtp_transaction(local_addr, port_num, sequence, gtpc_trans);

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_DDNREQ, s11_mme_sockaddr.sin_addr.s_addr);



#if 0
			add_gtpv2c_if_timer_entry(
					UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid),
					&s11_mme_sockaddr, gtp_tx_buf, payload_length,
					UE_BEAR_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid) - 5,
					S11_IFACE);
#endif

     //       increment_mme_peer_stats(MSG_TX_GTPV2_S11_CBREQ, s11_mme_sockaddr.sin_addr.s_addr);
	//	}
	//}

	return 0;
}

uint8_t
process_pfcp_sess_mod_resp_cbr(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);
	uint32_t sequence = get_gtp_sequence(); 

	/* Retrive the session information based on session id. */
	if (get_sess_entry_seid(sess_id, &context) != 0){
		LOG_MSG(LOG_ERROR, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, sess_id);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Retrieve the UE context */
	ret = get_ue_context(teid, &context);
	if (ret < 0) {
			LOG_MSG(LOG_ERROR, "%s:%d Failed to update UE State for teid: %u\n",
					__func__, __LINE__,
					teid);
	}

	ebi_index = UE_BEAR_ID(sess_id) - 5;
	bearer = context->eps_bearers[ebi_index];
	/* Update the UE state */
	pdn = GET_PDN(context, ebi_index);
	pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	if (!bearer) {
		LOG_MSG(LOG_ERROR,
				"%s:%d Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

    uint8_t ebi = 0;
    get_bearer_info_install_rules(pdn, &ebi);
    bearer = context->eps_bearers[ebi];
    if (!bearer) {
        LOG_MSG(LOG_ERROR,
                "%s:%d Retrive modify bearer context but EBI is non-existent- "
                "Bitmap Inconsistency - Dropping packet\n", __func__, __LINE__);
        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }

    /* TODO: NC Need to remove hard coded pti value */
    set_create_bearer_request(gtpv2c_tx, sequence, context,
            bearer, pdn->default_bearer_id, 0, NULL, 0);

    //resp->state = CREATE_BER_REQ_SNT_STATE;
    pdn->state = CREATE_BER_REQ_SNT_STATE;

    return 0;

}

void
process_create_bearer_resp_and_send_raa( proc_context_t *proc)
{
	char *send_buf =  NULL;
	uint32_t buflen ;

	gx_msg *resp = malloc(sizeof(gx_msg));
	memset(resp, 0, sizeof(gx_msg));

	/* Filling Header value of RAA */
	resp->msg_type = GX_RAA_MSG ;
	//create_bearer_resp_t cbresp = {0};

	//fill_raa_msg( &(resp->data.cp_raa), &cbresp );
    pdn_connection_t *pdn = (pdn_connection_t *)proc->pdn_context;
    resp->data.cp_raa.presence.session_id = PRESENT;
	resp->data.cp_raa.session_id.len = strlen(pdn->gx_sess_id);
	memcpy(resp->data.cp_raa.session_id.val, pdn->gx_sess_id, resp->data.cp_raa.session_id.len);

	/* Result code */
	resp->data.cp_raa.result_code = 2001;
	resp->data.cp_raa.presence.result_code = PRESENT;

	/* Cal the length of buffer needed */
	buflen = gx_raa_calc_length (&resp->data.cp_raa);

	send_buf = malloc( buflen + sizeof(resp->msg_type)+sizeof(resp->seq_num));
	memset(send_buf, 0, buflen + sizeof(resp->msg_type)+sizeof(resp->seq_num));

	/* encoding the raa header value to buffer */
	memcpy(send_buf, &resp->msg_type, sizeof(resp->msg_type));
    memcpy(send_buf+sizeof(resp->msg_type), &proc->rar_seq_num, sizeof(resp->seq_num));
    
	if ( gx_raa_pack(&(resp->data.cp_raa), (unsigned char *)(send_buf + sizeof(resp->msg_type) + sizeof(resp->seq_num)), buflen ) == 0 )
    {
		LOG_MSG(LOG_DEBUG,"RAA Packing failure \n");
    }
    LOG_MSG(LOG_DEBUG, "RAA successfully sent ");

    gx_send(my_sock.gx_app_sock, send_buf, buflen + sizeof(resp->msg_type) + sizeof(resp->seq_num));
}

void process_sgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_sgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg)
{
	int ret;
    RTE_SET_USED(ret);
	uint8_t ebi_index;
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
    create_bearer_rsp_t *cb_rsp = &msg->gtpc_msg.cb_rsp;
    ue_context_t *context = proc->ue_context;

#if 0
	ret = get_ue_context(cb_rsp->header.teid.has_teid.teid, &context);
	if (ret) {
		LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}
#endif

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if(bearer == NULL)
	{
         LOG_MSG(LOG_ERROR, "Received CBRsp and  bearer not found. Ignore CBRsp");
		/* TODO:
		 * This mean ebi we allocated and received doesnt match
		 * In correct design match the bearer in transtient struct from sgw-u teid
		 * */
		return -1;
	}

	bearer->s1u_enb_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s1u_enb_fteid.ipv4_address;
	bearer->s1u_enb_gtpu_teid = cb_rsp->bearer_contexts.s1u_enb_fteid.teid_gre_key;
	bearer->s1u_sgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s1u_sgw_fteid.ipv4_address;
	bearer->s1u_sgw_gtpu_teid = cb_rsp->bearer_contexts.s1u_sgw_fteid.teid_gre_key;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cb_rsp->header.teid.has_teid.seq) ;
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	pfcp_sess_mod_req.create_pdr_count = 0;
	pfcp_sess_mod_req.update_far_count = 0;

	if (cb_rsp->bearer_contexts.s1u_enb_fteid.header.len  != 0) {
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s1u_enb_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s1u_enb_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(cb_rsp->bearer_contexts.s1u_enb_fteid.interface_type);
		update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		pfcp_sess_mod_req.update_far_count++;
	}

	uint32_t sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);


	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXA_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_sgwc_create_bearer_rsp_pfcp_timeout);
    trans_entry->self_initiated = 1;
    bearer->pdn->trans_entry = trans_entry; 
    proc->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc;

    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;


	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

#if 0
	if (get_sess_entry_seid(bearer->pdn->seid, &resp) != 0) {
		LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	resp->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	resp->msg_type = GTP_CREATE_BEARER_RSP;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
#endif

	return 0;
}


#ifdef FUTURE_NEED
void
process_pgwc_create_bearer_rsp_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_pgwc_create_bearer_rsp(proc_context_t *proc, msg_info_t *msg)
{
	uint8_t ret;
    RTE_SET_USED(ret);
	eps_bearer_t *bearer = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};
	uint8_t ebi_index;
    create_bearer_rsp_t *cb_rsp = &msg->gtpc_msg.cb_rsp;
    ue_context_t *context = proc->ue_context;

	ebi_index = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi - 5;

	bearer = context->eps_bearers[ebi_index];
	bearer->eps_bearer_id = cb_rsp->bearer_contexts.eps_bearer_id.ebi_ebi;
	if (NULL == bearer)
	{
        LOG_MSG(LOG_ERROR, "CBRsp received at PGW but bearer not found");
		/* TODO: Invalid ebi index handling */
		return -1;
	}

	bearer->s5s8_sgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s58_u_sgw_fteid.ipv4_address;
	bearer->s5s8_sgw_gtpu_teid = cb_rsp->bearer_contexts.s58_u_sgw_fteid.teid_gre_key;

	bearer->s5s8_pgw_gtpu_ipv4.s_addr = cb_rsp->bearer_contexts.s58_u_pgw_fteid.ipv4_address;
	bearer->s5s8_pgw_gtpu_teid = cb_rsp->bearer_contexts.s58_u_pgw_fteid.teid_gre_key;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cb_rsp->header.teid.has_teid.seq) ;
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	pfcp_sess_mod_req.create_pdr_count = 0;
	pfcp_sess_mod_req.update_far_count = 0;

#if 0
	if (cb_rsp->bearer_contexts.s58_u_sgw_fteid.header.len != 0) {
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.teid =
			bearer->s5s8_sgw_gtpu_teid;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			bearer->s5s8_sgw_gtpu_ipv4.s_addr;
		update_far[pfcp_sess_mod_req.update_far_count].upd_frwdng_parms.dst_intfc.interface_value =
			check_interface_type(cb_rsp->bearer_contexts.s58_u_sgw_fteid.interface_type);
		update_far[pfcp_sess_mod_req.update_far_count].apply_action.forw = PRESENT;
		pfcp_sess_mod_req.update_far_count++;
	}
#endif

	uint32_t sequence = fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cb_rsp->header, bearer, bearer->pdn, update_far, 0);

	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
    trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pgwc_create_bearer_rsp_pfcp_timeout);
    trans_entry->self_initiated = 1;
    bearer->pdn->trans_entry = trans_entry; 
    proc->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc;

    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, sequence, (void*)trans_entry);  
    trans_entry->sequence = sequence;

	//context->sequence = seq_no;
	bearer->pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	return 0;
}

int
process_cbresp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_pgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
		LOG_MSG(LOG_ERROR, "%s : Error: %d \n", __func__, ret);
		return ret;
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
			LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

int
process_create_bearer_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	ret = process_sgwc_create_bearer_rsp(&msg->gtpc_msg.cb_rsp);
	if (ret) {
			LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n",
					__func__, __LINE__, ret);
			return -1;
	}

	RTE_SET_USED(unused_param);
	return 0;
}

void 
process_create_bearer_request_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int
process_create_bearer_request(create_bearer_req_t *cbr)
{
	int ret;
	uint8_t ebi_index = 0;
	uint8_t new_ebi_index = 0;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	ret = get_ue_context_by_sgw_s5s8_teid(cbr->header.teid.has_teid.teid, &context);
	if (ret) {
		LOG_MSG(LOG_ERROR, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	bearer = rte_zmalloc_socket(NULL, sizeof(eps_bearer_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (bearer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate bearer "
				"structure: %s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
		return GTPV2C_CAUSE_SYSTEM_FAILURE;
	}

	ebi_index = cbr->lbi.ebi_ebi - 5;
	new_ebi_index = ++(context->pdns[ebi_index]->num_bearer) - 1;

	bearer->pdn = context->pdns[ebi_index];
	pdn = context->pdns[ebi_index];
	context->eps_bearers[new_ebi_index] = bearer;
	pdn->eps_bearers[new_ebi_index] = bearer;

	s11_mme_sockaddr.sin_addr.s_addr =
		context->s11_mme_gtpc_ipv4.s_addr;

	uint32_t  seq_no = 0;
	seq_no = bswap_32(cbr->header.teid.has_teid.seq);
	seq_no = seq_no >> 8;

	pfcp_update_far_ie_t update_far[MAX_LIST_SIZE];

	bearer->qos.arp.preemption_vulnerability = cbr->bearer_contexts.bearer_lvl_qos.pvi;
	bearer->qos.arp.priority_level = cbr->bearer_contexts.bearer_lvl_qos.pl;
	bearer->qos.arp.preemption_capability = cbr->bearer_contexts.bearer_lvl_qos.pci;
	bearer->qos.qci = cbr->bearer_contexts.bearer_lvl_qos.qci;
	bearer->qos.ul_mbr = cbr->bearer_contexts.bearer_lvl_qos.max_bit_rate_uplnk;
	bearer->qos.dl_mbr = cbr->bearer_contexts.bearer_lvl_qos.max_bit_rate_dnlnk;
	bearer->qos.ul_gbr = cbr->bearer_contexts.bearer_lvl_qos.guarntd_bit_rate_uplnk;
	bearer->qos.dl_gbr = cbr->bearer_contexts.bearer_lvl_qos.guarntd_bit_rate_dnlnk;

	bearer->s5s8_pgw_gtpu_ipv4.s_addr = cbr->bearer_contexts.s58_u_pgw_fteid.ipv4_address;
	bearer->s5s8_pgw_gtpu_teid = cbr->bearer_contexts.s58_u_pgw_fteid.teid_gre_key;

	fill_dedicated_bearer_info(bearer, context, pdn);

	pfcp_sess_mod_req.create_pdr_count = bearer->pdr_count;
	fill_pfcp_sess_mod_req(&pfcp_sess_mod_req, &cbr->header, bearer, pdn, update_far, 0);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_TX_PFCP_SXA_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_create_bearer_request_pfcp_timeout);
    pdn->trans_entry = trans_entry;

	context->sequence = seq_no;
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if (get_sess_entry_seid(context->pdns[ebi_index]->seid, &resp) != 0) {
		LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH\n");
		return -1;
	}

	memset(resp->eps_bearer_lvl_tft, 0, 257);
	memcpy(resp->eps_bearer_lvl_tft,
			cbr->bearer_contexts.tft.eps_bearer_lvl_tft,
			257);
	resp->tft_header_len = cbr->bearer_contexts.tft.header.len;
	resp->eps_bearer_id = new_ebi_index + 5;
	resp->msg_type = GTP_CREATE_BEARER_REQ;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
	resp->proc = DED_BER_ACTIVATION_PROC;
	pdn->proc = DED_BER_ACTIVATION_PROC;

	return 0;
}
#endif
