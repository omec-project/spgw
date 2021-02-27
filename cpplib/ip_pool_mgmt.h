// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#ifndef _IP_POOL_MGMT_H
#define _IP_POOL_MGMT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cp_log.h"
#include <string>
#include <queue>
#include <map>

class ue_dynamic_pool;

class ue_pool
{
    public:
        ue_pool(std::string pool_name)
        {
            pool_name_m = pool_name; 
        }
        virtual ~ue_pool() {}
        virtual uint32_t get_ip() { return 0;}
        virtual void free_ip(uint32_t ip) {}
        virtual void gen_pool(struct in_addr net, struct in_addr mask) {}
        virtual bool reserve_ip_node(struct in_addr host){return false;}
        virtual bool release_ip_node(struct in_addr host){return false;}
    private:
        std::string pool_name_m;
};

class ue_dynamic_pool: public ue_pool 
{
    public:
        ue_dynamic_pool(std::string pool_name):
                       ue_pool(pool_name)
        {
            network_m.s_addr = 0;
            mask_m.s_addr = 0;
            std::cout<<"ue_dynamic_pool name - "<<pool_name<<std::endl;
        }
        
        virtual ~ue_dynamic_pool() {}

        void gen_pool(struct in_addr net, struct in_addr mask) override
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

        virtual uint32_t get_ip() override
        {
            if(pool_m.empty())
                return 0;
            uint32_t addr = pool_m.front();
            pool_m.pop();
            return addr;    
        }
        
        virtual void free_ip(uint32_t ip) override
        {
	       	pool_m.push(ip);
            return;
        }
    private:
        std::queue<uint32_t> pool_m;
        struct in_addr network_m;
        struct in_addr mask_m;
};

struct ip_table
{
  struct ip_table *octet[256];
  char *ue_address; // address 
  bool used ; 
};

class ue_static_pool: public ue_pool
{
    public:
        ue_static_pool(std::string pool_name):
                       ue_pool(pool_name)
        {
            network_m.s_addr = 0;
            mask_m.s_addr = 0;
            addr_pool_m = NULL;
            std::cout<<"ue static pool name - "<<pool_name<<std::endl;
        }

        virtual ~ue_static_pool() {}

        void gen_pool(struct in_addr net, struct in_addr mask) override
        {
	        // create static pool
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


	        addr_pool_m = (struct ip_table *)calloc(1, sizeof(struct ip_table));
	        if (addr_pool_m == NULL) {
	        	LOG_MSG(LOG_ERROR, "Address pool allocation failed. ");
	        	return;
	        }

	        for (uint32_t i = 1; i < (total_address - 1); i++) {
	        	struct in_addr ue_ip;
	        	ue_ip.s_addr = net.s_addr + i; 
	        	add_ipaddr_in_pool(ue_ip);
	        }

            return;
        }

        void add_ipaddr_in_pool(struct in_addr host)
        {
        	unsigned char byte;
        	uint32_t addr = host.s_addr;
        	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
        	uint32_t shift[]  = {24, 16, 8, 0};
        
            struct ip_table *search_tree = addr_pool_m; 
        	for (int i = 0; i <= 3; i++) {
        		byte = (mask[i] & addr) >> shift[i];
        		if (search_tree->octet[byte] == NULL) {
        			search_tree->octet[byte] = (struct ip_table *)calloc(1, sizeof(struct ip_table));
        			assert(search_tree->octet[byte] != NULL);
        		}
        		search_tree = search_tree->octet[byte];
        	}
            host.s_addr = htonl(host.s_addr); // printing purpose
        	char *p = inet_ntoa(host);
        	search_tree->ue_address = (char *)calloc(1,20); /*abc.efg.hij.klm => 15 char */ 
        	strcpy(search_tree->ue_address, p);
        	return;
        }

        /* Check if given host is part of the tree */
        bool reserve_ip_node(struct in_addr host)
        {
        	unsigned char byte;
        	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
        	uint32_t shift[]  = {24, 16, 8, 0};
            struct ip_table *search_tree = addr_pool_m; 
        	if (search_tree == NULL) {
        		host.s_addr = htonl(host.s_addr);
        		LOG_MSG(LOG_ERROR,"Failed to reserve IP address %s. Static Pool not configured ", inet_ntoa(host));
        		return false;
        	}
        
        	for (int i = 0; i <= 3; i++) {
        		byte = ((host.s_addr) & mask[i]) >> shift[i];
        		if (search_tree->octet[byte] == NULL) {
        			return false;
        		}
        		search_tree = search_tree->octet[byte]; 
        	}
        
        	if (search_tree->used == true) {
        		LOG_MSG(LOG_ERROR, "Found address %s in static IP Pool. But this is already used. Reject call setup ", search_tree->ue_address);
        		return false;
        	}
        	LOG_MSG(LOG_DEBUG,"Found address %s in static IP Pool ", search_tree->ue_address);
        	/* Delete should free the flag.. Currently we are not taking care of hanging sessions. 
        	 * hangign sessions at PDN GW + Static Address is already trouble. This also means that
        	 * if new session comes to PDN GW and if old session found present then PDN GW should 
        	 * delete old session. this is TODO.
        	 */ 
        	search_tree->used = true;
        	return true;
        }

        /* Mark the host as free */
        bool release_ip_node(struct in_addr host) override
        {
        	unsigned char byte;
        	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
        	uint32_t shift[]  = {24, 16, 8, 0};
        
            struct ip_table *search_tree = addr_pool_m; 
        	if (search_tree == NULL) {
                struct in_addr printHost = {0};
        		printHost.s_addr = htonl(host.s_addr); // Changing just for print purpoise 
        		LOG_MSG(LOG_ERROR,"Failed to release IP address %s. Static Pool not configured ", inet_ntoa(printHost));
        		return false;
        	}
        
        	for (int i = 0; i <= 3; i++) {
        		byte = ((host.s_addr) & mask[i])>> shift[i];
        		if (search_tree->octet[byte] == NULL) {
        			return false;
        		}
        		search_tree = search_tree->octet[byte];
        	}
        
        	if (search_tree->used == true) {
        		LOG_MSG(LOG_DEBUG, "Found address %s in static IP Pool. Freeing the address ", search_tree->ue_address);
        		search_tree->used = false; 
        		return true;
        	}
        
        	LOG_MSG(LOG_ERROR, "Address %s was not part of static pool ", search_tree->ue_address);
        	return false;
        }
    private:
        struct ip_table *addr_pool_m;
        struct in_addr network_m;
        struct in_addr mask_m;
};

class ip_pools 
{
    public:
        static ip_pools *getInstance() 
        {
            static ip_pools pool_manager;
            return &pool_manager;
        }

        ue_pool* getIpPool(std::string pool_name)
        {
            return pool_objects[pool_name];
        }

        void addIpPool(ue_pool *p, std::string pool_name)
        {
            pool_objects[pool_name] = p;    
        }

    private:
        ip_pools() { }
        ~ip_pools() { }
    private:
        std::map<std::string, ue_pool*>  pool_objects;
};
#endif
