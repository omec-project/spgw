// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "spgw_config.h"
#include "spgw_tables.h"
static spgwTables *table = nullptr; 

extern "C"
{
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <arpa/inet.h>
     #include <stdio.h>
     #include "spgw_cpp_wrapper.h"

    spgw_config_profile_t *parse_subscriber_profiles_c(const char *file)
    {
        spgw_config_profile_t *new_config;
        printf("parse_config [%s] called in the library \n", file);
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
    
    void init_cpp_tables(void)
    {
        table = new spgwTables();
    }

    bool add_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
        return table->add_pfcp_trans(addr, port, msg_seq, trans); 
    }

    void* find_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
        return table->find_pfcp_trans(addr, port, msg_seq); 
    }

    void* delete_pfcp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
        return table->delete_pfcp_trans(addr, port, msg_seq); 
    }

    bool add_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq, void *trans)
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
        return table->add_gtp_trans(addr, port, msg_seq, trans); 
    }

    void* find_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq)
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
        return table->find_gtp_trans(addr, port, msg_seq); 
    }

    void* delete_gtp_transaction(uint32_t addr, uint16_t port, uint32_t msg_seq) 
    {
        struct in_addr test; test.s_addr = addr;
        printf("%s  Addr -  %s, Port - %d, Sequence = %d \n",__FUNCTION__,inet_ntoa(test), htons(port), msg_seq); 
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
}
