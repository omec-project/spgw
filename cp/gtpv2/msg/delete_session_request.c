// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_decoder.h"
#include "gtp_messages.h"
#include "vepc_cp_dp_api.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "sm_struct.h"
#include "spgw_config_struct.h"
#include "cp_log.h"
#include "ip_pool.h"
#include "cp_peer.h"
#include "spgw_cpp_wrapper.h"
#include "sm_structs_api.h"
#include "gtpv2_error_rsp.h"
#include "proc_detach.h"
#include "util.h"
#include "gtpv2_session.h"
#include "proc.h"

/**
 * @brief  : Handles the removal of data structures internal to the control plane
 *           as well as notifying the data plane of such changes.
 * @param  : ds_req
 *           structure containing create delete session request
 * @param  : _context
 *           returns the UE context structure pertaining to the session to be deleted
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
int
delete_context(gtp_eps_bearer_id_ie_t lbi, uint32_t teid,
	ue_context_t **_context, uint32_t *s5s8_pgw_gtpc_teid,
	uint32_t *s5s8_pgw_gtpc_ipv4)
{
	ue_context_t *context = NULL;

	context = (ue_context_t *)get_ue_context(teid);

	if (context == NULL) {
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
    }


	if (!lbi.header.len) {
		/* TODO: should be responding with response indicating error
		 * in request */
		LOG_MSG(LOG_ERROR,
			"Received delete session without ebi! - dropping");
		return GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT;
	}

	uint8_t ebi_index = lbi.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR,
		    "Received delete session on non-existent EBI - "
		    "Dropping packet");
		LOG_MSG(LOG_ERROR, "ebi %u", lbi.ebi_ebi);
		LOG_MSG(LOG_ERROR, "ebi_index %u", ebi_index);
		LOG_MSG(LOG_ERROR, "bearer_bitmap %04x", context->bearer_bitmap);
		LOG_MSG(LOG_ERROR, "mask %04x", (1 << ebi_index));
		return GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT;
	}

	pdn_connection_t *pdn = context->eps_bearers[ebi_index]->pdn;
	if (!pdn) {
		LOG_MSG(LOG_ERROR, "Received delete session on "
				"non-existent EBI");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	if (pdn->default_bearer_id != lbi.ebi_ebi) {
		LOG_MSG(LOG_ERROR,
		    "Received delete session referencing incorrect "
		    "default bearer ebi");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	eps_bearer_t *bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		LOG_MSG(LOG_ERROR,
			"Received delete session on non-existent default EBI");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	if (IF_PDN_ADDR_STATIC(pdn)) { /* BUG : dup release ?*/
		struct in_addr host = {0};
		host.s_addr = pdn->ipv4.s_addr;
		release_ip_node(host); 
	}

	if (cp_config->cp_type == SGWC) {
		/*VS: Fill teid and ip address */
		*s5s8_pgw_gtpc_teid = htonl(pdn->s5s8_pgw_gtpc_teid);
		*s5s8_pgw_gtpc_ipv4 = htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);

		LOG_MSG(LOG_DEBUG, "s5s8_pgw_gtpc_teid:%u, s5s8_pgw_gtpc_ipv4:%u",
				*s5s8_pgw_gtpc_teid, *s5s8_pgw_gtpc_ipv4);
	}

#ifdef DELETE_THIS
	int i;
	for (i = 0; i < MAX_BEARERS; ++i) {
		if (pdn->eps_bearers[i] == NULL)
			continue;

		if (context->eps_bearers[i] == pdn->eps_bearers[i]) {
			bearer = context->eps_bearers[i];
			struct session_info si;
			memset(&si, 0, sizeof(si));

			/**
			 * ebi and s1u_sgw_teid is set here for zmq/sdn
			 */
			si.bearer_id = lbi.ebi_ebi;
			si.ue_addr.u.ipv4_addr =
				htonl(pdn->ipv4.s_addr);
			si.ul_s1_info.sgw_teid =
				bearer->s1u_sgw_gtpu_teid;
			si.sess_id = SESS_ID(
					context->s11_sgw_gtpc_teid,
					si.bearer_id);
		} else {
            assert(0);
		}
	}
#endif
	*_context = context;
	return 0;
}

