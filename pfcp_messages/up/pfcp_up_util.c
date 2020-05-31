/*
 * Copyright (c) 2019 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/time.h>
#include <rte_hash.h>
#include <rte_errno.h>
#include <rte_debug.h>
#include <rte_jhash.h>
#include <rte_lcore.h>
#include <rte_hash_crc.h>

#include "pfcp_up_util.h"
#include "pfcp_up_set_ie.h"
#include "pfcp_messages.h"
#include "clogger.h"

#define LDB_ENTRIES_DEFAULT (1024 * 1024 * 4)

#define QUERY_RESULT_COUNT 16

extern int pfcp_fd;

struct rte_hash *node_id_hash;
struct rte_hash *heartbeat_recovery_hash;
struct rte_hash *associated_upf_hash;

int
pfcp_send(int fd, void *msg_payload, uint32_t size,
		struct sockaddr_in *peer_addr)
{
	socklen_t addr_len = sizeof(*peer_addr);
	uint32_t bytes = sendto(fd,
			(uint8_t *) msg_payload,
			size,
			MSG_DONTWAIT,
			(struct sockaddr *)peer_addr,
			addr_len);
	return bytes;
}

long
uptime(void)
{
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0) {
	}
	return s_info.uptime;
}

void
create_node_id_hash(void)
{

	struct rte_hash_parameters rte_hash_params = {
		.name = "node_id_hash",
		.entries = LDB_ENTRIES_DEFAULT,
		.key_len = sizeof(uint32_t),
		.hash_func = rte_hash_crc,
		.hash_func_init_val = 0,
		.socket_id = rte_socket_id()
	};

	node_id_hash = rte_hash_create(&rte_hash_params);
	if (!node_id_hash) {
		rte_panic("%s hash create failed: %s (%u)\n.",
				rte_hash_params.name,
				rte_strerror(rte_errno), rte_errno);
	}

}

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
	if (!heartbeat_recovery_hash) {
		rte_panic("%s hash create failed: %s (%u)\n.",
				rte_hash_params.name,
				rte_strerror(rte_errno), rte_errno);
	}
}

void
create_associated_upf_hash(void)
{
	struct rte_hash_parameters rte_hash_params = {
		.name = "associated_upf_hash",
		.entries = 50,
		.key_len = UINT32_SIZE,
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

uint32_t
current_ntp_timestamp(void) 
{

	struct timeval tim;
	uint8_t ntp_time[8] = {0};
	uint32_t timestamp = 0;

	gettimeofday(&tim, NULL);
	time_to_ntp(&tim, ntp_time);

	timestamp |= ntp_time[0] << 24 | ntp_time[1] << 16
								| ntp_time[2] << 8 | ntp_time[3];

	return timestamp;
}

void
time_to_ntp(struct timeval *tv, uint8_t *ntp)
{
	uint64_t ntp_tim = 0;
	uint8_t len = (uint8_t)sizeof(ntp)/sizeof(ntp[0]);
	uint8_t *p = ntp + len;

	int i = 0;

	ntp_tim = tv->tv_usec;
	ntp_tim <<= 32;
	ntp_tim /= 1000000;

	/* Setting the ntp in network byte order */

	for (i = 0; i < len/2; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}

	ntp_tim = tv->tv_sec;
	ntp_tim += OFFSET;

	/* Settting  the fraction of second */

	for (; i < len; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}
}
