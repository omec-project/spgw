// Copyright (c) 2019 Intel Corporation
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "util.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "cp_peer.h"
#include "cp_log.h"
#include "cp_io_poll.h"
#include "gtpv2_internal.h"
#include "spgw_config_struct.h"
#include "spgw_cpp_wrapper.h"

extern uint8_t rstCnt;
extern uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];
extern uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];

int
process_echo_request(gtpv2c_header_t *gtpv2c_rx, gtpv2c_header_t *gtpv2c_tx)
{
	/* Due to the union & structure alignments in the gtpv2c_header_t, the
	 * sequence number would always be present in the has_teid.seq memory
	 */
    set_gtpv2c_echo(gtpv2c_tx, gtpv2c_rx->gtpc.teid_flag, GTP_ECHO_RSP,
            0, gtpv2c_rx->teid.no_teid.seq);
	return 0;
}

/**
 * @brief  : Set values in recovery ie
 * @param  : recovery, ie structure to be filled
 * @param  : type, ie type
 * @param  : length, total length
 * @param  : instance, instance value
 * @return : Returns nothing
 */
static void
set_recovery_ie_t(gtp_recovery_ie_t *recovery, uint8_t type, uint16_t length,
					uint8_t instance)
{
	recovery->header.type = type;
	recovery->header.len = length;
	recovery->header.instance = instance;

	recovery->recovery = rstCnt;

}
/**
 * @brief  : Function to build GTP-U echo request
 * @param  : echo_pkt buf pointer
 * @param  : gtpu_seqnb, sequence number
 * @return : void
 */
void
build_gtpv2_echo_request(gtpv2c_header_t *echo_pkt, uint16_t gtpu_seqnb)
{
	echo_request_t echo_req = {0};

	set_gtpv2c_header((gtpv2c_header_t *)&echo_req.header, 0, 
			GTP_ECHO_REQ, 0, gtpu_seqnb);

	set_recovery_ie_t((gtp_recovery_ie_t *)&echo_req.recovery, GTP_IE_RECOVERY,
						sizeof(uint8_t), IE_INSTANCE_ZERO);

	uint16_t msg_len = 0;
	msg_len = encode_echo_request(&echo_req, (uint8_t *)echo_pkt);

	echo_pkt->gtpc.message_len = htons(msg_len - 4);
}

int 
handle_echo_request(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
	uint16_t payload_length = 0;
	int ret = 0;
    uint32_t iface = msg->source_interface; 
    struct sockaddr_in *peer_addr;
    peer_addr = &msg->peer_addr;
    gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) s11_tx_buf;

	if(iface == S11_IFACE) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_ECHOREQ, peer_addr->sin_addr.s_addr);
    } else if (iface == S5S8_IFACE) {
        increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_ECHOREQ, peer_addr->sin_addr.s_addr);
    } else {
        // TODOSTATS
		LOG_MSG(LOG_ERROR, "Invalid interface %d ", iface);
		return -1;
	}

	ret = process_echo_request(gtpv2c_rx, gtpv2c_tx);
	if (ret) {
		LOG_MSG(LOG_ERROR, "main.c::control_plane()::Error"
				"\n\tprocess_echo_req "
				"%s: (%d) %s\n",
				gtp_type_str(gtpv2c_rx->gtpc.message_type), ret,
				(ret < 0 ? strerror(-ret) : cause_str((enum cause_value)ret)));
	}

    /* Reset ECHO Timers */
    ret = process_response(peer_addr->sin_addr.s_addr);
    if (ret) {
        /*TODO: Error handling not implemented */
    }

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	if(iface == S11_IFACE){
		gtpv2c_send(my_sock.sock_fd_s11, s11_tx_buf, payload_length,
				(struct sockaddr *) peer_addr,
				sizeof(struct sockaddr_in));
        increment_mme_peer_stats(MSG_TX_GTPV2_S11_ECHOREQ, peer_addr->sin_addr.s_addr);
	} else {
		gtpv2c_send(my_sock.sock_fd_s5s8, s11_tx_buf, payload_length,
				(struct sockaddr *) peer_addr,
                sizeof(struct sockaddr_in));
        if(cp_config->cp_type == PGWC) {
            increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, peer_addr->sin_addr.s_addr);
        } else {
            increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, peer_addr->sin_addr.s_addr);
        }
	}
	return 0;
}

int 
handle_echo_response(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    msg_info_t *msg = *msg_p;
	int ret = 0;
    uint32_t iface = msg->source_interface; 
    struct sockaddr_in *peer_addr;
    peer_addr = &msg->peer_addr;

	if(iface == S11_IFACE) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_ECHORSP, peer_addr->sin_addr.s_addr);
    } else if (iface == S5S8_IFACE && cp_config->cp_type == PGWC) {
        increment_sgw_peer_stats(MSG_RX_GTPV2_S5S8_ECHORSP, peer_addr->sin_addr.s_addr);
    } else { 
		LOG_MSG(LOG_ERROR, "Invalid interface %d ",iface);
		return -1;
	}

    ret = process_response(peer_addr->sin_addr.s_addr);
    if (ret) {
        LOG_MSG(LOG_ERROR, "main.c::control_plane()::Error"
                "\n\tprocess_echo_resp "
                "%s: (%d) %s\n",
                gtp_type_str(gtpv2c_rx->gtpc.message_type), ret,
                (ret < 0 ? strerror(-ret) : cause_str((enum cause_value)ret)));
        /* Error handling not implemented */
        return -1;
    }
	return 0;
}
