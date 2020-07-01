
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <sys/types.h>
#include <sys/socket.h>
#include "stdint.h"
#include "ip_pool.h"
#include "gtpv2c_ie.h"
#include "ue.h"
#include "cp_config.h"
#include "clogger.h"

struct ip_table *static_addr_pool = NULL;


/* TODO - Priority 1 - only  1 ip pool is supported. Need better pool management  */
#define GET_UE_IP(ue_index) \
			(((cp_config->ip_pool_ip.s_addr | (~cp_config->ip_pool_mask.s_addr)) \
			  - htonl(ue_index)) - 0x01000000)


#define LDB_ENTRIES_DEFAULT (1024 * 1024 * 4)
/* TODO : Prio2 . Scaling needs change in this area. */
uint32_t
acquire_ip(struct in_addr *ipv4)
{
	static uint32_t next_ip_index;
	if (unlikely(next_ip_index == LDB_ENTRIES_DEFAULT)) {
		clLog(clSystemLog, eCLSeverityCritical, "IP Pool depleted\n");
		return GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED;
	}
	ipv4->s_addr = GET_UE_IP(next_ip_index++);
	return 0;
}

//network address in host order 
struct ip_table *
create_ue_pool(struct in_addr network, struct in_addr mask)
{
	// create static pool
	uint32_t total_address = 1;
    uint32_t n = mask.s_addr;
	n =  n >> 1;
	while ((n & 0x1) == 0x0) {
		total_address = total_address << 1;
		n =  n >> 1;
	}
	printf("\n Number of possible addresses = %d \n", total_address);
	struct ip_table *addr_pool = calloc(1, sizeof(struct ip_table));
	if (addr_pool == NULL) {
		printf("\n Address pool allocation failed. \n");
		return NULL;
	}
	for (uint32_t i = 1; i < (total_address - 1); i++) {
		struct in_addr ue_ip;
		ue_ip.s_addr = network.s_addr + i; 
		add_ipaddr_in_pool(addr_pool, ue_ip);
		ue_ip.s_addr = htonl(ue_ip.s_addr); // just for print purpose
		printf("Add UE IP address = %s  in pool \n", inet_ntoa(ue_ip)); 
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
			if (search_tree->octet[byte] == NULL)
				rte_panic("Unable to allocate memory for octet!\n");
		}
		search_tree = search_tree->octet[byte];
	}
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
		printf("Failed to reserve IP address %s. Static Pool not configured \n", inet_ntoa(host));
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
		printf("Found address %s in static IP Pool. But this is already used. Rejecy call setup  \n", search_tree->ue_address);
		return false;
	}
	printf("Found address %s in static IP Pool \n", search_tree->ue_address);
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
		printf("Failed to release IP address %s. Static Pool not configured \n", inet_ntoa(printHost));
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
		printf("Found address %s in static IP Pool. Freeing the addres \n", search_tree->ue_address);
		search_tree->used = false; 
		return true;
	}

	printf("address %s was not part of static pool \n", search_tree->ue_address);
	return false;
}
