// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_messages_decoder.h"
#include "gw_adapter.h"
#include "ue.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "clogger.h"
#include "cp_io_poll.h"
#include "vepc_cp_dp_api.h"
#include"cp_config.h"
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
#include "tables/tables.h"
#include "util.h"

extern struct cp_stats_t cp_stats;
extern udp_sock_t my_sock;

#if 0
/**
 * @brief  : Maintains downlink data notification acknowledgement information
 */
struct downlink_data_notification_ack_t {
	ue_context_t *context;

	gtpv2c_ie *cause_ie;
	uint8_t *delay;
	/* TODO! More to implement... See table 7.2.11.2-1
	 * 'Recovery: This IE shall be included if contacting the peer
	 * for the first time'
	 */
};
#endif

/**
 * @brief  : callback to handle downlink data notification messages from the
 *           data plane
 * @param  : msg_payload
 *           message payload received by control plane from the data plane
 * @return : 0 inicates success, error otherwise
 */
int
cb_ddn(struct msgbuf *msg_payload)
{
	int ret = ddn_by_session_id(msg_payload->msg_union.sess_entry.sess_id);

	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "Error on DDN Handling %s: (%d) %s\n",
				gtp_type_str(ret), ret,
				(ret < 0 ? strerror(-ret) : cause_str(ret)));
	}
	return ret;
}

/**
 * @brief  : creates and sends downlink data notification according to session
 *           identifier
 * @param  : session_id - session identifier pertaining to downlink data packets
 *           arrived at data plane
 * @return : 0 - indicates success, failure otherwise
 */
int
ddn_by_session_id(uint64_t session_id)
{
	uint8_t tx_buf[MAX_GTPV2C_UDP_LEN] = { 0 };
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) tx_buf;
	uint32_t sgw_s11_gtpc_teid = UE_SESS_ID(session_id);
	ue_context_t *context = NULL;
	static uint32_t ddn_sequence = 1;

	clLog(clSystemLog, eCLSeverityDebug, "%s: sgw_s11_gtpc_teid:%u\n",
			__func__, sgw_s11_gtpc_teid);

	int ret = get_ue_context(sgw_s11_gtpc_teid, &context);

	if (ret < 0 || !context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	ret = create_downlink_data_notification(context,
			UE_BEAR_ID(session_id),
			ddn_sequence,
			gtpv2c_tx);

	if (ret)
		return ret;

	struct sockaddr_in mme_s11_sockaddr_in = {
		.sin_family = AF_INET,
		.sin_port = htons(GTPC_UDP_PORT),
		.sin_addr.s_addr = htonl(context->s11_mme_gtpc_ipv4.s_addr),
		.sin_zero = {0},
	};


	uint16_t payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
			+ sizeof(gtpv2c_tx->gtpc);

	if (pcap_dumper) {
		dump_pcap(payload_length, tx_buf);
	} else {
		uint32_t bytes_tx = sendto(my_sock.sock_fd_s11, tx_buf, payload_length, 0,
		    (struct sockaddr *) &mme_s11_sockaddr_in,
		    sizeof(mme_s11_sockaddr_in));

		if (bytes_tx != (int) payload_length) {
			clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete GTPv2c Message:"
					"%u of %u tx bytes\n",
					payload_length, bytes_tx);
		}
	}
	ddn_sequence += 2;
	++cp_stats.ddn;
	

	update_cli_stats(mme_s11_sockaddr_in.sin_addr.s_addr,
					gtpv2c_tx->gtpc.message_type,SENT,S11);

	return 0;
}

/**
 * @brief  : parses gtpv2c message and populates downlink_data_notification_ack_t
 *           structure
 * @param  : gtpv2c_rx
 *           buffer containing received downlink data notification ack message
 * @param  : ddn_ack
 *           structure to contain parsed information from message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
int
parse_downlink_data_notification_ack(gtpv2c_header_t *gtpv2c_rx,
			downlink_data_notification_t *ddn_ack)
{

	gtpv2c_ie *current_ie;
	gtpv2c_ie *limit_ie;

	uint32_t teid = ntohl(gtpv2c_rx->teid.has_teid.teid);
	int ret = get_ue_context(teid, &ddn_ack->context);

	if (ret < 0 || !ddn_ack->context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;


	/** TODO: we should fully verify mandatory fields within received
	 * message */
	FOR_EACH_GTPV2C_IE(gtpv2c_rx, current_ie, limit_ie)
	{
		if (current_ie->type == GTP_IE_CAUSE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			ddn_ack->cause_ie = current_ie;
		} else if (current_ie->type == GTP_IE_DELAY_VALUE &&
				current_ie->instance == IE_INSTANCE_ZERO) {
			ddn_ack->delay =
					&IE_TYPE_PTR_FROM_GTPV2C_IE(delay_ie,
					current_ie)->delay_value;
		}
		/* TODO implement conditional IE "Recovery" */
	}

	/* Verify that cause is accepted */
	if (IE_TYPE_PTR_FROM_GTPV2C_IE(cause_ie,
			ddn_ack->cause_ie)->cause_ie_hdr.cause_value
	    != GTPV2C_CAUSE_REQUEST_ACCEPTED) {
		clLog(clSystemLog, eCLSeverityCritical, "Cause not accepted for DDNAck\n");
		return IE_TYPE_PTR_FROM_GTPV2C_IE(cause_ie,
				ddn_ack->cause_ie)->cause_ie_hdr.cause_value;
	}
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

