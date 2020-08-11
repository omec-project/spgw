// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/*
 * Keep only UPF information..NO APIS to be added in this file 
 */
#ifndef __UPF_STRUCT_H
#define __UPF_STRUCT_H
#include <stdint.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include "cp_timer.h"
#include "cp_peer_struct.h"
#include "trans_struct.h"
struct dp_info;

#define MAX_HOSTNAME_LENGTH							(256)

struct pending_proc_key {
    void *proc_context;
    LIST_ENTRY(pending_proc_key) procentries;
};
typedef struct pending_proc_key pending_proc_key_t;


#define GET_UPF_ADDR(upf)  (upf->upf_sockaddr.sin_addr.s_addr)
/**
 * @brief  : Maintains context of upf
 */
typedef struct upf_context {
    struct sockaddr_in upf_sockaddr;
	char fqdn[MAX_HOSTNAME_LENGTH];
	uint16_t up_supp_features;
	uint8_t  cp_supp_features;
	uint32_t s1u_ip;
	uint32_t s5s8_sgwu_ip;
	uint32_t s5s8_pgwu_ip;
	uint8_t  state;
    transData_t *trans_entry; /* association setup req/rsp transaction */
    LIST_HEAD(pendingprochead, pending_proc_key) pendingProcs;
} upf_context_t;

#endif
