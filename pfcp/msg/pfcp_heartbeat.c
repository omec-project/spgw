// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
/**
 * @brief  : Process incoming heartbeat request and send response
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "cp_io_poll.h"
#include "cp_peer.h"
#include "pfcp_messages_decoder.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_util.h"
#include "spgw_cpp_wrapper.h"
#include "cp_io_poll.h"
#include "tables/tables.h"

static void
fill_pfcp_heartbeat_resp(pfcp_hrtbeat_rsp_t *pfcp_heartbeat_resp)
{

	uint32_t seq  = 1;
	memset(pfcp_heartbeat_resp, 0, sizeof(pfcp_hrtbeat_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_heartbeat_resp->header),
			PFCP_HEARTBEAT_RESPONSE, NO_SEID, seq);

	set_recovery_time_stamp(&(pfcp_heartbeat_resp->rcvry_time_stmp));
}

static int
process_heartbeat_request(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{
	int encoded = 0;
	int decoded = 0;
	uint8_t pfcp_msg[1024]= {0};

	RTE_SET_USED(decoded);

	memset(pfcp_msg, 0, 1024);
	pfcp_hrtbeat_req_t *pfcp_heartbeat_req = malloc(sizeof(pfcp_hrtbeat_req_t));
	pfcp_hrtbeat_rsp_t  pfcp_heartbeat_resp = {0};
	decoded = decode_pfcp_hrtbeat_req_t(buf_rx, pfcp_heartbeat_req);
	fill_pfcp_heartbeat_resp(&pfcp_heartbeat_resp);
	pfcp_heartbeat_resp.header.seid_seqno.no_seid.seq_no = pfcp_heartbeat_req->header.seid_seqno.no_seid.seq_no;

	encoded = encode_pfcp_hrtbeat_rsp_t(&pfcp_heartbeat_resp,  pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

	/* Reset the periodic timers */
	process_response((uint32_t)peer_addr->sin_addr.s_addr);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, peer_addr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_ECHORSP, peer_addr->sin_addr.s_addr);
	free(pfcp_heartbeat_req);
	return 0;
}

int handle_pfcp_heartbit_req_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    int ret;
	struct sockaddr_in *peer_addr = &msg->peer_addr;
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_ECHOREQ, peer_addr->sin_addr.s_addr);

    ret = process_heartbeat_request((uint8_t*)pfcp_rx, peer_addr);
    if(ret != 0){
        LOG_MSG(LOG_ERROR, "Failed to process pfcp heartbeat request");
    }
    return 0;
}

/**
 * @brief  : Process hearbeat response message
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
process_heartbeat_response(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{
	process_response((uint32_t)peer_addr->sin_addr.s_addr);
	pfcp_hrtbeat_rsp_t pfcp_hearbeat_resp = {0};
	decode_pfcp_hrtbeat_rsp_t(buf_rx, &pfcp_hearbeat_resp);
	uint32_t recov_time;

	int ret = peer_heartbeat_entry_lookup(peer_addr->sin_addr.s_addr, &recov_time);

	if (ret == -ENOENT) {
		LOG_MSG(LOG_DEBUG, "No entry found for the heartbeat!!");

	} else {
		/*TODO: Restoration part to be added if recovery time is found greater*/
		uint32_t update_recov_time = 0;
		update_recov_time =  (pfcp_hearbeat_resp.rcvry_time_stmp.rcvry_time_stmp_val);

		if(update_recov_time > recov_time) {

			ret = add_data_to_heartbeat_hash_table(&peer_addr->sin_addr.s_addr, &update_recov_time);

		}
	}

	return 0;
}

int handle_pfcp_heartbit_rsp_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
	struct sockaddr_in *peer_addr = &msg->peer_addr;
    int ret;
    ret = process_heartbeat_response((uint8_t *)pfcp_rx, peer_addr);
    if(ret != 0){
        LOG_MSG(LOG_ERROR, "Failed to process pfcp heartbeat response");
    } else {
        increment_userplane_stats(MSG_RX_PFCP_SXASXB_ECHORSP, peer_addr->sin_addr.s_addr);
    }
    return 0;
}
