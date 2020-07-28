// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef SM_STRUCT_H
#define SM_STRUCT_H

#include "stdio.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "pfcp_messages.h"
#include "gtp_messages.h"
#include <sys/queue.h>
#include "gx_struct.h"
#include "gx_interface.h"
#include "gtpv2c_msg_struct.h"


enum source_interface {
	GX_IFACE = 1,
	S11_IFACE = 2,
	S5S8_IFACE = 3,
	PFCP_IFACE = 4,
};

//extern enum source_interface iface;


/* TODO: Need to optimized generic structure. */
/**
 * @brief  : Maintains decoded message from different messages
 */
typedef struct msg_info {
	uint8_t msg_type;
	uint8_t state;
	uint8_t event;
	uint8_t proc;

	/* VS: GX Msg retrieve teid of key for UE Context */
	uint8_t eps_bearer_id;
	uint32_t teid;

	union gtpc_msg_info {
		create_sess_req_t csr;
		create_sess_rsp_t cs_rsp;
		mod_bearer_req_t mbr;
		mod_bearer_rsp_t mb_rsp;
		del_sess_req_t dsr;
		del_sess_rsp_t ds_rsp;
		rel_acc_bearer_req_t rab;
		downlink_data_notification_t ddn_ack;
		create_bearer_req_t cb_req;
		create_bearer_rsp_t cb_rsp;
		del_bearer_req_t db_req;
		del_bearer_rsp_t db_rsp;
		upd_bearer_req_t ub_req;
		upd_bearer_rsp_t ub_rsp;
		pgw_rstrt_notif_ack_t pgw_rstrt_notif_ack;
		upd_pdn_conn_set_req_t upd_pdn_req;
		upd_pdn_conn_set_rsp_t upd_pdn_rsp;
		del_pdn_conn_set_req_t del_pdn_req;
		del_pdn_conn_set_rsp_t del_pdn_rsp;
		del_bearer_cmd_t  del_ber_cmd;
	}gtpc_msg;
	union pfcp_msg_info_t {
		pfcp_pfd_mgmt_rsp_t pfcp_pfd_resp;
		pfcp_assn_setup_rsp_t pfcp_ass_resp;
		pfcp_sess_estab_rsp_t pfcp_sess_est_resp;
		pfcp_sess_mod_rsp_t pfcp_sess_mod_resp;
		pfcp_sess_del_rsp_t pfcp_sess_del_resp;
		pfcp_sess_rpt_req_t pfcp_sess_rep_req;
		pfcp_sess_set_del_req_t pfcp_sess_set_del_req;
		pfcp_sess_set_del_rsp_t pfcp_sess_set_del_rsp;
	}pfcp_msg;

	union gx_msg_info_t {
		GxCCA cca;
		GxRAR rar;
	}gx_msg;

    uint8_t rx_interface;
    upf_context_t *upf_context;
    ue_context_t *ue_context;
    pdn_connection_t *pdn_context;
    eps_bearer_t *bearer_context;
    struct sockaddr_in peer_addr;
    proc_context_t *proc_context;
}msg_info_t;

/**
 * @brief  : Structure for handling CS/MB/DS request synchoronusly.
 */
struct resp_info {
	uint8_t proc;
	uint8_t state;
	uint8_t msg_type;
	uint8_t num_of_bearers;
	uint8_t eps_bearer_id;
	uint8_t list_bearer_ids[MAX_BEARERS];

	/* Default Bearer Id */
	uint8_t linked_eps_bearer_id;

	/* Dedicated Bearer Id */
	uint8_t bearer_count;
	uint8_t eps_bearer_ids[MAX_BEARERS];

	uint32_t s5s8_sgw_gtpc_teid;
	uint32_t s5s8_pgw_gtpc_ipv4;

	uint8_t eps_bearer_lvl_tft[257];
	uint8_t tft_header_len;

	union gtpc_msg {
		create_sess_req_t csr;
		mod_bearer_req_t mbr;
		del_sess_req_t dsr;
		del_bearer_cmd_t del_bearer_cmd;
	}gtpc_msg;
}__attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));


#endif
