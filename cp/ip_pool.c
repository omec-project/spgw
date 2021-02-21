
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <sys/types.h>
#include <sys/socket.h>
#include "stdint.h"
#include "ip_pool.h"
#include "gtpv2_ie.h"
#include "ue.h"
#include "cp_config.h"
#include "util.h"
#include "cp_log.h"
#include "assert.h"
#include "spgw_config_struct.h"
#include "spgw_cpp_wrapper.h"

struct ip_table *static_addr_pool = NULL;
struct ue_pool_dynamic_t *dyn_pool = NULL;


uint32_t
acquire_ip(struct in_addr *ipv4)
{
//#define TESTPOOL
#ifdef TESTPOOL
    // Test loop to allocate address and free it ..
    int i=0;
  start:
    i++;
#endif
    uint32_t addr = acquire_ip_cpp(dyn_pool);
    if(addr == 0) {
		LOG_MSG(LOG_ERROR, "IP Pool depleted");
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
    }
    ipv4->s_addr = htonl(addr); // give back in network order 
    LOG_MSG(LOG_ERROR, "IP address assigned to call %s ", inet_ntoa(*ipv4));
#ifdef TESTPOOL
    if(i < 10) {
        release_ip(*ipv4);
        goto start;
    }
#endif
    return 0;
}

void
release_ip(struct in_addr ipv4)
{
    ipv4.s_addr = htonl(ipv4.s_addr); // print
    LOG_MSG(LOG_ERROR, "IP address released %s ", inet_ntoa(ipv4));
    ipv4.s_addr = htonl(ipv4.s_addr); // release api needs input in host order 
    release_ip_cpp(dyn_pool, ipv4);
}

void create_ue_pool_dynamic(struct in_addr network, struct in_addr mask)
{
    dyn_pool = create_ue_pool_dynamic_cpp(network, mask);
}

//network address in host order 
struct ip_table *
create_ue_pool(struct in_addr network, struct in_addr mask)
{
	// create static pool
	uint32_t total_address = 1;
    uint32_t n = mask.s_addr;
	while ((n & 0x1) == 0x0) {
		total_address = total_address << 1;
		n =  n >> 1;
	}
	struct in_addr from_ue_ip;
    from_ue_ip.s_addr = network.s_addr + 1;
    from_ue_ip.s_addr = htonl(from_ue_ip.s_addr);
	struct in_addr to_ue_ip;
    to_ue_ip.s_addr = network.s_addr + (total_address - 2);
    to_ue_ip.s_addr = htonl(to_ue_ip.s_addr);

	LOG_MSG(LOG_INIT,"Number of possible addresses = %d. From %s ", 
                    total_address, inet_ntoa(from_ue_ip));
	LOG_MSG(LOG_INIT,"Number of possible addresses = %d. To %s  ", 
                    total_address, inet_ntoa(to_ue_ip));


	struct ip_table *addr_pool = calloc(1, sizeof(struct ip_table));
	if (addr_pool == NULL) {
		LOG_MSG(LOG_ERROR, "Address pool allocation failed. ");
		return NULL;
	}
	for (uint32_t i = 1; i < (total_address - 1); i++) {
		struct in_addr ue_ip;
		ue_ip.s_addr = network.s_addr + i; 
		add_ipaddr_in_pool(addr_pool, ue_ip);
	}
	return addr_pool;
}

/* Add nodes in the m-trie. Duplicate elements overwrite old elements
 * 4 comparisions to add the ip address
 */
void 
add_ipaddr_in_pool(struct ip_table *search_tree, struct in_addr host)
{
	unsigned char byte;
	uint32_t addr = host.s_addr;
	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
	uint32_t shift[]  = {24, 16, 8, 0};

	for (int i = 0; i <= 3; i++) {
		byte = (mask[i] & addr) >> shift[i];
		if (search_tree->octet[byte] == NULL) {
			search_tree->octet[byte] = calloc(1, sizeof(struct ip_table));
			assert(search_tree->octet[byte] != NULL);
		}
		search_tree = search_tree->octet[byte];
	}
    host.s_addr = htonl(host.s_addr); // printing purpose
	char *p = inet_ntoa(host);
	search_tree->ue_address = calloc(1,20); /*abc.efg.hij.klm => 15 char */ 
	strcpy(search_tree->ue_address, p);
	return;
}

/* Check if given host is part of the tree */
bool
reserve_ip_node(struct ip_table *search_tree , struct in_addr host)
{
	unsigned char byte;
	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
	uint32_t shift[]  = {24, 16, 8, 0};
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
bool 
release_ip_node(struct ip_table *search_tree , struct in_addr host)
{
	unsigned char byte;
	uint32_t mask[] = {0xff000000, 0xff0000, 0xff00, 0xff};
	uint32_t shift[]  = {24, 16, 8, 0};

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
