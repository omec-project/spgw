// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef __CP_PROC_H
#define __CP_PROC_H

#include <sys/queue.h>
#include "trans_struct.h"
#include "sm_struct.h"

#define UPF_ASSOCIATION_PENDING  0x00000001 

enum proc_result {
    PROC_RESULT_SUCCESS,
    PROC_RESULT_FAILURE
};


typedef void (*event_handler_t) (void*,void*);
/* GTP Req : Create transaction, create proc  and link proc to trans
 * respinse find transaction, linked proc pointer form trans
 */ 
// MUSTDO ? : TODO : we need union in this procedure ???
struct proc_context {
    uint32_t        proc_type;
    uint32_t        state;
    uint32_t        flags;
    uint32_t        result;
    void*           ue_context;
    void*           pdn_context;
    void*           bearer_context;
    void*           upf_context;
    transData_t*    pfcp_trans;
    transData_t*    gtpc_trans;
    event_handler_t handler;
    void            *msg_info;
    void            *gx_context;
    uint32_t        call_id;
    bool            cbrsp_received;
    uint16_t        rar_seq_num;
	TAILQ_ENTRY(proc_context) next_sub_proc;
	TAILQ_ENTRY(proc_context) next_node_proc;
};
typedef struct proc_context proc_context_t;

#define SET_PROC_MSG(proc, msg) { \
    free(proc->msg_info); \
    proc->msg_info = (void *)msg;\
    msg->refCnt++; \
}
#endif
