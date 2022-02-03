// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0


#ifndef __PROC_STRUCT_H
#define __PROC_STRUCT_H

#include <sys/queue.h>
#include "trans_struct.h"
#include "sm_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
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
    char            proc_name[32];
    uint32_t        state;
    uint32_t        flags;
    uint32_t        result;
    void*           ue_context;
    void*           pdn_context;
    void*           bearer_context;
    void*           upf_context;
    transData_t*    pfcp_trans;
    transData_t*    gtpc_trans;
    transData_t*    gx_trans;
    event_handler_t handler;
    void            *msg_info;
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

    // printing purpise
    uint64_t         imsi64;

    //init proc
    void*            sub_config;
    void*            req_pco;
};
typedef struct proc_context proc_context_t;

// FIXME - BUG : Should i check refCnt ?
// read code and analyze why would proc have the msg_info 
#define SET_PROC_MSG(proc, msg) { \
    if(proc->msg_info != NULL) { \
       msg_info_t *_tmp = (msg_info_t *)proc->msg_info;\
       free(_tmp->raw_buf); \
       free(_tmp); \
    } \
    proc->msg_info = (void *)msg;\
    msg->refCnt++; \
}

typedef enum 
{
	NONE_PROC,
	INITIAL_PDN_ATTACH_PROC,
	SERVICE_REQUEST_PROC,
	SGW_RELOCATION_PROC,
	CONN_SUSPEND_PROC,
	DETACH_PROC,
	RAR_PROC,
	DED_BER_ACTIVATION_PROC,
	PDN_GW_INIT_BEARER_DEACTIVATION,
	MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC,
	UPDATE_BEARER_PROC,
	RESTORATION_RECOVERY_PROC,
    PAGING_PROC,
    USAGE_REPORT_PROC,
    S1_RELEASE_PROC,
    PFCP_ASSOC_SETUP_PROC,
    SGW_RELOCATION_DETACH_PROC,
	NW_INIT_DETACH_PROC,
	END_PROC
}sm_proc;

/* VS: Defined different states of the STATE Machine */
typedef enum 
{
	SGWC_NONE_STATE,
	PFCP_ASSOC_REQ_SNT_STATE,
	PFCP_ASSOC_RESP_RCVD_STATE,
	PFCP_SESS_EST_REQ_SNT_STATE,
	PFCP_SESS_EST_RESP_RCVD_STATE,
	CONNECTED_STATE = 5,
	IDEL_STATE,
	CS_REQ_SNT_STATE,
	CS_RESP_RCVD_STATE,
	PFCP_SESS_MOD_REQ_SNT_STATE=9,
	PFCP_SESS_MOD_RESP_RCVD_STATE,
	PFCP_SESS_DEL_REQ_SNT_STATE=11,
	PFCP_SESS_DEL_RESP_RCVD_STATE,
	DS_REQ_SNT_STATE=13,
	DS_RESP_RCVD_STATE,
	DDN_REQ_SNT_STATE,
	DDN_ACK_RCVD_STATE = 16,
	MBR_REQ_SNT_STATE,
	MBR_RESP_RCVD_STATE,
	CREATE_BER_REQ_SNT_STATE=19,
	RE_AUTH_ANS_SNT_STATE=20,
	PGWC_NONE_STATE=21,
	CCR_SNT_STATE,
	CREATE_BER_RESP_SNT_STATE,
	PFCP_PFD_MGMT_RESP_RCVD_STATE=24,
	ERROR_OCCURED_STATE,
	UPDATE_BEARER_REQ_SNT_STATE=26,
	UPDATE_BEARER_RESP_SNT_STATE,
	DELETE_BER_REQ_SNT_STATE=28,
	CCRU_SNT_STATE,
	PGW_RSTRT_NOTIF_REQ_SNT_STATE=30,
	UPD_PDN_CONN_SET_REQ_SNT_STATE,
	DEL_PDN_CONN_SET_REQ_SNT_STATE,
	DEL_PDN_CONN_SET_REQ_RCVD_STATE=33,
	PFCP_SESS_SET_DEL_REQ_SNT_STATE,
	PFCP_SESS_SET_DEL_REQ_RCVD_STATE,
    UPF_SETUP_FAILED,
	PFCP_SESS_MOD_REQ_SNT_PRE_CBR_STATE,
	END_STATE
}sm_state;