#ifdef FUTURE_NEED
int
process_delete_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_s11_tx, gtpv2c_header_t *gtpv2c_s5s8_tx)
{
	int ret;
	ue_context_t *context = NULL;
	uint32_t s5s8_pgw_gtpc_teid = 0;
	uint32_t s5s8_pgw_gtpc_ipv4 = 0;
	del_sess_req_t ds_req = {0};

	decode_del_sess_req((uint8_t *) gtpv2c_rx, &ds_req);

	if (cp_config->cp_type == SGWC) {
		pdn_connection_t *pdn = NULL;
		uint32_t s5s8_pgw_gtpc_del_teid;
		static uint32_t process_sgwc_s5s8_ds_req_cnt;

		/* s11_sgw_gtpc_teid= key->ue_context_by_fteid_hash */
		context = (ue_context_t *)get_ue_context(ds_req.header.teid.has_teid.teid);

		if (context == NULL) {
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
        }

		uint8_t del_ebi_index = ds_req.lbi.ebi_ebi - 5;
		pdn = context->pdns[del_ebi_index];
		/* s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid =
		 * key->ue_context_by_fteid_hash */
		s5s8_pgw_gtpc_del_teid = ntohl(pdn->s5s8_pgw_gtpc_teid);

		ret =
			gen_sgwc_s5s8_delete_session_request(gtpv2c_rx,
				gtpv2c_s5s8_tx, s5s8_pgw_gtpc_del_teid,
				gtpv2c_rx->teid.has_teid.seq, ds_req.lbi.ebi_ebi);
		LOG_MSG(LOG_DEBUG,
				"\n\tprocess_delete_session_request::case= %d;"
				"\n\tprocess_sgwc_s5s8_ds_req_cnt= %u;"
				"\n\tue_ip= pdn->ipv4= %s;"
				"\n\tpdn->s5s8_sgw_gtpc_ipv4= %s;"
				"\n\tpdn->s5s8_sgw_gtpc_teid= %X;"
				"\n\tpdn->s5s8_pgw_gtpc_ipv4= %s;"
				"\n\tpdn->s5s8_pgw_gtpc_teid= %X;"
				"\n\tgen_delete_s5s8_session_request= %d",
				cp_config->cp_type, process_sgwc_s5s8_ds_req_cnt++,
				inet_ntoa(pdn->ipv4),
				inet_ntoa(pdn->s5s8_sgw_gtpc_ipv4),
				pdn->s5s8_sgw_gtpc_teid,
				inet_ntoa(pdn->s5s8_pgw_gtpc_ipv4),
				pdn->s5s8_pgw_gtpc_teid,
				ret);
		return ret;
	}

	gtpv2c_s11_tx->teid.has_teid.seq = gtpv2c_rx->teid.has_teid.seq;

	/* Lookup and get context of delete request */
	ret = delete_context(ds_req.lbi, ds_req.header.teid.has_teid.teid,
		&context, &s5s8_pgw_gtpc_teid, &s5s8_pgw_gtpc_ipv4);
	if (ret)
		return ret;

	set_gtpv2c_teid_header(gtpv2c_s11_tx, GTP_DELETE_SESSION_RSP,
	    htonl(context->s11_mme_gtpc_teid), gtpv2c_rx->teid.has_teid.seq);
	set_cause_accepted_ie(gtpv2c_s11_tx, IE_INSTANCE_ZERO);

	return 0;
}
#endif

int
handle_delete_session_request(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    int ret;
    struct sockaddr_in *peer_addr;
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;
    uint8_t ebi_index;
	eps_bearer_t *bearer  = NULL;

    peer_addr = &msg->peer_addr;

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    /* Decode delete session request */
    ret = decode_del_sess_req((uint8_t *) gtpv2c_rx,
            &msg->rx_msg.dsr);
    if (ret == 0)
        return -1;

    /* validate DSReq conente */

    // update cli sstats 
    assert(msg->msg_type == GTP_DELETE_SESSION_REQ);

    /* Find old transaction */
    uint32_t source_addr = msg->peer_addr.sin_addr.s_addr;
    uint16_t source_port = msg->peer_addr.sin_port;
    uint32_t seq_num = msg->rx_msg.dsr.header.teid.has_teid.seq;  
    transData_t *old_trans = (transData_t*)find_gtp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL) {
        LOG_MSG(LOG_ERROR, "Retransmitted DSReq received. Old DSReq is in progress");
        return -1;
    }

    context = (ue_context_t *)get_ue_context(msg->rx_msg.dsr.header.teid.has_teid.teid);
    if(context == NULL) {
        ds_error_response(NULL, msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        return -1;
    }
    msg->ue_context = context;
    ebi_index = msg->rx_msg.dsr.lbi.ebi_ebi - 5;

	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		LOG_MSG(LOG_ERROR, "Received delete session on non-existent EBI - "
				"Dropping packet");
		return -1;
	}

	bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		LOG_MSG(LOG_ERROR,
				"Received delete session on non-existent EBI - "
				"Bitmap Inconsistency - Dropping packet");
		return -1;
	}

    pdn = GET_PDN(context, ebi_index);
    assert(pdn == bearer->pdn);

    msg->pdn_context = pdn;
    msg->event = DS_REQ_RCVD_EVNT;

    if((msg->rx_msg.dsr.indctn_flgs.header.len !=0)  && 
            (msg->rx_msg.dsr.indctn_flgs.indication_si != 0)) {
        msg->proc = SGW_RELOCATION_DETACH_PROC;
    } else {
        msg->proc = DETACH_PROC;
    } 
    /* Allocate new Proc */
    proc_context_t *detach_proc = alloc_detach_proc(msg);

    /* Create new transaction */
    transData_t *gtpc_trans = (transData_t *) calloc(1, sizeof(transData_t));  
    RESET_TRANS_SELF_INITIATED(gtpc_trans);
    add_gtp_transaction(source_addr, source_port, seq_num, gtpc_trans);
    gtpc_trans->proc_context = (void *)detach_proc;
    detach_proc->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = seq_num;
    gtpc_trans->peer_sockaddr = msg->peer_addr;
    start_procedure(detach_proc);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;
    return 0;
}

