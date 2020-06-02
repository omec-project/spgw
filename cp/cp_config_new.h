/*
* Copyright 2020-present Open Networking Foundation
*
* SPDX-License-Identifier: Apache-2.0
*
*/

#ifndef __CP_CONFIG_NEW__
#define __CP_CONFIG_NEW__

#include <stdint.h>
#include <sys/queue.h>
#include "stdbool.h"

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

	/* MME Params. */
	uint16_t s11_mme_port;
	struct in_addr s11_mme_ip;

	uint16_t s5s8_port;
	struct in_addr s5s8_ip;

	/* Control-Plane IPs and Ports Params. */
	struct in_addr s11_ip;
	uint16_t s11_port;

	struct in_addr pfcp_ip;
	uint16_t pfcp_port;

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


	dns_cache_params_t dns_cache;
	dns_config_t ops_dns;
	dns_config_t app_dns;
} pfcp_config_t;

/**
 * Used to hold registered UPF context.
 * Also becomes a part of the TAILQ list node
 */
typedef struct cfg_upf_context {
	char zmq_pull_ifconnect[128];
	char zmq_push_ifconnect[128];
	void *zmqpull_sockctxt;
	void *zmqpull_sockcet;
	void *zmqpush_sockctxt;
	void *zmqpush_sockcet;
	uint16_t cp_comm_port;
	uint32_t dpId;

	TAILQ_ENTRY(cfg_upf_context) entries;
} cfg_upf_context;

#define DP_SITE_NAME_MAX		256

#define CP_CONFIG_OPT_PATH		"../config/app_config.cfg"
#define CP_CONFIG_FOLDER		"../config/"


struct mcc_mnc_key
{
	uint8_t mcc_digit_2 :4;
	uint8_t mcc_digit_1 :4;
	uint8_t mnc_digit_3 :4;
	uint8_t mcc_digit_3 :4;
	uint8_t mnc_digit_2 :4;
	uint8_t mnc_digit_1 :4;
};

struct app_config 
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
};


struct dp_key
{
	struct mcc_mnc_key mcc_mnc;
	uint16_t tac;
};

#define  CONFIG_DNS_PRIMARY  	0x00000001
#define  CONFIG_DNS_SECONDARY   0x00000002
#define DEFAULT_IPV4_MTU        (1450)

struct dp_info
{
	uint32_t  flags;
	struct dp_key key;
	char dpName[DP_SITE_NAME_MAX];
	uint32_t dpId;
	struct in_addr s1u_sgw_ip;
	struct cfg_upf_context *upf;
	struct in_addr dns_p, dns_s; 
	struct ip_table *static_pool_tree;
	char   *static_pool;
	uint16_t ip_mtu;
	LIST_ENTRY(dp_info) dpentries;
};

/*
- * Define type of Control Plane (CP)
- * SGWC - Serving GW Control Plane
- * PGWC - PDN GW Control Plane
- * SAEGWC - Combined SAEGW Control Plane
- */
enum cp_mode {
       SGWC = 01,
       PGWC = 02,
       SAEGWC = 03,
};

typedef struct cp_config {
	/* CP Configuration : SGWC=01; PGWC=02; SAEGWC=03 */
	enum cp_mode cp_type;

	/* logger parameter */
	uint8_t cp_logger;

	struct app_config *appl_config;

	/* IP_POOL_CONFIG Params */
	struct in_addr ip_pool_ip;
	struct in_addr ip_pool_mask;

	/* APN */
	uint32_t num_apn;
	/* apn apn_list[MAX_NUM_APN]; */
}cp_config_t;

extern cp_config_t *cp_config;
extern pfcp_config_t pfcp_config;
#endif
