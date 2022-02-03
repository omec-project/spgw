// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */
#ifndef __TRANS_STRUCT_H
#define __TRANS_STRUCT_H
#include <stdint.h>
#include <netinet/ip.h>
#include "cp_timer.h"
#include "cp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DELAYED_DELETE 0x01
#define SELF_INITIATED 0x02


#define IS_TRANS_SELF_INITIATED(trans) ((trans->flags & SELF_INITIATED) != 0x00)
#define SET_TRANS_SELF_INITIATED(trans) (trans->flags = trans->flags | SELF_INITIATED)
#define RESET_TRANS_SELF_INITIATED(trans) (trans->flags = trans->flags & (~SELF_INITIATED))

#define IS_TRANS_DELAYED_DELETE(trans) ((trans->flags & DELAYED_DELETE) != 0x00)
#define SET_TRANS_DELAYED_DELETE(trans) (trans->flags = trans->flags | DELAYED_DELETE)
#define RESET_TRANS_DELAYED_DELETE(trans) (trans->flags = trans->flags & (~DELAYED_DELETE))

/**
 * @brief  : Maintains transaction information 
 */
typedef void (*timeout_handler_t)(void *);

struct transData 
{
    uint8_t     flags;
    uint8_t     iface;
    uint8_t     msg_type;
    uint8_t     itr_cnt;
    uint32_t    sequence;
	uint16_t    buf_len;
	uint8_t     buf[MAX_PFCP_MSG_SIZE];
    void        *cb_data; /* UE context or upf context */ 
    void        *proc_context;
	gstimerinfo_t  rt;
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
