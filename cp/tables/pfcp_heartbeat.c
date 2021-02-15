// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#include <rte_hash.h>
#include <rte_jhash.h>
#include "cp_log.h"
#include "rte_lcore.h"
#include "rte_debug.h"
#include "rte_errno.h"
#include <rte_hash_crc.h>
#include "cp_log.h"


struct rte_hash *heartbeat_recovery_hash;
#define HEARTBEAT_ASSOCIATION_ENTRIES_DEFAULT  (1 << 6)

void
create_heartbeat_hash_table(void)
{
	struct rte_hash_parameters rte_hash_params = {
		.name = "RECOVERY_TIME_HASH",
		.entries = HEARTBEAT_ASSOCIATION_ENTRIES_DEFAULT,
		.key_len = sizeof(uint32_t),
		.hash_func = rte_hash_crc,
		.hash_func_init_val = 0,
		.socket_id = rte_socket_id()
	};

	heartbeat_recovery_hash = rte_hash_create(&rte_hash_params);
    assert(heartbeat_recovery_hash != NULL);
}

int
peer_heartbeat_entry_lookup(uint32_t peer_ip, uint32_t *recov_time)
{
    LOG_MSG(LOG_DEBUG,"peer entry find heartbeat_recovery_hash address %s ", inet_ntoa(*((struct in_addr *)&peer_ip)));
	int ret = rte_hash_lookup_data(heartbeat_recovery_hash,
			(const void*) &(peer_ip), (void **)recov_time);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN PEER heartbeat HASH [%u]", peer_ip);
		return -1;
	}

	return 0;
}

int
add_data_to_heartbeat_hash_table(uint32_t *ip, uint32_t *recov_time)
{
	int ret = 0;
	uint32_t key = UINT32_MAX;
	uint32_t *temp = NULL;
	memcpy(&key,ip,sizeof(uint32_t));

	temp = rte_zmalloc_socket(NULL, sizeof(uint32_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (temp == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate fseid context "
				"structure: %s ", rte_strerror(rte_errno));
		return 1;
	}
	*temp = *recov_time;
	ret = rte_hash_add_key_data(heartbeat_recovery_hash,
			(const void *)&key, temp);
	if (ret < 0) {
		LOG_MSG(LOG_ERROR,"%s - Error on rte_hash_add_key_data add in heartbeat",
				strerror(ret));
		free(temp);
		return 1;
	}
	return 0;
}

void add_ip_to_heartbeat_hash(struct sockaddr_in *peer_addr, uint32_t recovery_time)
{
	uint32_t *default_recov_time = NULL;
	default_recov_time = rte_zmalloc_socket(NULL, sizeof(uint32_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());

	if(default_recov_time == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate memory in adding ip to heartbeat"
				"structure: %s ", rte_strerror(rte_errno));
	} else {

		*default_recov_time = recovery_time;
		int ret = add_data_to_heartbeat_hash_table( &peer_addr->sin_addr.s_addr ,
				default_recov_time);

		if(ret !=0) {
			LOG_MSG(LOG_ERROR,"%s - Error on rte_hash_add_key_data add in heartbeat",
					strerror(ret));
		}
	}
}


void delete_entry_heartbeat_hash(struct sockaddr_in *peer_addr)
{
	int ret = rte_hash_del_key(heartbeat_recovery_hash,
			(const void *)&(peer_addr->sin_addr.s_addr));
	if (ret == -EINVAL || ret == -ENOENT) {
		LOG_MSG(LOG_ERROR,"%s - Error on rte_delete_enrty_key_data add in heartbeat",
				strerror(ret));
	}
    
}

void clear_heartbeat_hash_table(void)
{
	rte_hash_free(heartbeat_recovery_hash);
}

