/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2019 Sprint
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __PFCP_INTERFACE_H_
#define __PFCP_INTERFACE_H_

#include "sm_struct.h"
#include "pfcp_ies.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  : Process PFCP message.
 * @param  : buf_rx
 *           buf - message buffer.
 * @param  : bytes_rx
 *           received message buffer size
 * @return : Returns 0 in case of success , -1 otherwise
 */
void* msg_handler_pfcp(void*);

void init_pfcp_msg_handlers(void);

typedef int (*pfcp_handler)(msg_info_t **msg, pfcp_header_t *); 

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

void
process_pfcp_msg(void *data, uint16_t event);

/**
 * @brief  : Send data to peer node
 * @param  : fd, socket or file descriptor to use to send data
 * @param  : msg_payload, buffer to store data to be send
 * @param  : size, max size to send data
 * @param  : peer_addr, peer node address
 * @return : Returns sent number of bytes
 */
int
pfcp_send(int fd,void *msg_payload, uint32_t size,
		struct sockaddr_in *peer_addr);


void* out_handler_pfcp(void *data);

void init_pfcp(void);

void init_pfcp_msg_handlers(void);

void init_pfcp_msg_threads(void);
#ifdef __cplusplus
}
#endif
#endif
