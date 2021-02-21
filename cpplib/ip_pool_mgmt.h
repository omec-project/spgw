// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef _IP_POOL_MGMT_H
#define _IP_POOL_MGMT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cp_log.h"

class ue_pool_mgmt
{
    public:
        ue_pool_mgmt()
        {
            network_m.s_addr = 0;
            mask_m.s_addr = 0;
        }

        void gen_pool(struct in_addr net, struct in_addr mask)
        {
	        uint32_t total_address = 1;
            uint32_t n = mask.s_addr;
	        while ((n & 0x1) == 0x0) {
	        	total_address = total_address << 1;
	        	n =  n >> 1;
	        }
	       	struct in_addr from_ue_ip;
            from_ue_ip.s_addr = net.s_addr + 1;
            from_ue_ip.s_addr = htonl(from_ue_ip.s_addr);
	       	struct in_addr to_ue_ip;
            to_ue_ip.s_addr = net.s_addr + (total_address - 2);
            to_ue_ip.s_addr = htonl(to_ue_ip.s_addr);

	        LOG_MSG(LOG_INIT,"Number of possible addresses = %d. From %s ", 
                            total_address, inet_ntoa(from_ue_ip));
	        LOG_MSG(LOG_INIT,"Number of possible addresses = %d. To %s  ", 
                            total_address, inet_ntoa(to_ue_ip));

	        for (uint32_t i = total_address - 2; i > 0; i--) {
	        	struct in_addr ue_ip;
	        	ue_ip.s_addr = net.s_addr + i; 
	        	pool_m.push(ue_ip.s_addr);
	        }
            return;
        }

        uint32_t get_ip()
        {
            if(pool_m.empty())
                return 0;
            uint32_t addr = pool_m.front();
            pool_m.pop();
            return addr;    
        }
        
        void free_ip(uint32_t ip)
        {
	       	pool_m.push(ip);
            return;
        }
    private:
        std::queue<uint32_t> pool_m;
        struct in_addr network_m;
        struct in_addr mask_m;
};
#endif
