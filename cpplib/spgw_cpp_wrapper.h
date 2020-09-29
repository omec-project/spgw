// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW__CPP_WRAPPER__H
#define __SPGW__CPP_WRAPPER__H
#include "spgw_config_struct.h"
#include "spgwStatsPromEnum.h"

spgw_config_profile_t* parse_subscriber_profiles_c(const char *);

/* API to set the config handler in library */
void set_cp_config(spgw_config_profile_t *);

/* When config is modified, this api is used to switch from old to new config
 * and also old config is deleted 
 */
void switch_config(spgw_config_profile_t *);

/* API to be called by application to get the profiles */
sub_profile_t* match_sub_selection(sub_selection_keys_t *key); 

/* API to find matching profile */
apn_profile_t * match_apn_profile(char *, uint16_t len);

void invalidate_upf_dns_results(uint32_t ip);
void init_cpp_tables(void);
bool add_pfcp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans);
void* find_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq);
void* delete_pfcp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq);
bool add_gtp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans);
void* find_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq);
void* delete_gtp_transaction(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq);
void queue_stack_unwind_event_cpp(void *context); 
void *get_stack_unwind_event_cpp(void);
void *get_t2tMsg(void);

/* Prometheus APIs */
void decrement_stat(int stat_id);
void increment_stat(int stat_id);
void set_num_ue_stat(int stat_id, uint32_t val);
void setup_prometheus(uint16_t port);
void setup_webserver(uint16_t port);
void increment_userplane_stats(int stat_id, uint32_t peer_addr);
void increment_mme_peer_stats(int stat_id, uint32_t peer_addr);
void increment_sgw_peer_stats(int stat_id, uint32_t peer_addr);
void increment_pgw_peer_stats(int stat_id, uint32_t peer_addr);
void increment_gx_peer_stats(int stat_id, uint32_t peer_addr);
void increment_proc_mme_peer_stats_reason(int stat_id, uint32_t peer_addr, uint32_t reason);
void increment_proc_mme_peer_stats(int stat_id, uint32_t peer_addr);
void set_data_stats(int stat_id, uint64_t imsi, uint32_t bytes);

#endif
