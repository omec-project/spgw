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
#include "gtpv2_msg_struct.h"

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
		dnlnk_data_notif_ack_t ddn_ack;
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
		pfcp_assn_setup_req_t pfcp_ass_req;
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

    uint32_t source_interface;
    struct sockaddr_in peer_addr;
    void *ue_context;
    void *pdn_context;
    void *bearer_context;
    void *proc_context;
    uint32_t refCnt;
    void *raw_buf;
    uint16_t rar_seq_num;
}msg_info_t;

#endif
