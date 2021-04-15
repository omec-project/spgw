// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __GTPV2_MSG_STRUCT_H
#define __GTPV2_MSG_STRUCT_H

#include "gtp_ies.h"
#include "gtpv2_ie.h"

#ifdef __cplusplus
extern "C" {
#endif
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

#ifdef __cplusplus
}
#endif
#endif
