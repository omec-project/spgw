/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/*
 * Keep only UPF information..NO APIS to be added in this file 
 */
#ifndef __UPF_STRUCT_H
#define __UPF_STRUCT_H
#include <stdint.h>
#include <sys/queue.h>
#include "restoration_timer.h"

#define MAX_HOSTNAME_LENGTH							(256)

typedef enum pfcp_assoc_status_en {
	ASSOC_IN_PROGRESS = 0,
	ASSOC_ESTABLISHED = 1,
} pfcp_assoc_status_en;

/**
 * @brief  : Maintains ue context Bearer identifier and tied
 */
struct ue_context_key {
	/* Bearer identifier */
	uint8_t ebi_index;
	/* UE Context key == teid */
	uint32_t teid;
	/* UE Context key == sender teid */
	uint32_t sender_teid;
	/* UE Context key == sequence number */
	uint32_t sequence;

    LIST_ENTRY(ue_context_key) csrentries;
};
typedef struct ue_context_key context_key;


/**
 * @brief  : Maintains context of upf
 */
typedef struct upf_context_t {
	pfcp_assoc_status_en	assoc_status;
	char	fqdn[MAX_HOSTNAME_LENGTH];

	uint16_t up_supp_features;
	uint8_t  cp_supp_features;
	uint32_t s1u_ip;
	uint32_t s5s8_sgwu_ip;
	uint32_t s5s8_pgwu_ip;
	uint8_t  state;
	/* Add timer_entry for pfcp assoc req */
	peerData *timer_entry;
#ifdef DELETE_THIS
	create_sess_req_t csr;
#endif
    LIST_HEAD(pendingcsrhead, ue_context_key) pendingCSRs;
} upf_context_t;


#endif
