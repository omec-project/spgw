/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2019 Sprint
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
 *
 */
#ifndef __CSID_CP_CLEANUP_H
#define __CSID_CP_CLEANUP_H
#ifdef __cplusplus
extern "C" {
#endif


#ifdef USE_CSID
int8_t
fill_pgw_restart_notification(gtpv2c_header_t *gtpv2c_tx,
		uint32_t s11_sgw, uint32_t s5s8_pgw);

/* Cleanup Session information by local csid*/
int8_t
del_peer_node_sess(uint32_t node_addr, uint8_t iface);

int8_t
process_del_pdn_conn_set_req_t(del_pdn_conn_set_req_t *del_pdn_req,
		gtpv2c_header_t *gtpv2c_tx);

int8_t
fill_gtpc_del_set_pdn_conn_rsp(gtpv2c_header_t *gtpv2c_tx, uint8_t seq_t,
		uint8_t casue_value);

int8_t
process_del_pdn_conn_set_rsp_t(del_pdn_conn_set_rsp_t *del_pdn_rsp);

int8_t
process_upd_pdn_conn_set_req_t(upd_pdn_conn_set_req_t *upd_pdn_req);

int8_t
process_upd_pdn_conn_set_rsp_t(upd_pdn_conn_set_rsp_t *upd_pdn_rsp);

/* Function */
int process_pfcp_sess_set_del_req_t(pfcp_sess_set_del_req_t *del_set_req,
		gtpv2c_header_t *gtpv2c_tx);

/* Function */
int process_pfcp_sess_set_del_rsp_t(pfcp_sess_set_del_rsp_t *del_set_rsp);


#endif

#ifdef __cplusplus
}
#endif
#endif

