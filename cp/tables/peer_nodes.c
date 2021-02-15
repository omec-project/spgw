// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#include <rte_hash.h>
#include <rte_jhash.h>
#include "rte_lcore.h"
#include "rte_debug.h"
#include "rte_errno.h"
#include "cp_log.h"

struct rte_hash *conn_hash_handle;

#define NUM_CONN	500

void echo_table_init(void)
{
    struct rte_hash_parameters
        conn_hash_params = {
            .name = "CONN_TABLE",
            .entries = NUM_CONN,
            .reserved = 0,
            .key_len = sizeof(uint32_t),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0
        };

    /* Create conn_hash for maintain each port peer connection details */
    /* Create arp_hash for each port */
    conn_hash_params.socket_id = rte_socket_id();
    conn_hash_handle = rte_hash_create(&conn_hash_params);
    assert(conn_hash_handle != NULL);
    return;
}

int 
add_peer_entry(uint32_t ipaddr, peerData_t *peer)
{
	int ret;

	LOG_MSG(LOG_DEBUG, "Add entry to connection table  ip:%s",
			inet_ntoa(*(struct in_addr *)&ipaddr));

	ret = rte_hash_add_key_data(conn_hash_handle,
			(const void *)&ipaddr, (void *)peer);

	if (ret < 0) {
		LOG_MSG(LOG_ERROR,
				"%s - Error on rte_hash_add_key_data add",
				strerror(ret));
		return -1;
	}
	return 0;

}

/* LOOKUP - TEID to session */
int
get_peer_entry(uint32_t ipaddr, peerData_t **entry)
{
	int ret = rte_hash_lookup_data(conn_hash_handle,
			(const void*) &(ipaddr), (void **) entry);

	if (ret < 0) {
		LOG_MSG(LOG_DEBUG, "NO ENTRY FOUND IN PEER HASH [%s]",
				inet_ntoa(*((struct in_addr *)&ipaddr)));
		return -1;
	}
	return 0;
}

void del_entry_from_hash(uint32_t ipAddr)
{

	int ret = 0;

	LOG_MSG(LOG_DEBUG, " Delete entry from connection table of ip:%s",
			inet_ntoa(*(struct in_addr *)&ipAddr));

	/* Delete entry from connection hash table */
	ret = rte_hash_del_key(conn_hash_handle,
			&ipAddr);

	if (ret == -ENOENT)
		LOG_MSG(LOG_DEBUG, "key is not found");
	if (ret == -EINVAL)
		LOG_MSG(LOG_DEBUG, "Invalid Params: Failed to del from hash table");
	if (ret < 0)
		LOG_MSG(LOG_DEBUG, "VS: Failed to del entry from hash table");

	//conn_cnt--; ajay

}

