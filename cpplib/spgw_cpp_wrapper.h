// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW__CPP_WRAPPER__H
#define __SPGW__CPP_WRAPPER__H
#include "stdbool.h"
#include "stdint.h"
#include "spgw_config_struct.h"
#include "spgwStatsPromEnum.h"
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif


spgw_config_profile_t* parse_subscriber_profiles_c(const char *);

/* API to be called by application to get the profiles */
sub_config_t* match_sub_selection(sub_selection_keys_t *key);

/* API to find matching profile */
apn_profile_t * match_apn_profile(char *, uint16_t len);

int 
get_user_plane_services(user_plane_service_names_t *profile, int max);

void init_cpp_tables(void);
cfg_func_ptr getConfigCallback(void);

bool add_pfcp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans);
void* find_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq);
void* delete_pfcp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq);

bool add_gtp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans);
void* find_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq);
void* delete_gtp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq);

bool add_gx_transaction(uint32_t msg_seq, void *trans);
void* find_gx_transaction(uint32_t msg_seq);
void* delete_gx_transaction(uint32_t msg_seq);


void queue_stack_unwind_event_cpp(void *context); 
void *get_stack_unwind_event_cpp(void);

void *get_t2tMsg(void);

void queue_test_stack_unwind_event_cpp(void *context);
void *get_test_stack_unwind_event_cpp(void);

void queue_gtp_out_event_cpp(void *context);
void *get_gtp_out_event(void);

void queue_pfcp_out_event_cpp(void *context);
void *get_pfcp_out_event(void);

void queue_gx_out_event_cpp(void *context);
void *get_gx_out_event(void);

/* Prometheus APIs */
void decrement_stat(int stat_id);
void increment_stat(int stat_id);
void set_num_ue_stat(int stat_id, uint32_t val);
void setup_prometheus(uint16_t port);
void setup_webserver(uint16_t port, cfg_func_ptr val);
void increment_userplane_stats(int stat_id, uint32_t peer_addr);
void increment_mme_peer_stats(int stat_id, uint32_t peer_addr);
void increment_sgw_peer_stats(int stat_id, uint32_t peer_addr);
void increment_pgw_peer_stats(int stat_id, uint32_t peer_addr);
void increment_gx_peer_stats(int stat_id, uint32_t peer_addr);
/* Procedure stats start */
void increment_proc_mme_peer_stats_reason(int stat_id, uint32_t peer_addr, uint32_t reason, uint16_t tac);
void increment_proc_mme_peer_stats(int stat_id, uint32_t peer_addr, uint16_t tac);
/* Procedure stats end */
void increment_ue_info_stats(int stat_id, uint64_t imsi, uint32_t ipv4); 
void decrement_ue_info_stats(int stat_id, uint64_t imsi, uint32_t ipv4); 

void set_data_stats(int stat_id, uint64_t imsi, uint32_t bytes);

/***************UPF Table *********************************/
int
upf_context_entry_add(uint32_t *upf_ip, void *entry);

void*
upf_context_entry_lookup(uint32_t upf_ip);

int
upf_context_delete_entry(uint32_t upf_ip);

int
upf_context_entry_add_service(const char *upf, void *entry);

void*
upf_context_entry_lookup_service(const char *upf);

int
upf_context_delete_entry_service(const char *upf);

/*****************UPF Table end *******************************/

/**************Bearer Table **********************************/
int 
bearer_context_entry_add_teidKey(uint32_t teid, void *context);

void*
get_bearer_by_teid(uint32_t teid);

int 
bearer_context_delete_entry_teidKey(uint32_t teid);
/*****************Bearer Table end *******************************/

int
add_pdn_conn_entry(uint32_t call_id, void *pdn);

void*
get_pdn_conn_entry(uint32_t call_id);

int
del_pdn_conn_entry(uint32_t call_id);

int
pdn_context_entry_add_teidKey(uint32_t teid, void *pdn);

void*
get_pdn_context(uint32_t teid);

int
pdn_context_delete_entry_teidKey(uint32_t teid);

/* UE APIs */
int
add_sess_entry_seid(uint64_t sess_id, void *context);

void*
get_sess_entry_seid(uint64_t sess_id);

int
del_sess_entry_seid(uint64_t sess_id);

int 
ue_context_entry_add_teidKey(uint32_t teid, void *entry);

void* 
get_ue_context(uint32_t teid);

int 
ue_context_delete_entry_teidKey(uint32_t teid);

int 
ue_context_entry_add_imsiKey(uint64_t imsi, void *context);

void*
ue_context_entry_lookup_imsiKey(uint64_t imsi);

int 
ue_context_delete_entry_imsiKey(uint64_t imsi);

// peer table APIS 

int 
add_peer_entry(uint32_t ipaddr, void *peer);

void*
get_peer_entry(uint32_t ipaddr);

int 
del_entry_from_hash(uint32_t ipAddr);

int 
peer_heartbeat_entry_lookup(uint32_t peer_ip, uint32_t *recov_time);

void 
add_ip_to_heartbeat_hash(struct sockaddr_in *peer_addr, uint32_t recovery_time);

void 
delete_entry_heartbeat_hash(struct sockaddr_in *peer_addr);

/* URR APIS */

int
add_urr_entry(uint32_t urr_id, void *cntxt);

void*
get_urr_entry(uint32_t urr_id);

int
del_urr_entry(uint32_t urr_id);
/* URR APIS end */

/* QER APIS */
int
add_qer_entry(uint32_t qer_id, void *cntxt);

void*
get_qer_entry(uint32_t qer_id);

int
del_qer_entry(uint32_t qer_id);
/* QER APIS end */

/* PDR APIS end */

int
add_pdr_entry(uint16_t rule_id, void *cntxt);

void* 
get_pdr_entry(uint16_t rule_id);

int
del_pdr_entry(uint16_t rule_id);

int 
add_rule_name_entry(const char *rule_name, uint8_t bearer);

int8_t 
get_rule_name_entry(const char *rule_name);

int 
del_rule_name_entry(const char *rule_name);


/* Gx APIS */

int 
gxsessid_context_entry_add(const uint8_t *sess_id, void *context);

void*
get_ue_context_from_gxsessid(const uint8_t *sessid);

int
remove_gxsessid_to_context(const uint8_t *sessid);

uint32_t
acquire_ip_cpp(const char *pool);

void
release_ip_cpp(const char *pool, struct in_addr ipv4);

bool
reserve_static_ip_cpp(const char *pool_n, struct in_addr host);

bool 
release_static_ip_cpp(const char *pool_n, struct in_addr addr);

int parse_cp_json(cp_config_t *cfg, const char *file);

void add_delayed_free_memory_task(void *data);

void* delete_delayed_free_memory_task(int *size);

#ifdef __cplusplus
}
#endif
#endif
