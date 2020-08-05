// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <rte_debug.h>
#include "gtp_messages_decoder.h"
#include "gtp_messages.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "gtpv2c_set_ie.h"
#include "gtpv2_interface.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "clogger.h"
#include "ip_pool.h"
#include "gw_adapter.h"
#include "cp_peer.h"
#include "spgw_cpp_wrapper.h"
#include "sm_structs_api.h"
#include "gtpv2c_error_rsp.h"
#include "detach_proc.h"

int
delete_context(gtp_eps_bearer_id_ie_t lbi, uint32_t teid,
	ue_context_t **_context, uint32_t *s5s8_pgw_gtpc_teid,
	uint32_t *s5s8_pgw_gtpc_ipv4);

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
	int ret;
	ue_context_t *context = NULL;

	ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
	    (const void *) &teid,
	    (void **) &context);

	if (ret < 0 || !context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;


	if (!lbi.header.len) {
		/* TODO: should be responding with response indicating error
		 * in request */
		clLog(sxlogger, eCLSeverityCritical,
			"%s : Received delete session without ebi! - dropping\n", __func__);
		return GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT;
	}

	uint8_t ebi_index = lbi.ebi_ebi - 5;
	if (!(context->bearer_bitmap & (1 << ebi_index))) {
		clLog(sxlogger, eCLSeverityCritical,
		    "Received delete session on non-existent EBI - "
		    "Dropping packet\n");
		clLog(sxlogger, eCLSeverityCritical, "ebi %u\n", lbi.ebi_ebi);
		clLog(sxlogger, eCLSeverityCritical, "ebi_index %u\n", ebi_index);
		clLog(sxlogger, eCLSeverityCritical, "bearer_bitmap %04x\n", context->bearer_bitmap);
		clLog(sxlogger, eCLSeverityCritical, "mask %04x\n", (1 << ebi_index));
		return GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT;
	}

	pdn_connection_t *pdn = context->eps_bearers[ebi_index]->pdn;
	if (!pdn) {
		clLog(sxlogger, eCLSeverityCritical, "Received delete session on "
				"non-existent EBI\n");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	if (pdn->default_bearer_id != lbi.ebi_ebi) {
		clLog(sxlogger, eCLSeverityCritical,
		    "Received delete session referencing incorrect "
		    "default bearer ebi");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

	eps_bearer_t *bearer = context->eps_bearers[ebi_index];
	if (!bearer) {
		clLog(sxlogger, eCLSeverityCritical,
			"Received delete session on non-existent default EBI\n");
		return GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
	}

#if 0
#ifdef STATIC_ADDR_ALLOC /* ajay TODO */
#ifdef MULTI_UPFS
	struct dp_info *dp = context->upf_context->dp_info; 
	if (dp != NULL && (IF_PDN_ADDR_STATIC(pdn))) {
		struct in_addr host = {0};
		host.s_addr = pdn->ipv4.s_addr;
		release_ip_node(dp->static_pool_tree, host);
	}
#else
	if (static_addr_pool != NULL && (IF_PDN_ADDR_STATIC(pdn))) {
		struct in_addr host = {0};
		host.s_addr = pdn->ipv4.s_addr;
		release_ip_node(static_addr_pool, host); 
	}
#endif
#endif
#endif

	if (cp_config->cp_type == SGWC) {
		/*VS: Fill teid and ip address */
		*s5s8_pgw_gtpc_teid = htonl(pdn->s5s8_pgw_gtpc_teid);
		*s5s8_pgw_gtpc_ipv4 = htonl(pdn->s5s8_pgw_gtpc_ipv4.s_addr);

		clLog(s5s8logger, eCLSeverityDebug, "s5s8_pgw_gtpc_teid:%u, s5s8_pgw_gtpc_ipv4:%u\n",
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
			rte_panic("Incorrect provisioning of bearers\n");
		}
	}
#endif
	*_context = context;
	return 0;
}

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
		ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
			(const void *) &ds_req.header.teid.has_teid.teid,
			(void **) &context);

		if (ret < 0 || !context)
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

		uint8_t del_ebi_index = ds_req.lbi.ebi_ebi - 5;
		pdn = context->pdns[del_ebi_index];
		/* s11_sgw_gtpc_teid = s5s8_pgw_gtpc_base_teid =
		 * key->ue_context_by_fteid_hash */
		s5s8_pgw_gtpc_del_teid = ntohl(pdn->s5s8_pgw_gtpc_teid);

		ret =
			gen_sgwc_s5s8_delete_session_request(gtpv2c_rx,
				gtpv2c_s5s8_tx, s5s8_pgw_gtpc_del_teid,
				gtpv2c_rx->teid.has_teid.seq, ds_req.lbi.ebi_ebi);
		clLog(s11logger, eCLSeverityDebug,
				"\n\tprocess_delete_session_request::case= %d;"
				"\n\tprocess_sgwc_s5s8_ds_req_cnt= %u;"
				"\n\tue_ip= pdn->ipv4= %s;"
				"\n\tpdn->s5s8_sgw_gtpc_ipv4= %s;"
				"\n\tpdn->s5s8_sgw_gtpc_teid= %X;"
				"\n\tpdn->s5s8_pgw_gtpc_ipv4= %s;"
				"\n\tpdn->s5s8_pgw_gtpc_teid= %X;"
				"\n\tgen_delete_s5s8_session_request= %d\n",
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

// saegw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT => process_ds_req_handler
// saegw - DETACH_PROC IDEL_STATE  DS_REQ_RCVD_EVNT => process_ds_req_handler
// pgw - DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler
// pgw - DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT ==> process_ds_req_handler 
// sgw DETACH_PROC CONNECTED_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 
// sgw DETACH_PROC IDEL_STATE DS_REQ_RCVD_EVNT : process_ds_req_handler 

int
handle_delete_session_request(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    int ret;
    struct sockaddr_in s11_peer_sockaddr = {0};
    s11_peer_sockaddr = msg->peer_addr;
    /* Reset periodic timers */
    process_response(s11_peer_sockaddr.sin_addr.s_addr);

    /* Decode delete session request */
    ret = decode_del_sess_req((uint8_t *) gtpv2c_rx,
            &msg->gtpc_msg.dsr);
    if (ret == 0)
        return -1;

    /* validate DSReq conente */

    // update cli sstats 
    RTE_SET_USED(gtpv2c_rx);
    ue_context_t *context = NULL;
    pdn_connection_t *pdn = NULL;
    uint8_t ebi_index;
    assert(msg->msg_type == GTP_DELETE_SESSION_REQ);

    /* Find old transaction */
    uint32_t source_addr = msg->peer_addr.sin_addr.s_addr;
    uint16_t source_port = msg->peer_addr.sin_port;
    uint32_t seq_num = msg->gtpc_msg.dsr.header.teid.has_teid.seq;  
    transData_t *old_trans = find_gtp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL)
    {
        printf("Retransmitted DSReq received. Old DSReq is in progress\n");
        return -1;
    }

    if(get_ue_context(msg->gtpc_msg.dsr.header.teid.has_teid.teid,
                &context) != 0) {
        ds_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        return -1;
    }
    msg->ue_context = context;
    ebi_index = msg->gtpc_msg.dsr.lbi.ebi_ebi - 5;
    pdn = GET_PDN(context, ebi_index);
    msg->pdn_context = pdn;
    msg->event = DS_REQ_RCVD_EVNT;
    msg->proc = DETACH_PROC;

#ifdef FUTURE_NEED
    if((msg->gtpc_msg.dsr.indctn_flgs.header.len !=0)  && 
            (msg->gtpc_msg.dsr.indctn_flgs.indication_si != 0)) {
        msg->proc = SGW_RELOCATION_PROC;
    } 
#endif
    /*Set the appropriate event type.*/

    /* Allocate new Proc */
    proc_context_t *detach_proc = alloc_detach_proc(msg);

    /* Create new transaction */
    transData_t *gtpc_trans = (transData_t *) calloc(1, sizeof(transData_t));  
    add_gtp_transaction(source_addr, source_port, seq_num, gtpc_trans);
    gtpc_trans->proc_context = (void *)detach_proc;
    detach_proc->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = seq_num;
    gtpc_trans->peer_sockaddr = msg->peer_addr;
    context->current_proc = detach_proc;

    detach_proc->handler((void*)detach_proc, msg->event, (void*)msg); 
    return 0;
}