/* VS: Register different types of events */
typedef enum 
{
	NONE_EVNT,
	CS_REQ_RCVD_EVNT = 1,
	PFCP_ASSOC_SETUP_SNT_EVNT,
	PFCP_ASSOC_SETUP_RESP_RCVD_EVNT=3,
	PFCP_SESS_EST_REQ_RCVD_EVNT,
	PFCP_SESS_EST_RESP_RCVD_EVNT=5,
	CS_RESP_RCVD_EVNT,
	MB_REQ_RCVD_EVNT = 7,
	PFCP_SESS_MOD_REQ_RCVD_EVNT=8,
	PFCP_SESS_MOD_RESP_RCVD_EVNT=9,
	MB_RESP_RCVD_EVNT,
	REL_ACC_BER_REQ_RCVD_EVNT,
	DS_REQ_RCVD_EVNT = 12 ,
	PFCP_SESS_DEL_REQ_RCVD_EVNT,
	PFCP_SESS_DEL_RESP_RCVD_EVNT = 14,
	DS_RESP_RCVD_EVNT=15,
	ECHO_REQ_RCVD_EVNT,
	ECHO_RESP_RCVD_EVNT,
	DDN_ACK_RESP_RCVD_EVNT,
	PFCP_SESS_RPT_REQ_RCVD_EVNT,
	RE_AUTH_REQ_RCVD_EVNT=20,
	CREATE_BER_RESP_RCVD_EVNT,
	CCA_RCVD_EVNT = 22,
	CREATE_BER_REQ_RCVD_EVNT,
	PFCP_PFD_MGMT_RESP_RCVD_EVNT=24,
	ERROR_OCCURED_EVNT,
	UPDATE_BEARER_REQ_RCVD_EVNT,
	UPDATE_BEARER_RSP_RCVD_EVNT = 27,
	DELETE_BER_REQ_RCVD_EVNT,
	DELETE_BER_RESP_RCVD_EVNT = 29,
	DELETE_BER_CMD_RCVD_EVNT,
	CCAU_RCVD_EVNT,
	PGW_RSTRT_NOTIF_ACK_RCVD_EVNT,
	UPD_PDN_CONN_SET_REQ_RCVD_EVNT = 33,
	UPD_PDN_CONN_SET_RESP_RCVD_EVNT,
	DEL_PDN_CONN_SET_REQ_RCVD_EVNT,
	DEL_PDN_CONN_SET_RESP_RCVD_EVNT = 36,
	PFCP_SESS_SET_DEL_REQ_RCVD_EVNT,
	PFCP_SESS_SET_DEL_RESP_RCVD_EVNT,
    SEND_PFCP_DEL_SESSION_REQ, 
    SEND_GTP_DEL_BEARER_REQ,
    PFCP_ASSOCIATION_SETUP,
    PFCP_ASSOCIATION_SETUP_RSP,
	PFCP_ASSOC_SETUP_SUCCESS,
	PFCP_ASSOC_SETUP_FAILED,
    PFCP_SESS_EST_EVNT,
	PFCP_SESS_EST_RESP_TIMEOUT_EVNT,
    GX_CCAI_FAILED,
    DDN_TIMEOUT, 
	BEARER_CREATE_EVNT,
    SEND_RE_AUTH_RSP_EVNT,
    DEFAULT_BEARER_DELETE,
    NW_DETACH_DBREQ_TIMEOUT,
    RCVD_GTP_DEL_BEARER_RSP,
	GX_CCRI_TIMEOUT_EVNT,
	END_EVNT
}sm_event;



#ifdef __cplusplus
}
#endif
#endif
