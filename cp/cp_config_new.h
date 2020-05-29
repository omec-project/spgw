/*
* Copyright 2020-present Open Networking Foundation
*
* SPDX-License-Identifier: Apache-2.0
*
*/

#ifndef __CP_CONFIG_NEW__
#define __CP_CONFIG_NEW__
/*PFCP Config file*/
#define STATIC_CP_FILE "../config/cp.cfg"

#define MAX_DP_SIZE   5
#define MAX_CP_SIZE   1
#define MAX_NUM_MME   5
#define MAX_NUM_SGWC  5
#define MAX_NUM_PGWC  5
#define MAX_NUM_SGWU  5
#define MAX_NUM_PGWU  5
#define MAX_NUM_SAEGWU 5

#define MAX_NUM_APN   16

#define MAX_NUM_NAMESERVER 8

#define SGWU_PFCP_PORT   8805
#define PGWU_PFCP_PORT   8805
#define SAEGWU_PFCP_PORT   8805


/**
 * @brief  : Maintains dns cache information
 */
typedef struct dns_cache_params_t {
	uint32_t concurrent;
	uint32_t sec;
	uint8_t percent;

	unsigned long timeoutms;
	uint32_t tries;
} dns_cache_params_t;

/**
 * @brief  : Maintains dns configuration
 */
typedef struct dns_config_t {
	uint8_t freq_sec;
	char filename[PATH_MAX];
	uint8_t nameserver_cnt;
	char nameserver_ip[MAX_NUM_NAMESERVER][INET_ADDRSTRLEN];
} dns_config_t;

/**
 * @brief  : Maintains pfcp configuration
 */
typedef struct pfcp_config_t {
	/* CP Configuration : SGWC=01; PGWC=02; SAEGWC=03 */
	uint8_t cp_type;

	/* MME Params. */
	uint16_t s11_mme_port;
	struct in_addr s11_mme_ip;

	/* Control-Plane IPs and Ports Params. */
	uint16_t s11_port;
	uint16_t s5s8_port;
	uint16_t pfcp_port;
	struct in_addr s11_ip;
	struct in_addr s5s8_ip;
	struct in_addr pfcp_ip;

	/* User-Plane IPs and Ports Params. */
	uint16_t upf_pfcp_port;  /* ajay -this should not be part of this config at all  */
	struct in_addr upf_pfcp_ip;

	/* RESTORATION PARAMETERS */
	uint8_t transmit_cnt;
	int transmit_timer;
	int periodic_timer;

	/* CP Timer Parameters */
	uint8_t request_tries;
	int request_timeout;    /* Request time out in milisecond */

	/* logger parameter */
	uint8_t cp_logger;

	/* APN */
	uint32_t num_apn;
	/* apn apn_list[MAX_NUM_APN]; */

	dns_cache_params_t dns_cache;
	dns_config_t ops_dns;
	dns_config_t app_dns;

	/* IP_POOL_CONFIG Params */
	struct in_addr ip_pool_ip;
	struct in_addr ip_pool_mask;


} pfcp_config_t;


#endif
