// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */
#ifndef __TRANS_STRUCT_H
#define __TRANS_STRUCT_H
#include <stdint.h>
#include <netinet/ip.h>
#include "cp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  : Maintains transaction information 
 */
typedef void (*timeout_handler_t)(void *);

struct transData 
{
    uint8_t     iface;
    uint8_t     msg_type;
    uint8_t     self_initiated;
    uint32_t    sequence;
	uint16_t    buf_len;
	uint8_t     buf[1024];
    void        *cb_data; /* UE context or upf context */ 
    void        *proc_context;
	gstimerinfo_t  rt;
    uint8_t itr_cnt;
    timeout_handler_t timeout_function;

    /* This is important field, since sender FTEID and actual sender address can be different */
    // Requirement - addr, port should be kept, this helps in cleaning both self initiated & peer initiated transactions
    struct sockaddr_in peer_sockaddr; 
};
typedef struct transData transData_t; 
#ifdef __cplusplus
}
#endif

#endif
