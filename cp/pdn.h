// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef PDN_H
#define PDN_H
/**
 * @file
 *
 * Contains all data structures required by 3GPP TS 23.401 Tables 5.7.3-1 and
 * 5.7.4-1 (that are nessecary for current implementaiton) to describe the
 * Connections, state, bearers, etc as well as functions to manage and/or
 * obtain value for their fields.
 *
 */

#include <stdint.h>
#include <arpa/inet.h>
#include "cp_common.h"
#include "gtpv2_ie.h"
#include "cp_interface.h"
#include "pfcp_cp_struct.h"
#include "cp_timer.h"
#include "cp_peer_struct.h"
#include "upf_struct.h"
#include "spgw_config_struct.h"
#include "proc_struct.h"
#include <sys/queue.h>
#include "sm_struct.h"
#include "bearer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dynamic_rule;
struct ue_context; 
enum rule_action_t {
	RULE_ACTION_INVALID,
	RULE_ACTION_ADD = 1,
	RULE_ACTION_MODIFY = 2,
	RULE_ACTION_DELETE = 3,
	RULE_ACTION_MAX
};

#define PCC_RULE_OP_PENDING  0x00000001
typedef struct pcc_rule{
    uint32_t            flags;
	enum rule_action_t action;
	struct dynamic_rule *dyn_rule;
    TAILQ_ENTRY(pcc_rule) next_pcc_rule;
}pcc_rule_t;
/* Currently policy from PCRF can be two thing
 * 1. Default bearer QOS
 * 2. PCC Rule
 * Default bearer QOS can be modified
 * PCC Rules can be Added, Modified or Deleted
 * These policy shoulbe be applied to the PDN or eps_bearer
 * data strutures only after sucess from access side
 */
typedef struct policy {
    /*GXCLEAN : get rid of unwanted fields in this struct */
	bool default_bearer_qos_valid;
	bearer_qos_ie default_bearer_qos;
	uint8_t count;
    TAILQ_HEAD(pending_pcc_head, pcc_rule) pending_pcc_rules; /* list of uninstalled pcc rules */ 
	//pcc_rule_t pcc_rule[32]; /* GXCLEAN - remove this structure. For now its kept just to keep compilation happy  */
}policy_t;


typedef struct ue_tz
{
	uint8_t tz;
	uint8_t dst;
}ue_tz_t;

enum PDN_CONTEXT_STATE {
    PDN_STATE_UNKNOWN,
    PDN_STATE_ACTIVE_PENDING,
    PDN_STATE_ACTIVE,
    PDN_STATE_PAGING_PENDING,
    PDN_STATE_DETACH_PENDING,
    PDN_STATE_IDLE_PENDING,
    PDN_STATE_IDLE,
    PDN_STATE_DETACH
};


#define PDN_STATIC_ADDR         0x00000001 /* UE address is fixed/static and UE pool controlled by control plane */
#define PDN_ADDR_ALLOC_CONTROL  0x00000002 /* Control Plane has allocated UE address */
#define PDN_ADDR_ALLOC_UPF      0x00000004 /* User Plane has allocated UE address */

#define SET_PDN_ADDR_STATIC(pdn,flag)  \
do { \
    if(flag == true) { \
      pdn->pdn_flags = pdn->pdn_flags | PDN_STATIC_ADDR;\
    }\
}\
while(0);

#define IF_PDN_ADDR_STATIC(pdn)   ((pdn->pdn_flags & PDN_STATIC_ADDR) == PDN_STATIC_ADDR)
#define SET_PDN_ADDR_METHOD(pdn, flag) (pdn->pdn_flags = pdn->pdn_flags | flag)
#define RESET_PDN_ADDR_METHOD(pdn, flag) (pdn->pdn_flags = pdn->pdn_flags & (~flag))
#define IF_PDN_ADDR_ALLOC_CONTROL(pdn) (pdn->pdn_flags & PDN_ADDR_ALLOC_CONTROL)
#define IF_PDN_ADDR_ALLOC_UPF(pdn) (pdn->pdn_flags & PDN_ADDR_ALLOC_UPF)

/**
 * @brief  : Maintains pdn connection information
 */
struct pdn_connection {
	uint8_t state;
	uint8_t bearer_control_mode;

	/*VS : Call ID ref. to session id of CCR */
	uint32_t call_id;


	uint8_t apn[MAX_APN_LEN];
	uint8_t apn_len;

	ambr_ie apn_ambr;
	uint32_t apn_restriction;

	ambr_ie session_ambr;
	ambr_ie session_gbr;

	struct in_addr upf_ipv4;
	uint64_t seid;
	uint64_t dp_seid;

	struct in_addr ipv4;
	struct in6_addr ipv6;

	/* VS: Need to Discuss teid and IP should be part of UE context */
	uint32_t s5s8_sgw_gtpc_teid;
	struct in_addr s5s8_sgw_gtpc_ipv4;

	bool old_sgw_addr_valid;
	struct in_addr old_sgw_addr;

	uint32_t s5s8_pgw_gtpc_teid;
	struct in_addr s5s8_pgw_gtpc_ipv4;

	uint8_t ue_time_zone_flag;
	ue_tz_t ue_tz;
	ue_tz_t old_ue_tz;
	bool old_ue_tz_valid;

	uint8_t rat_type;
	uint8_t old_ret_type;
	bool old_rat_type_valid;


	/* VS: Support partial failure functionality of FQ-CSID */

	pdn_type_ie pdn_type;
	/* See  3GPP TS 32.298 5.1.2.2.7 for Charging Characteristics fields*/
	charging_characteristics_ie charging_characteristics;

	uint8_t default_bearer_id;
	/* VS: Need to think on it */
	uint8_t num_bearer;

	/* VS: Create a cyclic linking to access the data structures of UE */
	struct ue_context *context;

	uint8_t fqdn[FQDN_LEN];
	struct eps_bearer *eps_bearers[MAX_BEARERS]; /* index by ebi - 1 */

	struct eps_bearer *packet_filter_map[MAX_FILTERS_PER_UE];

	char gx_sess_id[MAX_SESS_ID_LEN];
	struct dynamic_rule *dynamic_rules[16];

	/* need to maintain reqs ptr for RAA*/
    // FIXME : this should be just pointer..
	policy_t policy;

	/* timer entry data for stop timer session */
	peerData_t *timer_entry;
    transData_t *trans_entry;

    uint32_t pdn_flags; 
};

typedef struct pdn_connection pdn_connection_t;


/* PDN APIs */
void cleanup_pdn_context(pdn_connection_t *pdn);

eps_bearer_t *
get_default_bearer(pdn_connection_t *pdn);

eps_bearer_t *
get_bearer(pdn_connection_t *pdn, bearer_qos_ie *qos);

int8_t
get_new_bearer_id(pdn_connection_t *pdn_cntxt);

int8_t
compare_default_bearer_qos(bearer_qos_ie *default_bearer_qos,
		bearer_qos_ie *rule_qos);

#ifdef __cplusplus
}
#endif

#endif /* PDN_H */