#ifdef FUTURE_NEED
void process_pgwc_s5s8_delete_session_request_pfcp_timeout(void *data)
{
    return;
}


int
process_pgwc_s5s8_delete_session_request(del_sess_req_t *ds_req)
{
	struct gw_info _resp = {0};
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	int ebi_index = 0;
	int ret = delete_pgwc_context(ds_req, &context, &_resp);

	if (ret)
		return ret;

	pdn = GET_PDN(context , ebi_index);

	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
	fill_pfcp_sess_del_req(&pfcp_sess_del_req);

	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = _resp.seid;
	ebi_index =  UE_BEAR_ID(pfcp_sess_del_req.header.seid_seqno.has_seid.seid) -5 ;

	uint8_t pfcp_msg[512]={0};

	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg,encoded, &context->upf_context->upf_sockaddr);

    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_pgwc_s5s8_delete_session_request_pfcp_timeout);
    pdn->trans_entry = trans_entry;

	/* Update UE State */
	pdn->state = PFCP_SESS_DEL_REQ_SNT_STATE;

	/* VS: Stored/Update the session information. */
	if (get_sess_entry(_resp.seid, &resp) != 0) {
		LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH");
		return -1;
	}

	/* Store s11 struture data into sm_hash for sending delete response back to s11 */
	resp->eps_bearer_id = _resp.eps_bearer_id;
	resp->s5s8_pgw_gtpc_ipv4 = _resp.s5s8_pgw_gtpc_ipv4;
	resp->msg_type = GTP_DELETE_SESSION_REQ;
	resp->state = PFCP_SESS_DEL_REQ_SNT_STATE;

	return 0;
}

void process_sgwc_s5s8_delete_session_request_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p ", data);
    return;
}


