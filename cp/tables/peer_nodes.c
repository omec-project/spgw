// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "tables/tables.h"
#include <rte_hash.h>
#include <rte_jhash.h>
#include "clogger.h"
#include "rte_lcore.h"
#include "rte_debug.h"
#include "rte_errno.h"

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
    if (!conn_hash_handle) {
        rte_panic("%s::"
                "\n\thash create failed::"
                "\n\trte_strerror= %s; rte_errno= %u\n",
                conn_hash_params.name,
                rte_strerror(rte_errno),
                rte_errno);
    }
    return;
}

int 
add_peer_entry(uint32_t ipaddr, peerData_t *peer)
{
	int ret;

	clLog(clSystemLog, eCLSeverityDebug, "Add entry to connection table  ip:%s",
			inet_ntoa(*(struct in_addr *)&ipaddr));

	ret = rte_hash_add_key_data(conn_hash_handle,
			(const void *)&ipaddr, (void *)peer);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
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
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN PEER HASH HASH [%s]",
				__func__, __LINE__, inet_ntoa(*((struct in_addr *)&ipaddr)));
		return -1;
	}
	return 0;
}

void del_entry_from_hash(uint32_t ipAddr)
{

	int ret = 0;

	clLog(clSystemLog, eCLSeverityDebug, " Delete entry from connection table of ip:%s",
			inet_ntoa(*(struct in_addr *)&ipAddr));

	/* Delete entry from connection hash table */
	ret = rte_hash_del_key(conn_hash_handle,
			&ipAddr);

	if (ret == -ENOENT)
		clLog(clSystemLog, eCLSeverityDebug, "key is not found\n");
	if (ret == -EINVAL)
		clLog(clSystemLog, eCLSeverityDebug, "Invalid Params: Failed to del from hash table\n");
	if (ret < 0)
		clLog(clSystemLog, eCLSeverityDebug, "VS: Failed to del entry from hash table\n");

	//conn_cnt--; ajay

}

