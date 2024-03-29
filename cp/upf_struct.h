// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

/*
 * Keep only UPF information..NO APIS to be added in this file 
 */
#ifndef __UPF_STRUCT_H
#define __UPF_STRUCT_H
#include <stdint.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include "cp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HOSTNAME_LENGTH							(256)

struct pending_proc_key {
    void *proc_context;
    LIST_ENTRY(pending_proc_key) procentries;
};
typedef struct pending_proc_key pending_proc_key_t;


#define GET_UPF_ADDR(upf)  (upf->upf_sockaddr.sin_addr.s_addr)
#define IS_UPF_SUPP_FEAT_UEIP(upf) (upf->add_up_supp_features1 & 0x0400)
/**
 * @brief  : Maintains context of upf
 */
typedef struct upf_context {
    struct sockaddr_in upf_sockaddr;
	char fqdn[MAX_HOSTNAME_LENGTH];
	uint16_t up_supp_features;
	uint16_t add_up_supp_features1;
	uint16_t add_up_supp_features2;
	uint8_t  cp_supp_features;
	uint32_t s1u_ip;
	uint32_t s5s8_sgwu_ip;
	uint32_t s5s8_pgwu_ip;
	uint8_t  state;
    void     *proc; // association setup procedure  
    gstimerinfo_t  upf_pt;
    LIST_HEAD(pending_sub_procs_head, pending_proc_key) pending_sub_procs;
    TAILQ_HEAD(upf_procs_head, proc_context) pending_node_procs;
    bool     global_address; /* true : control plane allocates address, false : upf allocates address */
} upf_context_t;

#ifdef __cplusplus
}
#endif
#endif
