// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "pfcp.h"
#include "cp_log.h"
#include "sm_structs_api.h"
#include "spgw_config_struct.h"
#include "sm_struct.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "gtpv2_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "vepc_cp_dp_api.h"
#include "pfcp_cp_util.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "cp_transactions.h"
#include "spgw_cpp_wrapper.h"
#include "cp_peer.h"
#include "gtpv2_error_rsp.h"
#include "proc_s1_release.h"
#include "sm_structs_api.h"
#include "util.h"
#include "cp_io_poll.h"

/**
 * @brief  : from parameters, populates gtpv2c message 'release access bearer response'
 *           and populates required information elements as defined by
 *           clause 7.2.22 3gpp 29.274
 * @param  : gtpv2c_tx
 *           transmission buffer to contain 'release access bearer request' message
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : context
 *           UE Context data structure pertaining to the bearer to be modified
 * @return : Returns nothing
 */
void
set_release_access_bearer_response(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, uint32_t s11_mme_gtpc_teid)
{
	set_gtpv2c_teid_header(gtpv2c_tx, GTP_RELEASE_ACCESS_BEARERS_RSP,
	    htonl(s11_mme_gtpc_teid), sequence);

	set_cause_accepted_ie(gtpv2c_tx, IE_INSTANCE_ZERO);

}

// saegw - CONN_SUSPEND_PROC CONNECTED_STATE REL_ACC_BER_REQ_RCVD_EVNT => process_rel_access_ber_req_handler  
// sgw   - CONN_SUSPEND_PROC CONNECTED_STATE REL_ACC_BER_REQ_RCVD_EVNT => process_rel_access_ber_req_handler 
int handle_rab_request(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    int ret;
    struct sockaddr_in *s11_peer_sockaddr;
    ue_context_t *context = NULL;

    s11_peer_sockaddr = &msg->peer_addr;

    increment_mme_peer_stats(MSG_RX_GTPV2_S11_RABREQ, s11_peer_sockaddr->sin_addr.s_addr);
    /* Reset periodic timers */
    process_response(s11_peer_sockaddr->sin_addr.s_addr);

    ret = decode_rel_acc_bearer_req((uint8_t *)gtpv2c_rx, &msg->gtpc_msg.rab);
    if (ret == 0) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_RABREQ_DROP, s11_peer_sockaddr->sin_addr.s_addr);
        return -1;
    }

    /* Find old transaction */
    uint32_t source_addr = msg->peer_addr.sin_addr.s_addr;
    uint16_t source_port = msg->peer_addr.sin_port;
    uint32_t seq_num = msg->gtpc_msg.rab.header.teid.has_teid.seq;  
    transData_t *old_trans = (transData_t*)find_gtp_transaction(source_addr, source_port, seq_num);

    if(old_trans != NULL) {
        LOG_MSG(LOG_WARN, "Retransmitted RAB received. Old RAB is in progress");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_RABREQ_DROP, s11_peer_sockaddr->sin_addr.s_addr);
        return -1;
    }

    context = (ue_context_t *)get_ue_context(msg->gtpc_msg.rab.header.teid.has_teid.teid);
    if(context == NULL) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_RABREQ_DROP, s11_peer_sockaddr->sin_addr.s_addr);
        rab_error_response(msg, GTPV2C_CAUSE_CONTEXT_NOT_FOUND,
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
        return -1;
    }
    msg->ue_context = context;
	msg->proc = S1_RELEASE_PROC;

    for(uint8_t ebi_index = 0; ebi_index <= 11; ebi_index++) {
        pdn_connection_t *pdn = GET_PDN(context, ebi_index);
        if(pdn != NULL) {
            msg->pdn_context = pdn;
            break;
        }
    }
	/*Set the appropriate event type.*/
	msg->event = REL_ACC_BER_REQ_RCVD_EVNT;

    /* Allocate new Proc */
    proc_context_t *rab_proc = alloc_rab_proc(msg);

    /* Create new transaction */
    transData_t *trans = (transData_t *) calloc(1, sizeof(transData_t));  
    trans->self_initiated = 0;
    add_gtp_transaction(source_addr, source_port, seq_num, trans);
    trans->proc_context = (void *)rab_proc;
    rab_proc->gtpc_trans = trans;
    trans->sequence = seq_num;
    trans->peer_sockaddr = msg->peer_addr;

    start_procedure(rab_proc, msg);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

    return 0;
}
