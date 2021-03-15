/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#ifndef __CSID_API_H
#define __CSID_API_H
#include "gtp_messages.h"
#include "ue.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef USE_CSID
int8_t
update_peer_csid_link(fqcsid_t *fqcsid, fqcsid_t *fqcsid_t);

int
csrsp_fill_peer_node_info(create_sess_req_t *csr,
			pdn_connection_t *pdn, eps_bearer_t *bearer);

void
set_gtpc_fqcsid_t(gtp_fqcsid_ie_t *fqcsid,
		enum ie_instance instance, fqcsid_t *csids);

/* Cleanup Session information by local csid*/
int8_t
del_pfcp_peer_node_sess(uint32_t node_addr, uint8_t iface);

/* Fill the FQ-CSID values in session est request */
int8_t
fill_fqcsid_sess_est_req(pfcp_sess_estab_req_t *pfcp_sess_est_req, ue_context_t *context);


int
fill_peer_node_info(pdn_connection_t *pdn, eps_bearer_t *bearer);

#endif

#ifdef __cplusplus
}
#endif
#endif