int
process_ddn_ack(downlink_data_notification_t ddn_ack, uint8_t *delay, msg_info_t *data)
{
	int ret = 0;
    ue_context_t *context = NULL;

	/* Lookup entry in hash table on the basis of session id*/
	uint64_t ebi_index = 0;   /*ToDo : Need to revisit this.*/
	if (get_sess_entry_seid((ddn_ack.context)->pdns[ebi_index]->seid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "NO Session Entry Found for sess ID:%lu\n",
				(ddn_ack.context)->pdns[ebi_index]->seid);
		return -1;
	}
    assert(context == data->ue_context);

#ifdef DDN_STATES
	/* VS: Update the session state */
	resp->msg_type = GTP_DOWNLINK_DATA_NOTIFICATION_ACK;
	resp->state = DDN_ACK_RCVD_STATE;
#endif

	/* check for conditional delay value, set if necessary,
	 * or indicate no delay */
	if (ddn_ack.delay != NULL)
		*delay = *ddn_ack.delay;
	else
		*delay = 0;

	/* VS: Update the UE State */
	ret = update_ue_state((ddn_ack.context)->s11_sgw_gtpc_teid,
			DDN_ACK_RCVD_STATE ,ebi_index);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:Failed to update UE State for teid: %u\n", __func__,
				(ddn_ack.context)->s11_sgw_gtpc_teid);
	}
	return 0;

}

static int
process_ddn_ack_resp_handler(void *data, void *unused_param)
{
    int ret = 0;
	msg_info_t *msg = (msg_info_t *)data;

	uint8_t delay = 0; /*TODO move this when more implemented?*/
	ret = process_ddn_ack(msg->gtpc_msg.ddn_ack, &delay, data);
	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d:Error"
				"\n\tprocess_ddn_ack_resp_hand "
				"%s: (%d) %s\n", __func__, __LINE__,
				gtp_type_str(msg->msg_type), ret,
				(ret < 0 ? strerror(-ret) : cause_str(ret)));
		/* Error handling not implemented */
		return ret;
	}

	/* TODO something with delay if set */
	/* TODO Implemente the PFCP Session Report Resp message sent to dp */

	RTE_SET_USED(unused_param);
	return 0;
}

// saegw - CONN_SUSPEND_PROC DDN_REQ_SNT_STATE DDN_ACK_RESP_RCVD_EVNT ==> process_ddn_ack_resp_handler
// sgw  CONN_SUSPEND_PROC DDN_REQ_SNT_STATE DDN_ACK_RESP_RCVD_EVNT ==> process_ddn_ack_resp_handler 
int
handle_ddn_ack(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx)
{
    int ret;
    struct sockaddr_in s11_peer_sockaddr = {0};

    s11_peer_sockaddr = msg->peer_addr;
    /* Reset periodic timers */
    process_response(s11_peer_sockaddr.sin_addr.s_addr);

    ret = parse_downlink_data_notification_ack(gtpv2c_rx,
            &msg->gtpc_msg.ddn_ack);
    if (ret)
        return ret;

    // update CLCI stats 
    // validate message 
    ue_context_t *context = NULL;
    RTE_SET_USED(gtpv2c_rx);
    RTE_SET_USED(msg);

    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;
    uint32_t seq_num = gtpv2c_rx->teid.has_teid.seq;

    transData_t *gtpc_trans = delete_gtp_transaction(local_addr, port_num, seq_num);

    /* Retrive the session information based on session id. */
    if(gtpc_trans == NULL) {
        printf("Unsolicitated DDN Ack responnse \n");
        return -1;
    }

    proc_context_t *proc_context = gtpc_trans->proc_context;
    ue_context_t *context1 = proc_context->ue_context;


    /*Retrive UE state. */
    if (get_ue_context(ntohl(gtpv2c_rx->teid.has_teid.teid), &context) != 0) {
        return -1;
    }
    assert(context != context1); // no cross connection 

    msg->proc_context = gtpc_trans->proc_context;
    msg->ue_context = proc_context->ue_context; 
    msg->pdn_context = proc_context->pdn_context; 

    for(int i=0; i < MAX_BEARERS; i++){
        if(context->pdns[i] == NULL){
            continue;
        }
        else{
            msg->state = context->pdns[i]->state;
            msg->proc = context->pdns[i]->proc;
        }
    }

    /*Set the appropriate event type.*/
    msg->event = DDN_ACK_RESP_RCVD_EVNT;

    clLog(s11logger, eCLSeverityDebug, "%s: Callback called for"
            "Msg_Type:%s[%u], Teid:%u, "
            "Procedure:%s, State:%s, Event:%s\n",
            __func__, gtp_type_str(msg->msg_type), msg->msg_type,
            gtpv2c_rx->teid.has_teid.teid,
            get_proc_string(msg->proc),
            get_state_string(msg->state), get_event_string(msg->event));

    process_ddn_ack_resp_handler(msg, NULL);
    return 0;
}