void
fill_ds_request(del_sess_req_t *ds_req, ue_context_t *context,
		 uint8_t ebi_index)
{
	int len = 0;
	set_gtpv2c_header(&ds_req->header, 1,
			GTP_DELETE_SESSION_REQ, context->pdns[ebi_index]->s5s8_pgw_gtpc_teid,
			context->sequence);

	set_ie_header(&ds_req->lbi.header, GTP_IE_EPS_BEARER_ID,
			IE_INSTANCE_ZERO, sizeof(uint8_t));

	set_ebi(&ds_req->lbi, IE_INSTANCE_ZERO,
			context->eps_bearers[ebi_index]->eps_bearer_id);

	if (context->uli.lai) {
		ds_req->uli.lai = context->uli.lai;
		ds_req->uli.lai2.lai_mcc_digit_2 = context->uli.lai2.lai_mcc_digit_2;
		ds_req->uli.lai2.lai_mcc_digit_1 = context->uli.lai2.lai_mcc_digit_1;
		ds_req->uli.lai2.lai_mnc_digit_3 = context->uli.lai2.lai_mnc_digit_3;
		ds_req->uli.lai2.lai_mcc_digit_3 = context->uli.lai2.lai_mcc_digit_3;
		ds_req->uli.lai2.lai_mnc_digit_2 = context->uli.lai2.lai_mnc_digit_2;
		ds_req->uli.lai2.lai_mnc_digit_1 = context->uli.lai2.lai_mnc_digit_1;
		ds_req->uli.lai2.lai_lac = context->uli.lai2.lai_lac;
		len += sizeof(ds_req->uli.lai2);
	}
	if (context->uli.tai) {
		ds_req->uli.tai = context->uli.tai;
		ds_req->uli.tai2.tai_mcc_digit_2 = context->uli.tai2.tai_mcc_digit_2;
		ds_req->uli.tai2.tai_mcc_digit_1 = context->uli.tai2.tai_mcc_digit_1;
		ds_req->uli.tai2.tai_mnc_digit_3 = context->uli.tai2.tai_mnc_digit_3;
		ds_req->uli.tai2.tai_mcc_digit_3 = context->uli.tai2.tai_mcc_digit_3;
		ds_req->uli.tai2.tai_mnc_digit_2 = context->uli.tai2.tai_mnc_digit_2;
		ds_req->uli.tai2.tai_mnc_digit_1 = context->uli.tai2.tai_mnc_digit_1;
		ds_req->uli.tai2.tai_tac = context->uli.tai2.tai_tac;
		len += sizeof(ds_req->uli.tai2);
	}
	if (context->uli.rai) {
		ds_req->uli.rai = context->uli.rai;
		ds_req->uli.rai2.ria_mcc_digit_2 = context->uli.rai2.ria_mcc_digit_2;
		ds_req->uli.rai2.ria_mcc_digit_1 = context->uli.rai2.ria_mcc_digit_1;
		ds_req->uli.rai2.ria_mnc_digit_3 = context->uli.rai2.ria_mnc_digit_3;
		ds_req->uli.rai2.ria_mcc_digit_3 = context->uli.rai2.ria_mcc_digit_3;
		ds_req->uli.rai2.ria_mnc_digit_2 = context->uli.rai2.ria_mnc_digit_2;
		ds_req->uli.rai2.ria_mnc_digit_1 = context->uli.rai2.ria_mnc_digit_1;
		ds_req->uli.rai2.ria_lac = context->uli.rai2.ria_lac;
		ds_req->uli.rai2.ria_rac = context->uli.rai2.ria_rac;
		len += sizeof(ds_req->uli.rai2);
	}
	if (context->uli.sai) {
		ds_req->uli.sai = context->uli.sai;
		ds_req->uli.sai2.sai_mcc_digit_2 = context->uli.sai2.sai_mcc_digit_2;
		ds_req->uli.sai2.sai_mcc_digit_1 = context->uli.sai2.sai_mcc_digit_1;
		ds_req->uli.sai2.sai_mnc_digit_3 = context->uli.sai2.sai_mnc_digit_3;
		ds_req->uli.sai2.sai_mcc_digit_3 = context->uli.sai2.sai_mcc_digit_3;
		ds_req->uli.sai2.sai_mnc_digit_2 = context->uli.sai2.sai_mnc_digit_2;
		ds_req->uli.sai2.sai_mnc_digit_1 = context->uli.sai2.sai_mnc_digit_1;
		ds_req->uli.sai2.sai_lac = context->uli.sai2.sai_lac;
		ds_req->uli.sai2.sai_sac = context->uli.sai2.sai_sac;
		len += sizeof(ds_req->uli.sai2);
	}
	if (context->uli.cgi) {
		ds_req->uli.cgi = context->uli.cgi;
		ds_req->uli.cgi2.cgi_mcc_digit_2 = context->uli.cgi2.cgi_mcc_digit_2;
		ds_req->uli.cgi2.cgi_mcc_digit_1 = context->uli.cgi2.cgi_mcc_digit_1;
		ds_req->uli.cgi2.cgi_mnc_digit_3 = context->uli.cgi2.cgi_mnc_digit_3;
		ds_req->uli.cgi2.cgi_mcc_digit_3 = context->uli.cgi2.cgi_mcc_digit_3;
		ds_req->uli.cgi2.cgi_mnc_digit_2 = context->uli.cgi2.cgi_mnc_digit_2;
		ds_req->uli.cgi2.cgi_mnc_digit_1 = context->uli.cgi2.cgi_mnc_digit_1;
		ds_req->uli.cgi2.cgi_lac = context->uli.cgi2.cgi_lac;
	    ds_req->uli.cgi2.cgi_ci = context->uli.cgi2.cgi_ci;
		len += sizeof(ds_req->uli.cgi2);
	}
	if (context->uli.ecgi) {
		ds_req->uli.ecgi = context->uli.ecgi;
		ds_req->uli.ecgi2.ecgi_mcc_digit_2 = context->uli.ecgi2.ecgi_mcc_digit_2;
		ds_req->uli.ecgi2.ecgi_mcc_digit_1 = context->uli.ecgi2.ecgi_mcc_digit_1;
		ds_req->uli.ecgi2.ecgi_mnc_digit_3 = context->uli.ecgi2.ecgi_mnc_digit_3;
		ds_req->uli.ecgi2.ecgi_mcc_digit_3 = context->uli.ecgi2.ecgi_mcc_digit_3;
		ds_req->uli.ecgi2.ecgi_mnc_digit_2 = context->uli.ecgi2.ecgi_mnc_digit_2;
		ds_req->uli.ecgi2.ecgi_mnc_digit_1 = context->uli.ecgi2.ecgi_mnc_digit_1;
		ds_req->uli.ecgi2.ecgi_spare = context->uli.ecgi2.ecgi_spare;
	    ds_req->uli.ecgi2.eci = context->uli.ecgi2.eci;
		len += sizeof(ds_req->uli.ecgi2);
	}
	if (context->uli.macro_enodeb_id) {
		ds_req->uli.macro_enodeb_id = context->uli.macro_enodeb_id;
		ds_req->uli.macro_enodeb_id2.menbid_mcc_digit_2 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_2;
		ds_req->uli.macro_enodeb_id2.menbid_mcc_digit_1 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_1;
		ds_req->uli.macro_enodeb_id2.menbid_mnc_digit_3 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_3;
		ds_req->uli.macro_enodeb_id2.menbid_mcc_digit_3 =
			context->uli.macro_enodeb_id2.menbid_mcc_digit_3;
		ds_req->uli.macro_enodeb_id2.menbid_mnc_digit_2 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_2;
		ds_req->uli.macro_enodeb_id2.menbid_mnc_digit_1 =
			context->uli.macro_enodeb_id2.menbid_mnc_digit_1;
		ds_req->uli.macro_enodeb_id2.menbid_spare =
			context->uli.macro_enodeb_id2.menbid_spare;
		ds_req->uli.macro_enodeb_id2.menbid_macro_enodeb_id =
			context->uli.macro_enodeb_id2.menbid_macro_enodeb_id;
		ds_req->uli.macro_enodeb_id2.menbid_macro_enb_id2 =
			context->uli.macro_enodeb_id2.menbid_macro_enb_id2;
		len += sizeof(ds_req->uli.macro_enodeb_id2);
	}
	if (context->uli.extnded_macro_enb_id) {
		ds_req->uli.extnded_macro_enb_id = context->uli.extnded_macro_enb_id;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_1 =
			context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_1;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_3 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_3;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_3 =
			context->uli.extended_macro_enodeb_id2.emenbid_mcc_digit_3;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_2 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_2;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_1 =
			context->uli.extended_macro_enodeb_id2.emenbid_mnc_digit_1;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_smenb =
			context->uli.extended_macro_enodeb_id2.emenbid_smenb;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_spare =
			context->uli.extended_macro_enodeb_id2.emenbid_spare;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id =
			context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id;
		ds_req->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2 =
			context->uli.extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2;
		len += sizeof(ds_req->uli.extended_macro_enodeb_id2);
	}
	len += 1;
	set_ie_header(&ds_req->uli.header, GTP_IE_USER_LOC_INFO, IE_INSTANCE_ZERO,
		len);
}

