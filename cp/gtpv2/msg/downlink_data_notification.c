// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_decoder.h"
#include "ue.h"
#include "sm_struct.h"
#include "cp_log.h"
#include "cp_io_poll.h"
#include "vepc_cp_dp_api.h"
#include "spgw_config_struct.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "gtpv2_ie_parsing.h"
#include "gtpv2_internal.h"
#include "cp_interface.h"
#include "sm_structs_api.h"
#include "cp_config_defs.h"
#include "cp_peer.h"
#include "spgw_cpp_wrapper.h"
#include "sm_structs_api.h"
#include "util.h"
#include "cp_io_poll.h"
#include "proc_session_report.h"
#include "cp_transactions.h"
#include "proc.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

// saegw - CONN_SUSPEND_PROC DDN_REQ_SNT_STATE DDN_ACK_RESP_RCVD_EVNT ==> process_ddn_ack_resp_handler
// sgw  CONN_SUSPEND_PROC DDN_REQ_SNT_STATE DDN_ACK_RESP_RCVD_EVNT ==> process_ddn_ack_resp_handler 
int
handle_ddn_ack(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
    int ret = 0;
    struct sockaddr_in *peer_addr = &msg->peer_addr;
    ue_context_t *context = NULL;
    dnlnk_data_notif_ack_t *ddn_ack = &msg->rx_msg.ddn_ack;
	// uint8_t delay = 0; /*TODO move this when more implemented?*/

    increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK, peer_addr->sin_addr.s_addr);

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    ret = decode_dnlnk_data_notif_ack((uint8_t*)gtpv2c_rx, ddn_ack); 
    if (!ret) {
        LOG_MSG(LOG_DEBUG0, "DDN decode failure ");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK_DROP, peer_addr->sin_addr.s_addr);
        return ret;
    }

    context = (ue_context_t *)get_ue_context(ddn_ack->header.teid.has_teid.teid);
    if (context == NULL) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK_DROP, peer_addr->sin_addr.s_addr);
        LOG_MSG(LOG_DEBUG0, "Rcvd DDN, Context not found for TEID %u ", gtpv2c_rx->teid.has_teid.teid);
        return -1;
    }

    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;
    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;

    transData_t *gtpc_trans = (transData_t *)delete_gtp_transaction(local_addr, port_num, seq_num);

    if(gtpc_trans == NULL) {
        LOG_MSG(LOG_DEBUG3, "Unsolicitated DDN Ack response ");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DDNACK_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }

	/* stop and delete timer entry for DDN req */
	stop_transaction_timer(gtpc_trans);

    proc_context_t *proc_context = (proc_context_t *)gtpc_trans->proc_context;

    if(context == NULL) {
        context = (ue_context_t*)proc_context->ue_context;
    } else {
        assert(proc_context->ue_context == context);
    }

    // check cause from peer node...
    // if MME accepted then good..
    // if MME rejects DDN with context not found then cleanup our UE context 
    // If accept then take delay/buffer count values and pass it to user plane 

    msg->proc_context = gtpc_trans->proc_context;
    msg->event = DDN_ACK_RESP_RCVD_EVNT;
    msg->proc  = proc_context->state;
    SET_PROC_MSG(proc_context, msg);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:%s[%u], Teid:%u, "
            "Procedure:%s, Event:%s\n",
            gtp_type_str(msg->msg_type), msg->msg_type,
            gtpv2c_rx->teid.has_teid.teid,
            get_proc_string(msg->proc),
            get_event_string(msg->event));

    proc_context->handler(proc_context, msg);

	/* TODO Implemente the PFCP Session Report Resp message sent to dp */
	return 0;
}

/**
 * @brief  : from parameters, populates gtpv2c message 'downlink data notification' and
 *           populates required information elements as defined by
 *           clause 7.2.11 3gpp 29.274
 * @param  : gtpv2c_tx
 *           transmission buffer to contain 'modify bearer request' message
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : context
 *           UE Context data structure pertaining to the bearer to be modified
 * @param  : bearer
 *           bearer data structure to be modified
 * @return : Returns nothing
 */
static void
set_downlink_data_notification(gtpv2c_header_t *gtpv2c_tx,
		uint32_t sequence, ue_context_t *context, eps_bearer_t *bearer)
{
	set_gtpv2c_teid_header(gtpv2c_tx, GTP_DOWNLINK_DATA_NOTIFICATION,
			htonl(context->s11_mme_gtpc_teid), sequence);
	set_ebi_ie(gtpv2c_tx, IE_INSTANCE_ZERO, bearer->eps_bearer_id);
	set_ar_priority_ie(gtpv2c_tx, IE_INSTANCE_ZERO, bearer);
}


int
create_downlink_data_notification(ue_context_t *context, uint8_t eps_bearer_id,
		uint32_t sequence, gtpv2c_header_t *gtpv2c_tx)
{
	eps_bearer_t *bearer = context->eps_bearers[eps_bearer_id - 5];
	if (bearer == NULL)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	set_downlink_data_notification(gtpv2c_tx, sequence, context, bearer);

	return 0;
}

void ddn_indication_timeout(void *data)
{
    proc_context_t *proc_context = (proc_context_t *)data;
    // Option 1 - Retry few times 
    // Opton  2 - after configurable retry, generate timeout event for fsm  
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    msg->event = DDN_TIMEOUT;
    msg->proc_context = proc_context;
    SET_PROC_MSG(proc_context, msg);
    proc_context->handler(proc_context, msg);

}
/**
 * @brief  : creates and sends downlink data notification according to session
 *           identifier
 * @param  : session_id - session identifier pertaining to downlink data packets
 *           arrived at data plane
 * @return : 0 - indicates success, failure otherwise
 */
int
send_ddn_indication(proc_context_t *proc_ctxt, uint8_t ebi_index)
{
    ue_context_t *context = (ue_context_t *)proc_ctxt->ue_context;
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;
	uint32_t ddn_sequence = get_gtp_sequence(); 
    int ret;

	ret = create_downlink_data_notification(context,
			ebi_index,
			ddn_sequence,
			gtpv2c_tx);

	if (ret) {
		return ret;
    }

	struct sockaddr_in mme_s11_sockaddr_in;
    memset((void*)&mme_s11_sockaddr_in, 0, sizeof(struct sockaddr_in));
	mme_s11_sockaddr_in.sin_family = AF_INET;
	mme_s11_sockaddr_in.sin_port = htons(GTPC_UDP_PORT);
	mme_s11_sockaddr_in.sin_addr.s_addr = htonl(context->s11_mme_gtpc_ipv4.s_addr);


	uint16_t payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
			+ sizeof(gtpv2c_tx->gtpc);

    /* Send response on s11 interface */
    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
            (struct sockaddr *) &mme_s11_sockaddr_in,
            sizeof(struct sockaddr_in));

    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;

    transData_t *gtpc_trans;
    gtpc_trans = start_response_wait_timer(proc_ctxt, (uint8_t *)gtp_tx_buf, 
                                            payload_length, 
                                            ddn_indication_timeout);

    SET_TRANS_SELF_INITIATED(gtpc_trans);
    gtpc_trans->proc_context = (void *)proc_ctxt;
    proc_ctxt->gtpc_trans = gtpc_trans;
    gtpc_trans->sequence = ddn_sequence;
    gtpc_trans->peer_sockaddr = mme_s11_sockaddr_in;
    add_gtp_transaction(local_addr, port_num, ddn_sequence, gtpc_trans);

    increment_mme_peer_stats(MSG_TX_GTPV2_S11_DDNREQ, mme_s11_sockaddr_in.sin_addr.s_addr);

	return 0;
}
