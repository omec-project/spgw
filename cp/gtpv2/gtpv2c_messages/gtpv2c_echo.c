// Copyright (c) 2019 Intel Corporation
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "util.h"
#include "gtp_messages.h"
#include "gtpv2c_set_ie.h"
#include "gtpv2_interface.h"

extern uint8_t rstCnt;

int
process_echo_request(gtpv2c_header_t *gtpv2c_rx, gtpv2c_header_t *gtpv2c_tx)
{
	/* Due to the union & structure alignments in the gtpv2c_header_t, the
	 * sequence number would always be present in the has_teid.seq memory
	 */
	if (gtpv2c_rx->gtpc.teid_flag)
		set_gtpv2c_echo(gtpv2c_tx,
				gtpv2c_rx->gtpc.teid_flag, GTP_ECHO_RSP,
				gtpv2c_rx->teid.has_teid.teid,
				gtpv2c_rx->teid.has_teid.seq);
	else
		set_gtpv2c_echo(gtpv2c_tx,
				gtpv2c_rx->gtpc.teid_flag, GTP_ECHO_RSP,
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
 * @param  : echo_pkt rte_mbuf pointer
 * @param  : gtpu_seqnb, sequence number
 * @return : void
 */
void
build_gtpv2_echo_request(gtpv2c_header_t *echo_pkt, uint16_t gtpu_seqnb)
{
	echo_request_t echo_req = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *)&echo_req.header,
			GTP_ECHO_REQ, 0, gtpu_seqnb);

	set_recovery_ie_t((gtp_recovery_ie_t *)&echo_req.recovery, GTP_IE_RECOVERY,
						sizeof(uint8_t), IE_INSTANCE_ZERO);

	uint16_t msg_len = 0;
	msg_len = encode_echo_request(&echo_req, (uint8_t *)echo_pkt);

	echo_pkt->gtpc.message_len = htons(msg_len - 4);
}
