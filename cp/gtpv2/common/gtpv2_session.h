// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef GTPC_SESSION_H
#define GTPC_SESSION_H
#include "ue.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "spgw_config_struct.h"
#include "ipc_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Maintains seid, bearer id, sgw teid , pgw ip for cp
 */
struct gw_info {
	uint8_t eps_bearer_id;
	uint32_t s5s8_sgw_gtpc_teid;
	uint32_t s5s8_pgw_gtpc_ipv4;
	uint64_t seid;
};

/**
 * @brief  : deletes ue context information
 * @param  : ds_req, holds info from delete sess request
 * @param  : context, context to be deleted
 * @param  : s5s8_pgw_gtpc_teid, pgwc teid
 * @param  : s5s8_pgw_gtpc_ip, pgwc ip
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
delete_context(gtp_eps_bearer_id_ie_t lbi, uint32_t teid,
	ue_context_t **_context, uint32_t *s5s8_pgw_gtpc_teid,
	uint32_t *s5s8_pgw_gtpc_ipv4);

/**
 * @brief  : Fill create session response on pgwc
 * @param  : cs_resp, response structure to be filled
 * @param  : sequence, sequence number
 * @param  : context, ue context info
 * @param  : ebi_index, index of bearer in bearer array
 * @return : Returns nothing
 */
void
fill_pgwc_create_session_response(create_sess_rsp_t *cs_resp,
		uint32_t sequence, ue_context_t *context, uint8_t ebi_index);
/**
 * @brief  : Process delete session request received on s5s8 interface , on pgwc
 * @param  : ds_req, holds info from request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pgwc_s5s8_delete_session_request(del_sess_req_t *ds_req);

/**
 * @brief  : Delete ue context on sgwc
 * @param  : gtpv2c_teid, teid
 * @param  : context, ue context to be deleted
 * @param  : seid, seid
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
delete_sgwc_context(uint32_t gtpv2c_teid, ue_context_t **_context, uint64_t *seid);

int
process_s11_upd_bearer_response(upd_bearer_rsp_t *ub_rsp);

int
process_s5s8_upd_bearer_response(upd_bearer_rsp_t *ub_rsp);

#ifdef __cplusplus
}
#endif
#endif
