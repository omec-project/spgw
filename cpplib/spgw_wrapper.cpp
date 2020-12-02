// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "spgw_config.h"
#include "spgw_tables.h"
#include "spgwStatsPromClient.h"
#include <thread>
#include <sstream>
#include "spgw_webserver.h"

spgwTables *table = nullptr; 

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

    void set_cp_config(spgw_config_profile_t *new_config)
    {
        spgwConfig::set_cp_config_cpp(new_config);
    }

    void switch_config(spgw_config_profile_t *new_config)
    {
        spgwConfig::switch_config_cpp(new_config);
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

    void init_cpp_tables()
    {
        table = new spgwTables();
    }

    bool add_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->add_pfcp_trans(addr, port, msg_seq, trans); 
    }

    void* find_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->find_pfcp_trans(addr, port, msg_seq); 
    }

    void* delete_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->delete_pfcp_trans(addr, port, msg_seq); 
    }

    bool add_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->add_gtp_trans(addr, port, msg_seq, trans); 
    }

    void* find_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
        return table->find_gtp_trans(addr, port, msg_seq); 
    }

    void* delete_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(*((struct in_addr *)&addr)), htons(port), msg_seq); 
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
        std::cout<<"setting data usage imsi ["<<imsi_s.str()<<"] = "<<bytes<<std::endl;
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

    void increment_proc_mme_peer_stats_reason(int stat_id, uint32_t peer_addr, uint32_t reason)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
       std::stringstream r_string;
       r_string<<reason;
	   spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}, {"reason", r_string.str()}});
    }

    void increment_proc_mme_peer_stats(int stat_id, uint32_t peer_addr)
    {
       spgwStatsCounter id = static_cast<spgwStatsCounter>(stat_id); 
	   spgwStats::Instance()->increment(id, {{"mme_addr",inet_ntoa(*((struct in_addr *)&peer_addr))}});
    }

}