#endif
#ifdef FUTURE_NEED
int
gen_sgwc_s5s8_delete_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx, uint32_t pgw_gtpc_del_teid,
		uint32_t sequence, uint8_t del_ebi)
{

	gtpv2c_ie *current_rx_ie;
	gtpv2c_ie *limit_rx_ie;

	set_gtpv2c_teid_header(gtpv2c_tx, GTP_DELETE_SESSION_REQ,
		    pgw_gtpc_del_teid, sequence);

	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_rx_ie, limit_rx_ie)
	{
		if (current_rx_ie->type == GTP_IE_EPS_BEARER_ID &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ebi_ie(gtpv2c_tx, IE_INSTANCE_ZERO, del_ebi);
		} else if (current_rx_ie->type == GTP_IE_USER_LOC_INFO &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		} else if (current_rx_ie->type == GTP_IE_INDICATION &&
				current_rx_ie->instance == IE_INSTANCE_ZERO) {
			set_ie_copy(gtpv2c_tx, current_rx_ie);
		}
	}

	return 0;
}

/**
 * @brief  : Maintans gateway information
 */
struct gw_info {
	uint8_t eps_bearer_id;
	uint32_t s5s8_sgw_gtpc_teid;
	uint32_t s5s8_pgw_gtpc_ipv4;
	uint64_t seid;  /*NK: used to retrive seid */
};

