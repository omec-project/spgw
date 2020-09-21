// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW_TABLES_H__
#define __SPGW_TABLES_H__
#include <iostream>
#include <map>
#include <queue>

struct transKey {
    uint32_t src_port;
    uint32_t src_ip;
    uint32_t seq_num;
};
typedef struct transKey transKey_t;

struct comp
{
    bool operator()(const struct transKey &lhs, const struct transKey &rhs) const
    {
#ifdef DEBUG_LOG_CPP
        std::cout<<std::endl<<"**Compare function called "<<std::endl;
#endif
        if(lhs.src_port == rhs.src_port)
        {
#ifdef DEBUG_LOG_CPP
            std::cout<<"Port Match found "<<std::endl;
#endif
            if(lhs.src_ip == rhs.src_ip)
            {
#ifdef DEBUG_LOG_CPP
                std::cout<<"Source Match found "<<std::endl;
#endif
                if(lhs.seq_num == rhs.seq_num)
                {
#ifdef DEBUG_LOG_CPP
                    std::cout<<"Seq num Match found "<<std::endl;
#endif
                    return false;
                }
#ifdef DEBUG_LOG_CPP
                std::cout<<"seq num not matched "<<std::endl;
#endif
                return lhs.seq_num < rhs.seq_num;
            }
#ifdef DEBUG_LOG_CPP
            std::cout<<"IP address not matched "<<std::endl;
#endif
            return lhs.src_ip < rhs.src_ip;
        }
#ifdef DEBUG_LOG_CPP
        std::cout<<"port number not matched "<<std::endl;
#endif
        return lhs.src_port < rhs.src_port;
    }
};

#define CONFIG_CHANGE_NOTIFICATION   0x01

class spgwTables
{
   public:
      spgwTables()
      {
        spgw_pfcp_transaction_map.clear();
        spgw_gtp_transaction_map.clear();
      }
      ~spgwTables()
      {
        /* need handling ? */
        spgw_pfcp_transaction_map.clear();
        spgw_gtp_transaction_map.clear();
      }


     std::map<transKey_t, void *, comp> spgw_pfcp_transaction_map; 
     std::map<transKey_t, void *, comp> spgw_gtp_transaction_map; 
     std::queue<void *> stack_events_queue;
     std::queue<void *> t2tmsg_queue;

     bool add_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans); 
     void *find_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     void *delete_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     bool add_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans); 
     void *find_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     void *delete_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     void  queue_event(void *context); 
     void  *pop_event(void); 
     void  queue_t2t_msg_event(void *context) { t2tmsg_queue.push(context);} 
     void  *pop_t2t_msg_event(void) 
     {
        if(t2tmsg_queue.empty())
           return NULL;
        void *context = t2tmsg_queue.front();
        t2tmsg_queue.pop();
        return context;
     }
};

#endif
