// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef BEARER_H
#define BEARER_H
/**
 * @file
 *
 * Contains all data structures required by 3GPP TS 23.401 Tables 5.7.3-1 and
 * 5.7.4-1 (that are nessecary for current implementaiton) to describe the
 * Connections, state, bearers, etc as well as functions to manage and/or
 * obtain value for their fields.
 *
 */

#include <stdint.h>
#include <arpa/inet.h>
#include "cp_common.h"
#include "gtpv2_ie.h"
#include "cp_interface.h"
#include "pfcp_cp_struct.h"
#include "cp_timer.h"
#include "cp_peer_struct.h"
#include "upf_struct.h"
#include "spgw_config_struct.h"
#include "proc_struct.h"
#include <sys/queue.h>
#include "sm_struct.h"
#include "policy.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Need to handle case of multiple charging rule for signle bearer
 * this count will change once added handling
 * */
//FIXME we need to have higher limit or dynamic allocations
#define NUMBER_OF_PDR_PER_BEARER 16
#define NUMBER_OF_QER_PER_BEARER 16
#define NUMBER_OF_URR_PER_BEARER 16

struct pdn_connection;
/**
 * @brief  : Maintains eps bearer related information
 */
struct eps_bearer {
	uint8_t eps_bearer_id;
	/* Packet Detection identifier/Rule_ID */
	uint8_t pdr_count;
	pdr_t *pdrs[NUMBER_OF_PDR_PER_BEARER];

	/* As per discussion der will be only one qer per bearer */
	uint8_t qer_count;
	qer_id_t qer_id[NUMBER_OF_QER_PER_BEARER];

	uint8_t urr_count;
	urr_id_t urr_id[NUMBER_OF_URR_PER_BEARER];


	bearer_qos_ie qos;

	/*VSD: Fill the ID in intial attach */
	/* Generate ID while creating default bearer */
	uint32_t charging_id;

	struct in_addr s1u_sgw_gtpu_ipv4;
	uint32_t s1u_sgw_gtpu_teid;
	struct in_addr s5s8_sgw_gtpu_ipv4;
	uint32_t s5s8_sgw_gtpu_teid;
	struct in_addr s5s8_pgw_gtpu_ipv4;
	uint32_t s5s8_pgw_gtpu_teid;
	struct in_addr s1u_enb_gtpu_ipv4;
	uint32_t s1u_enb_gtpu_teid;

	struct in_addr s11u_mme_gtpu_ipv4;
	uint32_t s11u_mme_gtpu_teid;

	struct pdn_connection *pdn;

	uint8_t num_packet_filters;
	int packet_filter_map[MAX_FILTERS_PER_UE];

	uint8_t num_dynamic_filters; // FIXME : change variable name 
	struct dynamic_rule *dynamic_rules[16];
    bool ambr_qer_flag;
};

typedef struct eps_bearer  eps_bearer_t;

void cleanup_bearer_context(eps_bearer_t *bearer);

/**
 * Create the ue eps Bearer context by PDN (if needed), and key is sgwc s5s8 teid.
 * @param fteid_key
 *    value of information element of the sgwc s5s8 teid
 * @param bearer
 *  Eps Bearer context
 * @return
 *    \- 0 if successful
 *    \- > if error occurs during packet filter parsing corresponds to
 *          3gpp specified cause error value
 *   \- < 0 for all other errors
*/
int
add_bearer_entry_by_sgw_s5s8_tied(uint32_t fteid_key, eps_bearer_t **bearer);


#ifdef __cplusplus
}
#endif

#endif /* BEARER_H */
