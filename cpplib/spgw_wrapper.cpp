// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "spgw_config.h"
#include "spgw_tables.h"
#include "upf_tables.h"
#include "bearer_tables.h"
#include "pdn_tables.h"
#include "ue_tables.h"
#include "peer_tables.h"
#include "dns_upf_tables.h"
#include "urr_tables.h"
#include "qer_tables.h"
#include "pdr_tables.h"
#include "rule_tables.h"
#include "spgwStatsPromClient.h"
#include <thread>
#include <sstream>
#include "spgw_webserver.h"
#include "ip_pool_mgmt.h"
#include "delayed_task.h"

spgwTables      *table = nullptr; 
upfTables       *upf_table = nullptr;
bearerTables    *bearer_table = nullptr;
pdnTables       *pdn_table = nullptr;
ueTables        *ue_table  = nullptr;
peerTables      *peer_table = nullptr;
dnsUpfTables    *dns_upf_table = nullptr;
urrTables       *urr_table = nullptr; 
qerTables       *qer_table = nullptr; 
pdrTables       *pdr_table = nullptr; 
ruleTables      *rule_table = nullptr; 

extern "C"
{
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include "spgw_cpp_wrapper.h"
    #include "spgwStatsPromEnum.h"
	#include "cp_log.h"

    spgw_config_profile_t *parse_subscriber_profiles_c(const char *file)
    {
        spgw_config_profile_t *new_config;
        new_config = spgwConfig::parse_subscriber_profiles_cpp(file);
        return new_config;
    }

    sub_profile_t *match_sub_selection(sub_selection_keys_t *key)
    {
        return spgwConfig::match_sub_selection_cpp(key);
    }
    
    apn_profile_t* match_apn_profile(char *apn, uint16_t len)
    {
        return spgwConfig::match_apn_profile_cpp(apn, len);
    }
    
    void invalidate_upf_dns_results(uint32_t ip) 
    {
        spgwConfig::invalidate_user_plane_address(ip);
    }
    
    user_plane_profile_t* get_user_plane_profile_ref(const char *name)
    {
        return spgwConfig::get_user_plane_profile_ref(name);
    }

    int get_user_plane_profiles(profile_names_t *ptr, int max_size) 
    {
        return spgwConfig::get_user_plane_profiles(ptr, max_size);
    }
    
    int parse_cp_json(cp_config_t *cfg, const char *file)
    {
        return spgwConfig::parse_cp_json_cpp(cfg, file);
    }

    void init_cpp_tables()
    {
        table = new spgwTables();
        upf_table = new upfTables();
        bearer_table = new bearerTables();
        pdn_table = new pdnTables();
        ue_table  = new ueTables();
        peer_table = new peerTables();
        dns_upf_table = new dnsUpfTables();
        urr_table     = new urrTables();
        qer_table     = new qerTables();
        pdr_table     = new pdrTables();
        rule_table    = new ruleTables();
    }

    bool add_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        LOG_MSG(LOG_DEBUG5,"Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->add_pfcp_trans(addr, port, msg_seq, trans); 
    }

    void* find_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        LOG_MSG(LOG_DEBUG5,"Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->find_pfcp_trans(addr, port, msg_seq); 
    }

    void* delete_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        LOG_MSG(LOG_DEBUG5, "Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->delete_pfcp_trans(addr, port, msg_seq); 
    }

    bool add_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        LOG_MSG(LOG_DEBUG5, "Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->add_gtp_trans(addr, port, msg_seq, trans); 
    }

    void* find_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        LOG_MSG(LOG_DEBUG5, "Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->find_gtp_trans(addr, port, msg_seq); 
    }

    void* delete_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        LOG_MSG(LOG_DEBUG5, "Addr -  %s, Port - %d, Sequence = %d ",inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->delete_gtp_trans(addr, port, msg_seq); 
    }

    void queue_stack_unwind_event_cpp(void *context) 
    {
        return table->queue_event(context);
    }

    void *get_stack_unwind_event_cpp(void) 
    {
        return table->pop_event();
    }

    void queue_test_stack_unwind_event_cpp(void *context)
    {
        return table->queue_test_event(context);
    }

    void *get_test_stack_unwind_event_cpp(void)
    {
        return table->pop_test_event();
    }

    void queue_gtp_out_event_cpp(void *context)
    {
        return table->queue_gtp_out_event(context);
    }

    void *get_gtp_out_event(void)
    {
        return table->pop_gtp_out_event();
    }

    void queue_pfcp_out_event_cpp(void *context)
    {
        return table->queue_pfcp_out_event(context);
    }

    void *get_pfcp_out_event(void)
    {
        return table->pop_pfcp_out_event();
    }

    void queue_gx_out_event_cpp(void *context)
    {
        return table->queue_gx_out_event(context);
    }

    void *get_gx_out_event(void)
    {
        return table->pop_gx_out_event();
    }

    void *get_t2tMsg() 
    {
        return table->pop_t2t_msg_event();
    }

    void setup_prometheus(uint16_t port)
    {
        std::thread prom(spgwStatsSetupPrometheusThread, port);
        prom.detach();
    }

    void setup_webserver(uint16_t port)
    {
        std::thread webserver(spgwWebserverThread, port);
        webserver.detach();
    }


    void increment_mme_peer_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       
	   spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }

    void increment_sgw_peer_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       
	   spgwStats::Instance()->increment(id, {{"sgw_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }

    void increment_pgw_peer_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       
	   spgwStats::Instance()->increment(id, {{"pgw_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }

    void increment_userplane_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       
	   spgwStats::Instance()->increment(id, {{"spgwu_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }

    void set_data_stats(int stat_id, uint64_t imsi, uint32_t bytes)
    {
        spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
        std::stringstream imsi_s;
        imsi_s << imsi; 
        LOG_MSG(LOG_DEBUG, "setting data usage imsi [%s], bytes %u ",imsi_s.str().c_str(), bytes);
        spgwStats::Instance()->set(id, bytes, {{"imsi",imsi_s.str()}});
    }

    void increment_gx_peer_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
	   spgwStats::Instance()->increment(id, {{"pcrf_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }


    void decrement_stat(int stat_id) 
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
	   spgwStats::Instance()->decrement(id);
    }

    void increment_stat(int stat_id)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
	   spgwStats::Instance()->increment(id);
    }

    void set_num_ue_stat(int stat_id, uint32_t val)
    {
        spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
        spgwStats::Instance()->set(id, val);
    }

    void increment_proc_mme_peer_stats_reason(int stat_id, uint32_t peer_addr, uint32_t reason, uint16_t tac)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       std::stringstream r_string;
       r_string<<reason;
       if(tac) {
            std::stringstream tac_s;
            tac_s<<tac;
	        spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}, {"reason", r_string.str()}, {"tac",tac_s.str()}});
       } else {
	        spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}, {"reason", r_string.str()}});
       }
    }

    void increment_proc_mme_peer_stats(int stat_id, uint32_t peer_addr, uint16_t tac)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       if(tac) {
            std::stringstream tac_s;
            tac_s<<tac;
	        spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))},{"tac",tac_s.str()}});
       } else {
	        spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
       }
    }

    void increment_ue_info_stats(int stat_id, uint64_t imsi, uint32_t ipv4) 
    {
        ipv4 = htonl(ipv4);
        spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
        spgwStats::Instance()->increment(id, {{"imsi",std::to_string(imsi)}, {"mobile_ip",inet_ntoa(*((struct in_addr *)&ipv4))}}); 
        return;
    }

    void decrement_ue_info_stats(int stat_id, uint64_t imsi, uint32_t ipv4) 
    {
        ipv4 = htonl(ipv4);
        spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
        spgwStats::Instance()->decrement(id, {{"imsi",std::to_string(imsi)}, {"mobile_ip",inet_ntoa(*((struct in_addr *)&ipv4))}}); 
        return;
    }

    /* UPF APIS... Start */
    int upf_context_entry_add(uint32_t *upf_ip, void *entry)
    {
        return upf_table->add_upf(upf_ip, entry);
    }
    
    void* upf_context_entry_lookup(uint32_t upf_ip)
    {
        return upf_table->find_upf(upf_ip);
    }
    
    int upf_context_delete_entry(uint32_t upf_ip)
    {
        return upf_table->delete_upf(upf_ip);
    }
    /* UPF APIS... end */

    /* Bearer APIS... Start */
    int bearer_context_entry_add_teidKey(uint32_t teid, void *entry)
    {
        return bearer_table->add_teid_to_bearer_mapping(teid, entry);
    }
    
    void* get_bearer_by_teid(uint32_t teid)
    {
        return bearer_table->find_bearer_from_teid(teid);
    }
    
    int bearer_context_delete_entry_teidKey(uint32_t teid)
    {
        return bearer_table->delete_bearer_teid_mapping(teid);
    }
    /* Bearer APIS... end */

    /* PDN APIS... Start */
    int add_pdn_conn_entry(uint32_t call_id, void *entry)
    {
        return pdn_table->add_callid_to_pdn_mapping(call_id, entry);
    }
    
    void* get_pdn_conn_entry(uint32_t call_id)
    {
        return pdn_table->find_pdn_from_callid(call_id);
    }

    int del_pdn_conn_entry(uint32_t call_id)
    {
        return pdn_table->delete_callid_pdn_mapping(call_id);
    }

    int pdn_context_entry_add_teidKey(uint32_t teid, void *entry)
    {
        return pdn_table->add_teid_to_pdn_mapping(teid, entry);
    }
    
    void* get_pdn_context(uint32_t teid)
    {
        return pdn_table->find_pdn_from_teid(teid);
    }

    int pdn_context_delete_entry_teidKey(uint32_t teid)
    {
        return pdn_table->delete_teid_pdn_mapping(teid);
    }
    /* PDN APIS... end */

    /* UE APIS... start */
    int add_sess_entry_seid(uint64_t seid, void *entry)
    {
        return ue_table->add_seid_to_ue_mapping(seid, entry);
    }
    
    void* get_sess_entry_seid(uint64_t seid)
    {
        return ue_table->find_ue_from_seid(seid);
    }

    int del_sess_entry_seid(uint64_t seid)
    {
        return ue_table->delete_seid_ue_mapping(seid);
    }

    int ue_context_entry_add_teidKey(uint32_t teid, void *entry)
    {
        return ue_table->add_teid_to_ue_mapping(teid, entry);
    }
    
    void* get_ue_context(uint32_t teid)
    {
        return ue_table->find_ue_from_teid(teid);
    }

    int ue_context_delete_entry_teidKey(uint32_t teid)
    {
        return ue_table->delete_teid_ue_mapping(teid);
    }

    int ue_context_entry_add_imsiKey(uint64_t imsi, void *entry)
    {
        return ue_table->add_imsi_to_ue_mapping(imsi, entry);
    }
    
    void* ue_context_entry_lookup_imsiKey(uint64_t imsi)
    {
        return ue_table->find_ue_from_imsi(imsi);
    }

    int ue_context_delete_entry_imsiKey(uint64_t imsi)
    {
        return ue_table->delete_imsi_ue_mapping(imsi);
    }

    /* UE APIS... end */
    /* Peer APIs start */
    int add_peer_entry(uint32_t peer_addr, void *entry)
    {
        return peer_table->add_addr_to_peer_mapping(peer_addr, entry);
    }
    
    void* get_peer_entry(uint32_t peer_addr)
    {
        return peer_table->find_peer_from_addr(peer_addr);
    }

    int del_entry_from_hash(uint32_t peer_addr)
    {
        return peer_table->delete_addr_peer_mapping(peer_addr);
    }

    int peer_heartbeat_entry_lookup(uint32_t peer_ip, uint32_t *recov_time)
    {
        return peer_table->find_peer_recov_time(peer_ip, recov_time);
    }

    void add_ip_to_heartbeat_hash(struct sockaddr_in *peer_addr, uint32_t recovery_time)
    {
        peer_table->add_peer_addr_to_recov_time_mapping(peer_addr->sin_addr.s_addr, recovery_time);
        return ;
    }

    void delete_entry_heartbeat_hash(struct sockaddr_in *peer_addr)
    {
        peer_table->reset_peer_recov_time(peer_addr->sin_addr.s_addr);
        return;
    }

    /* Peer APIs end */
    
    /* IMSI to DNS_UPF APIs end */

    int upflist_by_ue_hash_entry_add(uint64_t imsi, void *entry)
    {
        return dns_upf_table->add_imsi_to_upfdns_mapping(imsi, entry);
    }
    
    void* upflist_by_ue_hash_entry_lookup(uint64_t imsi)
    {
        return dns_upf_table->find_upfdns_from_imsi(imsi);
    }

    int upflist_by_ue_hash_entry_delete(uint64_t imsi)
    {
        return dns_upf_table->delete_imsi_upfdns_mapping(imsi);
    }
    /* IMSI to DNS_UPF APIs end*/

    /* URR APIs */
    int add_urr_entry(uint32_t urr_id, void *cntxt) 
    {
        return urr_table->add_urrid_to_urrctxt_mapping(urr_id, cntxt);
    }

    void* get_urr_entry(uint32_t urr_id)
    {
        return urr_table->find_urrctxt_from_urrid(urr_id);
    }

    int del_urr_entry(uint32_t urr_id)
    {
        return urr_table->delete_urrid_urrctxt_mapping(urr_id);
    }
    /* URR APIs end */

    /* QER APIs */
    int add_qer_entry(uint32_t qer_id, void *cntxt) 
    {
        return qer_table->add_qerid_to_qerctxt_mapping(qer_id, cntxt);
    }

    void* get_qer_entry(uint32_t qer_id)
    {
        return qer_table->find_qerctxt_from_qerid(qer_id);
    }

    int del_qer_entry(uint32_t qer_id)
    {
        return qer_table->delete_qerid_qerctxt_mapping(qer_id);
    }
    /* QER APIs end */

    /* PDRs API */
    int add_pdr_entry(uint16_t pdr_id, void *cntxt)
    {
        return pdr_table->add_pdrid_to_pdrctxt_mapping(pdr_id, cntxt);
    }

    void*  get_pdr_entry(uint16_t pdr_id)
    {
        return pdr_table->find_pdrctxt_from_pdrid(pdr_id);
    }

    int del_pdr_entry(uint16_t pdr_id)
    {
        return pdr_table->delete_pdrid_pdrctxt_mapping(pdr_id);
    }
    /* PDRs API end */

    /* Rules APIs */
    int add_rule_name_entry(const char *rule_name, uint8_t bearer)
    {
        return rule_table->add_rulename_to_bearerid_mapping(rule_name, bearer);
    }

    int8_t get_rule_name_entry(const char *rule_name)
    {
        return rule_table->find_bearerid_from_rulename(rule_name);
    }

    int del_rule_name_entry(const char *rule_name)
    {
        return rule_table->delete_bearerid_rulename_mapping(rule_name);
    }
    /* Rules APIs end */
    
    /* Gx context APIs start */

    int gxsessid_context_entry_add(const uint8_t *sess_id, void *context)
    {
        return ue_table->add_gx_sessid_to_ue_mapping(sess_id, context);
    }

    void* get_ue_context_from_gxsessid(const uint8_t *sess_id) 
    {
        return ue_table->find_ue_from_gx_sessid(sess_id);
    }

    int remove_gxsessid_to_context(const uint8_t *sess_id)
    {
        return ue_table->delete_gx_sessid_ue_mapping(sess_id);
    }

    /* Dynamic Pool APIs */
    uint32_t acquire_ip_cpp(const char *pool_n)
    {
        std::string pool_name(pool_n); 
        ue_pool *pool = ip_pools::getInstance()->getIpPool(pool_name);
        return pool->get_ip();
    }

    /* Dynamic Pool APIs */
    void release_ip_cpp(const char *pool_n, struct in_addr addr)
    {
        std::string pool_name(pool_n); 
        ue_pool *pool = ip_pools::getInstance()->getIpPool(pool_name);
        pool->free_ip(addr.s_addr);
        return;
    }
    /* Static Pool APIs */
    bool reserve_static_ip_cpp(const char *pool_n, struct in_addr host)
    {
        std::string pool_name(pool_n); 
        ue_pool *pool = ip_pools::getInstance()->getIpPool(pool_name);
        return pool->reserve_ip_node(host);
    }

    /* Static Pool APIs */
    bool release_static_ip_cpp(const char *pool_n, struct in_addr addr)
    {
        std::string pool_name(pool_n); 
        ue_pool *pool = ip_pools::getInstance()->getIpPool(pool_name);
        return pool->release_ip_node(addr);
    }

    void add_delayed_free_memory_task(void *trans)
    {
        delayTask *instance = delayTask::Instance();
        instance->add_delayed_free_memory_task(trans); 
        return;
    }

    void* delete_delayed_free_memory_task(int *size)
    {
        delayTask *instance = delayTask::Instance();
        return instance->delete_delayed_free_memory_task(size); 
    }
}