/**
 * @brief  : Parses delete session request message and handles the removal of
 *           corresponding data structures internal to the control plane - as well as
 *           notifying the data plane of such changes
 * @param  : gtpv2c_rx
 *           buffer containing create delete session request message
 * @param  : _context
 *           returns the UE context structure pertaining to the session to be deleted
 * @param  : del_teid_ptr
 *           returns pointer to s5s8_sgw_gtpc_teid to be deleted
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
//static int
//delete_pgwc_context(gtpv2c_header *gtpv2c_rx, ue_context_t **_context,
//		struct gw_info *resp)
//{
//	int ret = 0, i = 0;
//	gtpv2c_ie *current_ie;
//	gtpv2c_ie *limit_ie;
//	ue_context_t *context = NULL;
//	gtpv2c_ie *ebi_ei_to_be_removed = NULL;
//	static uint32_t process_pgwc_s5s8_ds_req_cnt;
//
//	//gtpv2c_rx->teid_u.has_teid.teid = ntohl(gtpv2c_rx->teid_u.has_teid.teid);
//	/* s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid =
//	 * key->ue_context_by_fteid_hash */
//	ret = r_t_e_hash_lookup_data(ue_context_by_fteid_hash,
//	    (const void *) &gtpv2c_rx->teid_u.has_teid.teid,
//	    (void **) &context);
//	if (ret < 0 || !context) {
//
//		LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
//				"\n\tprocess_pgwc_s5s8_delete_session_request:"
//				"\n\tdelete_pgwc_context-ERROR!!!"
//				"\n\tprocess_pgwc_s5s8_ds_req_cnt= %u;"
//				"\n\tgtpv2c_s5s8_rx->teid_u.has_teid.teid= %X;"
//					"ue_context_by_fteid_hash,..)= %d",
//				process_pgwc_s5s8_ds_req_cnt++,
//				gtpv2c_rx->teid_u.has_teid.teid,
//				ret);
//		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
//	}
//
//	/** TODO: we should verify mandatory fields within received message */
//	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_ie, limit_ie)
//	{
//		switch (current_ie->type) {
//		case GTP_IE_EPS_BEARER_ID:
//			if (current_ie->instance == IE_INSTANCE_ZERO)
//				ebi_ei_to_be_removed = current_ie;
//			break;
//		}
//	}
//
//	if (!ebi_ei_to_be_removed) {
//		/* TODO: should be responding with response indicating error
//		 * in request */
//		LOG_MSG(LOG_ERROR, "Received delete session without ebi! - "
//				"dropping");
//		return -EPERM;
//	}
//
//	uint8_t ebi = *IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t,
//			ebi_ei_to_be_removed);
//
//	/* VS: Fill the eps bearer id in response */
//	resp->eps_bearer_id = ebi;
//
//	uint8_t ebi_index = ebi - 5;
//	if (!(context->bearer_bitmap & (1 << ebi_index))) {
//		LOG_MSG(LOG_ERROR,
//		    "Received delete session on non-existent EBI - "
//		    "Dropping packet");
//		LOG_MSG(LOG_ERROR, "ebi %u",
//		    *IE_TYPE_PTR_FROM_GTPV2C_IE(uint8_t, ebi_ei_to_be_removed));
//		LOG_MSG(LOG_ERROR, "ebi_index %u", ebi_index);
//		LOG_MSG(LOG_ERROR, "bearer_bitmap %04x", context->bearer_bitmap);
//		LOG_MSG(LOG_ERROR, "mask %04x", (1 << ebi_index));
//		return -EPERM;
//	}
//
//	pdn_connection_t *pdn = context->pdns[ebi_index];
//	resp->seid = context->pdns[ebi_index]->seid;  //NK:change for seid
//	if (!pdn) {
//		LOG_MSG(LOG_ERROR, "Received delete session on "
//				"non-existent EBI");
//		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
//	}
//
//	if (pdn->default_bearer_id != ebi) {
//		LOG_MSG(LOG_ERROR,
//		    "Received delete session referencing incorrect "
//		    "default bearer ebi");
//		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
//	}
//	/* s11_sgw_gtpc_teid= s5s8_sgw_gtpc_teid =
//	 * key->ue_context_by_fteid_hash */
//	resp->s5s8_sgw_gtpc_teid = pdn->s5s8_sgw_gtpc_teid;
//	resp->s5s8_pgw_gtpc_ipv4 = pdn->s5s8_sgw_gtpc_ipv4.s_addr;
//
//	LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
//			"\n\tdelete_pgwc_context(...);"
//			"\n\tprocess_pgwc_s5s8_ds_req_cnt= %u;"
//			"\n\tue_ip= pdn->ipv4= %s;"
//			"\n\tpdn->s5s8_sgw_gtpc_ipv4= %s;"
//			"\n\tpdn->s5s8_sgw_gtpc_teid= %X;"
//			"\n\tpdn->s5s8_pgw_gtpc_ipv4= %s;"
//			"\n\tpdn->s5s8_pgw_gtpc_teid= %X;"
//				"ue_context_by_fteid_hash,..)= %d",
//			process_pgwc_s5s8_ds_req_cnt++,
//			inet_ntoa(pdn->ipv4),
//			inet_ntoa(pdn->s5s8_sgw_gtpc_ipv4),
//			pdn->s5s8_sgw_gtpc_teid,
//			inet_ntoa(pdn->s5s8_pgw_gtpc_ipv4),
//			pdn->s5s8_pgw_gtpc_teid,
//			ret);
//
//	eps_bearer_t *bearer = context->eps_bearers[ebi_index];
//	if (!bearer) {
//		LOG_MSG(LOG_ERROR, "Received delete session on non-existent "
//				"default EBI");
//		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
//	}
//
//	for (i = 0; i < MAX_BEARERS; ++i) {
//		if (pdn->eps_bearers[i] == NULL)
//			continue;
//
//		if (context->eps_bearers[i] == pdn->eps_bearers[i]) {
//			bearer = context->eps_bearers[i];
//			struct session_info si;
//			memset(&si, 0, sizeof(si));
//
//			/**
//			 * ebi and s1u_sgw_teid is set here for zmq/sdn
//			 */
//			si.bearer_id = ebi;
//			si.ue_addr.u.ipv4_addr =
//				htonl(pdn->ipv4.s_addr);
//			si.ul_s1_info.sgw_teid =
//				bearer->s1u_sgw_gtpu_teid;
//			si.sess_id = SESS_ID(
//					context->s11_sgw_gtpc_teid,
//					si.bearer_id);
//			/*
//			struct dp_id dp_id = { .id = DPN_ID };
//			session_delete(dp_id, si);
//			*/
//
//			free(pdn->eps_bearers[i]);
//			pdn->eps_bearers[i] = NULL;
//			context->eps_bearers[i] = NULL;
//			context->bearer_bitmap &= ~(1 << i);
//		} else {
//          assert(0);
//		}
//	}
//	--context->num_pdns;
//	free(pdn);
//	context->pdns[ebi_index] = NULL;
//	context->teid_bitmap = 0;
//
//	*_context = context;
//	return 0;
//}
//
//int
//process_pgwc_s5s8_delete_session_request(gtpv2c_header *gtpv2c_rx)
//{
//	struct gw_info _resp = {0};
//	ue_context_t *context = NULL;
//	struct resp_info *resp = NULL;
//
//	int ret = delete_pgwc_context(gtpv2c_rx, &context, &_resp);
//	if (ret)
//	return ret;
//
//	pfcp_sess_del_req_t pfcp_sess_del_req = {0};
//	fill_pfcp_sess_del_req(&pfcp_sess_del_req);
//	pfcp_sess_del_req.header.seid_seqno.has_seid.seid = _resp.seid;
//
//	pfcp_sess_del_req.header.seid_seqno.has_seid.seq_no =
//						(htonl(gtpv2c_rx->teid_u.has_teid.seq) >> 8);
//
//	uint8_t pfcp_msg[512]={0};
//
//	int encoded = encode_pfcp_sess_del_req_t(&pfcp_sess_del_req, pfcp_msg);
//	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
//	header->message_len = htons(encoded - 4);
//
//	if (pfcp_send(pfcp_fd, pfcp_msg,encoded,
//				&context->upf_context.upf_sockaddr) < 0 )
//		LOG_MSG(LOG_DEBUG,"Error sending: %i",errno);
//	else {
//	}
//
//	/* Update the sequence number */
//	context->sequence =
//		gtpv2c_rx->teid_u.has_teid.seq;
//
//	/* Update UE State */
//	context->state = PFCP_SESS_DEL_REQ_SNT_STATE;
//
//	/* VS: Stored/Update the session information. */
//	resp = get_sess_entry_seid(_resp.seid);
//	if (resp == NULL) {
//		LOG_MSG(LOG_ERROR, "Failed to add response in entry in SM_HASH");
//		return -1;
//	}
//
//	/* Store s11 struture data into sm_hash for sending delete response back to s11 */
//	resp->eps_bearer_id = _resp.eps_bearer_id;
//	resp->s5s8_sgw_gtpc_teid = _resp.s5s8_sgw_gtpc_teid;
//	resp->s5s8_pgw_gtpc_ipv4 = htonl(_resp.s5s8_pgw_gtpc_ipv4);
//	resp->msg_type = GTP_DELETE_SESSION_REQ;
//	resp->state = PFCP_SESS_DEL_REQ_SNT_STATE;
//	resp->proc = context->proc;
//
//	return 0;
//}
//
///* SGWC S5S8 handlers:
// * static int delete_sgwc_context(...)
// * int process_sgwc_s5s8_delete_session_response(...)
// * int gen_sgwc_s5s8_delete_session_request(...)
// *
// */
//
///**
// * @brief  : Parses delete session request message and handles the removal of
// *           corresponding data structures internal to the control plane - as well as
// *           notifying the data plane of such changes
// * @param  : gtpv2c_rx
// *           buffer containing create delete session request message
// * @param  : _context
// *           returns the UE context structure pertaining to the session to be deleted
// * @return : - 0 if successful
// *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
// *             specified cause error value
// *           - < 0 for all other errors
// */
//static int
//delete_sgwc_context(gtpv2c_header *gtpv2c_rx, ue_context_t **_context, uint64_t *seid)
//{
//	int ret;
//	int i;
//	static uint32_t process_sgwc_s5s8_ds_rsp_cnt;
//	ue_context_t *context = NULL;
//
//	//gtpv2c_rx->teid_u.has_teid.teid = ntohl(gtpv2c_rx->teid_u.has_teid.teid);
//	/* s11_sgw_gtpc_teid= s5s8_sgw_gtpc_teid =
//	 * key->ue_context_by_fteid_hash */
//	ret = r_t_e_hash_lookup_data(ue_context_by_fteid_hash,
//	    (const void *) &gtpv2c_rx->teid_u.has_teid.teid,
//	    (void **) &context);
//	if (ret < 0 || !context) {
//
//		LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
//				"\n\tprocess_sgwc_s5s8_delete_session_request:"
//				"\n\tdelete_sgwc_context-ERROR!!!"
//				"\n\tprocess_sgwc_s5s8_ds_rep_cnt= %u;"
//				"\n\tgtpv2c_s5s8_rx->teid_u.has_teid.teid= %X;"
//					"ue_context_by_fteid_hash,..)= %d",
//				process_sgwc_s5s8_ds_rsp_cnt++,
//				gtpv2c_rx->teid_u.has_teid.teid,
//				ret);
//		LOG_MSG(LOG_DEBUG,"Conext not found");
//		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
//	}
//
//	LOG_MSG(LOG_DEBUG, "NGIC- delete_s5s8_session.c::"
//			"\n\tdelete_sgwc_context(...);"
//			"\n\tprocess_sgwc_s5s8_ds_rsp_cnt= %u;"
//			"\n\tgtpv2c_rx->teid_u.has_teid.teid= %X"
//				"ue_context_by_fteid_hash,..)= %d",
//			process_sgwc_s5s8_ds_rsp_cnt++,
//			gtpv2c_rx->teid_u.has_teid.teid,
//			ret);
//	pdn_connection_t *pdn_ctxt;
//
//	for (i = 0; i < MAX_BEARERS; ++i) {
//		if (context->pdns[i] == NULL) {
//			continue;
//		}
//
//		if (context->eps_bearers[i]) {
//			eps_bearer_t *bearer = context->eps_bearers[i];
//			pdn_ctxt = bearer->pdn;
//			struct session_info si;
//			memset(&si, 0, sizeof(si));
//
//			/**
//			 * ebi and s1u_sgw_teid is set here for zmq/sdn
//			 */
//			si.bearer_id = i + 5;
//			si.ue_addr.u.ipv4_addr =
//				htonl(pdn_ctxt->ipv4.s_addr);
//			si.ul_s1_info.sgw_teid =
//				bearer->s1u_sgw_gtpu_teid;
//			si.sess_id = SESS_ID(
//				context->s11_sgw_gtpc_teid,
//				si.bearer_id);
//			*seid = si.sess_id;
//
//			free(pdn_ctxt->eps_bearers[i]);
//			pdn_ctxt->eps_bearers[i] = NULL;
//			context->eps_bearers[i] = NULL;
//			context->bearer_bitmap &= ~(1 << i);
//			free(pdn_ctxt);
//		}
//	}
//	--context->num_pdns;
//	context->teid_bitmap = 0;
//
//	*_context = context;
//	return 0;
//}



#endif
