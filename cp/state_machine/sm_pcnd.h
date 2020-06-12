// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef SM_PCND_H
#define SM_PCND_H

#include "sm_enum.h"
#include "sm_hand.h"
#include "sm_struct.h"
#include "pfcp_messages.h"
#include "gtp_messages.h"

/**
 * @brief  : Decode and validate gtpv2c message
 * @param  : gtpv2c_rx, message data
 * @param  : msg, structure to store decoded message
 * @param  : bytes_rx, number of bytes in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
gtpc_pcnd_check(gtpv2c_header_t *gtpv2c_rx, msg_info *msg, int bytes_rx);

/**
 * @brief  : Decode and validate pfcp messages
 * @param  : pfcp_rx, message data
 * @param  : msg, structure to store decoded message
 * @param  : bytes_rx, number of bytes in message
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
pfcp_pcnd_check(uint8_t *pfcp_rx, msg_info *msg, int bytes_rx);

/**
 * @brief  : Decode and validate gx messages
 * @param  : gx_rx, message data
 * @param  : msg, structure to store decoded message
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
gx_pcnd_check(gx_msg *gx_rx, msg_info *msg);

#ifdef USE_DNS_QUERY
/**
 * @brief  : Retrive upf entry from hash
 * @param  : ctxt, ue context
 * @param  : entry, variable to store retrived dns entry
 * @param  : upf_ip, variable to store retrived ip
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_upf_ip(ue_context_t *ctxt, upfs_dnsres_t **_entry,
		uint32_t **upf_ip);

#endif /* USE_DNS_QUERY */
#endif
