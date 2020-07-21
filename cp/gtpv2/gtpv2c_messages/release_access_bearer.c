// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "ue.h"
#include "pfcp.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "sm_structs_api.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "sm_struct.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "gtpv2c_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "pfcp_cp_util.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "cp_transactions.h"
#include "spgw_cpp_wrapper.h"

extern udp_sock_t my_sock;
/**
 * @brief  : parses gtpv2c message and populates parse_release_access_bearer_request_t
 *           structure
 * @param  : gtpv2c_rx
 *           buffer containing received release access bearer request message
 * @param  : release_access_bearer_request
 *           structure to contain parsed information from message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *             specified cause error value
 *           - < 0 for all other errors
 */
#if 0
int
parse_release_access_bearer_request(gtpv2c_header_t *gtpv2c_rx,
		rel_acc_ber_req *rel_acc_ber_req_t)
{
	/* VS: Remove this part at integration of libgtpv2 lib*/
	rel_acc_ber_req_t->header = *(gtpv2c_header_t *)gtpv2c_rx;

	uint32_t teid = ntohl(gtpv2c_rx->teid.has_teid.teid);

	int ret = rte_hash_lookup_data(ue_context_by_fteid_hash,
	    (const void *) &teid,
	    (void **) &rel_acc_ber_req_t->context);

	if (ret < 0 || !rel_acc_ber_req_t->context)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;

	if(gtpv2c_rx != NULL) {
		if(gtpv2c_rx->gtpc.teid_flag == 1) {
			rel_acc_ber_req_t->seq = ntohl(gtpv2c_rx->teid.has_teid.seq);
		} else {
			rel_acc_ber_req_t->seq = ntohl(gtpv2c_rx->teid.no_teid.seq);
		}
	}

	return 0;
}
#endif

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


