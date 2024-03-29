// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0


#ifndef __CP_PEERS__H
#define __CP_PEERS__H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Timer callback
 * @param  : ti, holds information about timer
 * @param  : data_t, Peer node related information
 * @return : Returns nothing
 */
void timerCallback( gstimerinfo_t *ti, const void *data_t );

uint8_t process_response(uint32_t dstIp);

/**
 * @brief  : Adds node connection entry
 * @param  : dstIp, node ip address
 * @param  : portId, port number of node
 * @return : Returns nothing
 */
uint8_t
add_node_conn_entry(uint32_t dstIp, uint8_t portId);

//timeouts are handled as events 
void handle_timeout_event(void *data, uint16_t event);

#ifdef __cplusplus
}
#endif
#endif
