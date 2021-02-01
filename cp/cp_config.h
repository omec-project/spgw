// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __CP_CONFIG_NEW__
#define __CP_CONFIG_NEW__

#include <stdint.h>
#include <sys/queue.h>
#include "stdbool.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/limits.h>
#include "cp_config_defs.h"
#include "spgw_config_struct.h"

/**
 * @brief  : Maintains dns cache information
 */
typedef struct dns_cache_params 
{
	uint32_t concurrent;
	uint32_t sec;
	uint8_t percent;

	unsigned long timeoutms;
	uint32_t tries;
} dns_cache_params_t;

/**
 * @brief  : Maintains dns configuration
 */
typedef struct dns_config 
{
	uint8_t freq_sec;
	char filename[PATH_MAX];
	uint8_t nameserver_cnt;
	char nameserver_ip[MAX_NUM_NAMESERVER][INET_ADDRSTRLEN];
} dns_config_t;

/*
- * Define type of Control Plane (CP)
- * SGWC - Serving GW Control Plane
- * PGWC - PDN GW Control Plane
- * SAEGWC - Combined SAEGW Control Plane
- */
enum cp_mode 
{
       SGWC = 01,
       PGWC = 02,
       SAEGWC = 03,
};

typedef struct cp_config 
{
	/* CP Configuration : SGWC=01; PGWC=02; SAEGWC=03 */
	enum cp_mode cp_type;

	/* Control-Plane IPs and Ports Params. */
	struct in_addr s11_ip;
	uint16_t s11_port;

	/* MME Params. */
	uint16_t s11_mme_port;
	struct in_addr s11_mme_ip;

	uint16_t s5s8_port;
	struct in_addr s5s8_ip;

	struct in_addr pfcp_ip;
	uint16_t pfcp_port;

	/* User-Plane IPs and Ports Params. */
	uint16_t upf_pfcp_port;  

	uint16_t prom_port;

    uint16_t webserver_port;

	/* RESTORATION PARAMETERS */
	uint8_t transmit_cnt;
	int transmit_timer;
	int periodic_timer;

	/* CP Timer Parameters */
	uint8_t request_tries;
	int request_timeout;    /* Request time out in milisecond */

    spgw_config_profile_t *subscriber_rulebase;

	/* IP_POOL_CONFIG Params */
	struct in_addr ip_pool_ip;
	struct in_addr ip_pool_mask;

	/* STATIC_IP_POOL_CONFIG Params */
	struct in_addr static_ip_pool_ip;
	struct in_addr static_ip_pool_mask;

    uint32_t dns_enable;

	dns_cache_params_t dns_cache;
	dns_config_t ops_dns;
	dns_config_t app_dns;

    uint32_t  gx_enabled;
    uint32_t  urr_enable;
}cp_config_t;

extern cp_config_t *cp_config;
extern struct ip_table *static_addr_pool;

void init_config(void); 
#endif
