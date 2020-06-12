/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __GTPV2_EVT_HANDLER_H
#define __GTPV2_EVT_HANDLER_H
#include "sm_struct.h"
int handle_create_session_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_create_session_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_modify_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_modify_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_session_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_session_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_update_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_update_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_create_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_create_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_rel_access_bearer_req_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_ddn_ack_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_bearer_cmd_msg(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_delete_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_update_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_update_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int handle_pgw_restart_notf_ack(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
int process_gtp_message(gtpv2c_header_t *gtpv2c_rx, msg_info *msg);
#endif
