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
#include "apn_struct.h"
#include "cp_config_defs.h"

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

typedef struct mcc_mnc_key
{
	uint8_t mcc_digit_2 :4;
	uint8_t mcc_digit_1 :4;
	uint8_t mnc_digit_3 :4;
	uint8_t mcc_digit_3 :4;
	uint8_t mnc_digit_2 :4;
	uint8_t mnc_digit_1 :4;
} mcc_mnc_key_t;

typedef struct app_config 
{
	uint32_t  flags;
	/* Dataplane selection rules. */
	LIST_HEAD(dplisthead, dp_info) dpList;

	/* add any other dynamic config for spgw control app
	 * Common : various interfaces timeout intervals,  admin state etc.., logging enable/disable
	 * SGW : DDN profile, APN (optional) etc..
	 * PGW : APN, IP Pool etc..
	 */
	struct in_addr dns_p, dns_s; 
}app_config_t;

typedef struct dp_key
{
	struct mcc_mnc_key mcc_mnc;
	uint16_t tac;
}dp_key_t;

#define  CONFIG_DNS_PRIMARY  	0x00000001
#define  CONFIG_DNS_SECONDARY   0x00000002

typedef struct dp_info
{
	uint32_t  flags;
	dp_key_t key; /* TODO - support multiple keys */
	char dpName[DP_SITE_NAME_MAX];
	uint32_t dpId;
	struct in_addr dns_p, dns_s; 
	struct ip_table *static_pool_tree;
	char   *static_pool_net;
	char   *static_pool_mask;
	uint16_t ip_mtu;
	LIST_ENTRY(dp_info) dpentries;
}dp_info_t;

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
	uint16_t upf_pfcp_port;  /* ajay -this should not be part of this config at all  */
	struct in_addr upf_pfcp_ip;

	/* logger parameter */
	uint8_t cp_logger;

	/* RESTORATION PARAMETERS */
	uint8_t transmit_cnt;
	int transmit_timer;
	int periodic_timer;

	/* CP Timer Parameters */
	uint8_t request_tries;
	int request_timeout;    /* Request time out in milisecond */

	app_config_t *appl_config;

	/* IP_POOL_CONFIG Params */
	struct in_addr ip_pool_ip;
	struct in_addr ip_pool_mask;

	/* STATIC_IP_POOL_CONFIG Params */
	struct in_addr static_ip_pool_ip;
	struct in_addr static_ip_pool_mask;

	/* APN */
	uint32_t num_apn;

	dns_cache_params_t dns_cache;
	dns_config_t ops_dns;
	dns_config_t app_dns;
}cp_config_t;

extern cp_config_t *cp_config;

void init_config(void); 
#endif
