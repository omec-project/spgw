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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define UPF_ENTRIES_DEFAULT (1 << 16)

struct rte_hash *upf_context_by_ip_hash;

void
create_upf_context_hash(void)
{
	struct rte_hash_parameters rte_hash_params = {
        .name = "upf_context_by_ip_hash",
	    .entries = UPF_ENTRIES_DEFAULT,
	    .key_len = sizeof(uint32_t),
	    .hash_func = rte_jhash,
	    .hash_func_init_val = 0,
	    .socket_id = rte_socket_id(),
	};

	upf_context_by_ip_hash = rte_hash_create(&rte_hash_params);
	if (!upf_context_by_ip_hash) {
		rte_panic("%s hash create failed: %s (%u)\n.",
				rte_hash_params.name,
		    rte_strerror(rte_errno), rte_errno);
	}
}

#ifdef DELETE
struct rte_hash *associated_upf_hash;
void
create_associated_upf_hash(void)
{
	struct rte_hash_parameters rte_hash_params = {
		.name = "associated_upf_hash",
		.entries = 50,
		.key_len = sizeof(uint32_t),
		.hash_func = rte_jhash,
		.hash_func_init_val = 0,
		.socket_id = rte_socket_id(),
	};

	associated_upf_hash = rte_hash_create(&rte_hash_params);
	if (!associated_upf_hash) {
		rte_panic("%s Associated UPF hash create failed: %s (%u)\n.",
				rte_hash_params.name,
				rte_strerror(rte_errno), rte_errno);
	}

}
#endif

// should return int instead of uint... TODO
uint8_t
upf_context_entry_add(uint32_t *upf_ip, upf_context_t *entry)
{
    printf("%s UPF context entry add UPF address %s \n", __FUNCTION__,inet_ntoa(*((struct in_addr *)upf_ip)));
	int ret = 0;
	ret = rte_hash_add_key_data(upf_context_by_ip_hash,
			(const void *)upf_ip , (void *)entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				"%s - Error on rte_hash_add_key_data add\n",
				strerror(ret));
		return 1;
	}
	return 0;
}

int
upf_context_entry_lookup(uint32_t upf_ip, upf_context_t **entry)
{
	int ret = rte_hash_lookup_data(upf_context_by_ip_hash,
			(const void*) &(upf_ip), (void **) entry);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO ENTRY FOUND IN UPF HASH [%u]",
				__func__, __LINE__, upf_ip);
		return -1;
	}
    printf("%s UPF context entry find UPF address %s \n", __FUNCTION__,inet_ntoa(*((struct in_addr *)&upf_ip)));
	return 0;
}

int upf_context_delete_entry(uint32_t upf_ip)
{
    rte_hash_del_key(upf_context_by_ip_hash, (const void *) &upf_ip);
    return 0;
}
