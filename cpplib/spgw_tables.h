// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW_TABLES_H__
#define __SPGW_TABLES_H__
#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>

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
        if(lhs.src_port == rhs.src_port)
        {
            if(lhs.src_ip == rhs.src_ip)
            {
                if(lhs.seq_num == rhs.seq_num)
                {
                    return false;
                }
                return lhs.seq_num < rhs.seq_num;
            }
            return lhs.src_ip < rhs.src_ip;
        }
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
     std::map<uint32_t, void *> spgw_gx_transaction_map;
     std::queue<void *> stack_events_queue;
     std::queue<void *> stack_test_events_queue;
     std::queue<void *> gtp_out_queue;
     std::queue<void *> t2tmsg_queue;
     std::queue<void *> pfcp_out_queue;
     std::queue<void *> gx_out_queue;

     bool add_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans); 
     void *find_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     void *delete_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 

     bool add_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans); 
     void *find_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 
     void *delete_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq); 

     bool add_gx_trans(uint32_t msg_seq, void *trans);
     void *find_gx_trans(uint32_t msg_seq);
     void *delete_gx_trans(uint32_t msg_seq);


     void  queue_event(void *context); 
     void  *pop_event(void); 

	 /* Test events */
     void  queue_test_event(void *context);
     void* pop_test_event(void);

	 /* GTP events */
     void  queue_gtp_out_event(void *context);
     void* pop_gtp_out_event(void);

	 /* PFCP events */
     void  queue_pfcp_out_event(void *context);
     void* pop_pfcp_out_event(void);

	 /* GX events */
     void  queue_gx_out_event(void *context);
     void* pop_gx_out_event(void);



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
