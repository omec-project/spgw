/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __PFCP_INTERFACE_H_
#define __PFCP_INTERFACE_H_

#include "sm_struct.h"
#include "pfcp_ies.h"
#include "stdint.h"
/**
 * @brief  : Process PFCP message.
 * @param  : buf_rx
 *           buf - message buffer.
 * @param  : bytes_rx
 *           received message buffer size
 * @return : Returns 0 in case of success , -1 otherwise
 */
int msg_handler_sx_n4(void);

void init_pfcp_interface(void);

typedef int (*pfcp_handler)(msg_info_t **msg, pfcp_header_t *); 

extern pfcp_handler pfcp_msg_handler[256];

int 
handle_unknown_pfcp_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_heartbit_rsp_msg(msg_info_t **msg, pfcp_header_t *pfcp_header);

int 
handle_pfcp_heartbit_req_msg(msg_info_t **msg, pfcp_header_t *pfcp_header);

int 
handle_pfcp_association_setup_request_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_association_setup_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_session_est_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int
handle_pfcp_session_mod_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int
handle_pfcp_session_delete_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_session_report_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_pfd_management_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_set_deletion_response_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);

int 
handle_pfcp_session_delete_request_msg(msg_info_t **msg, pfcp_header_t *pfcp_rx);
#endif
