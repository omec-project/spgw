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

#define PROC_FLAGS_RUNNING  0x00000001

typedef void (*event_handler_t) (void*,void*);
typedef void (*add_child_proc_t) (void*, void*);
typedef void (*done_child_proc_t) (void*);
#define MAX_CHILD_PROC 8

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
    uint16_t        rar_seq_num;
	TAILQ_ENTRY(proc_context) next_sub_proc;
	TAILQ_ENTRY(proc_context) next_node_proc;

    // child procs 
    add_child_proc_t child_proc_add; 
    done_child_proc_t child_proc_done; 
    uint8_t          child_procs_cnt;
    void*            child_procs[MAX_CHILD_PROC]; 
    void*            parent_proc;
    uint16_t         tac;
};
typedef struct proc_context proc_context_t;

// BUG : Should i check refCnt ?
#define SET_PROC_MSG(proc, msg) { \
    if(proc->msg_info != NULL) { \
       msg_info_t *_tmp = (msg_info_t *)proc->msg_info;\
       free(_tmp->raw_buf); \
       free(_tmp); \
    } \
    proc->msg_info = (void *)msg;\
    msg->refCnt++; \
}
#endif
