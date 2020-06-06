/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __PFCP_CP_COMMON__
#define __PFCP_CP_COMMON__

#include "sm_struct.h"
int validate_pfcp_message_content(msg_info *msg);

/**
 * @brief  : Process PFCP message.
 * @param  : buf_rx
 *           buf - message buffer.
 * @param  : bytes_rx
 *           received message buffer size
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_msg(uint8_t *buf_rx,
		struct sockaddr_in *peer_addr);


#endif
