// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __CP_PROC_H
#define __CP_PROC_H

#include <sys/queue.h>
#include "trans_struct.h"

#define UPF_ASSOCIATION_PENDING  0x00000001 

typedef void (*event_handler_t) (void*,void*);
/* GTP Req : Create transaction, create proc  and link proc to trans
 * respinse find transaction, linked proc pointer form trans
 */ 
struct proc_context {
    uint32_t        proc_type;
    uint32_t        state;
    uint32_t        flags;
    void*           ue_context;
    void*           pdn_context;
    void*           upf_context;
    transData_t*    pfcp_trans;
    transData_t*    gtpc_trans;
    event_handler_t handler;
    void            *msg_info;
	TAILQ_ENTRY(proc_context) next_sub_proc;
	TAILQ_ENTRY(proc_context) next_node_proc;
};
typedef struct proc_context proc_context_t;

#endif
