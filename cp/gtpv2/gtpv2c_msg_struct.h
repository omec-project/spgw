// SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __GTPV2_MSG_STRUCT_H
#define __GTPV2_MSG_STRUCT_H

#include "gtp_ies.h"
#include "gtpv2c_ie.h"
#include "gtpv2c_ie.h"
struct ue_context;
/**
 * @brief  : Table 7.2.1-1: Information Elements in a Create Session Response -
 *           incomplete list
 */
typedef struct parse_sgwc_s5s8_create_session_response_t {
	uint8_t *bearer_context_to_be_created_ebi;
	fteid_ie *pgw_s5s8_gtpc_fteid;
	gtpv2c_ie *pdn_addr_alloc_ie;
	gtpv2c_ie *apn_restriction_ie;
	gtpv2c_ie *bearer_qos_ie;
	gtpv2c_ie *bearer_tft_ie;
	gtpv2c_ie *s5s8_pgw_gtpu_fteid;
}sgwc_s5s8_create_session_response_t;

/**
 * @brief  : Structure to downlink data notification ack information struct.
 */
typedef struct downlink_data_notification { /* This struct needs to be moved to right place */
	struct ue_context *context;

	gtpv2c_ie *cause_ie;
	uint8_t *delay;
	/* todo! more to implement... see table 7.2.11.2-1
	 * 'recovery: this ie shall be included if contacting the peer
	 * for the first time'
	 */
	/* */
	uint16_t dl_buff_cnt;
	uint8_t dl_buff_duration;
}downlink_data_notification_t;

/* TODO - following structre needs to move at right place */
/**
 * @brief  : Maintains information parsed from release access bearer request
 */
typedef struct parse_release_access_bearer_request_t {
	gtpv2c_header_t header;
	struct ue_context *context;
	uint32_t seq;
} rel_acc_ber_req;

#endif
