/*
* Copyright 2020-present Open Networking Foundation
* Copyright (c) 2019 Sprint
*
* SPDX-License-Identifier: Apache-2.0
*
*/

#ifndef __CP_CONFIG_H__
#define __CP_CONFIG_H__

#include "cp.h"

#include "gw_adapter.h"
#include "clogger.h"
#include "cstats.h"
/* for TAILQ */
#include <sys/queue.h>
#include "cp_config_new.h"

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

/**
 * @brief  : parse the SGWU/PGWU/SAEGWU IP from config file
 * @param  : pfcp_config, config file path
 * @return : Returns nothing
 */
void
config_cp_ip_port(pfcp_config_t *pfcp_config);

/**
 * @brief  : parse apn arguments
 * @param  : temp, input data
 * @param  : ptr[], array to store parsed arguments
 * @return : Returns nothing
 */
void parse_apn_args(char *temp,char *ptr[3]);
/*
 *
 **/
int
check_cp_req_timeout_config(char *value);
/*
 *
 **/
int
check_cp_req_tries_config(char *value);


#define DP_SITE_NAME_MAX		256
#include <stdint.h>
#include <sys/queue.h>
#include "stdbool.h"

#define CP_CONFIG_OPT_PATH		"../config/app_config.cfg"
#define CP_CONFIG_FOLDER		"../config/"

extern struct app_config *appl_config;
struct cfg_upf_context;

struct mcc_mnc_key
{
	uint8_t mcc_digit_1 :4;
	uint8_t mcc_digit_2 :4;
	uint8_t mcc_digit_3 :4;
	uint8_t mnc_digit_3 :4;
	uint8_t mnc_digit_1 :4;
	uint8_t mnc_digit_2 :4;
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
 * Set flag that Primary DNS config is available at Edge Site level. This should be called 
 * when primary DNS address is set in the PGW app level 
 */
void set_dp_dns_primary(struct dp_info *dp); 
void set_dp_dns_secondary(struct dp_info *dp); 

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

/*
 * Set flag that primary DNS config is available at App level. This should be called 
 * when primary DNS address is set in the PGW app level 
 */

void set_app_dns_primary(struct app_config *app); 

/*
 *  Get primary DNS address if available in dns_p and return true.
 *  In case address is not available then return false
 */

bool get_app_primary_dns(struct app_config *app, struct in_addr *dns_p);

/*
 * Set flag that secondary DNS config is available at App level. This should be called 
 * when secondary DNS address is set in the PGW app level 
 */

void set_app_dns_secondary(struct app_config *app); 

/*
 *  Get secondary DNS address if available in dns_p and return true.
 *  In case address is not available then return false
 */

bool get_app_secondary_dns(struct app_config *app, struct in_addr *dns_s);

void init_spgwc_dynamic_config(struct app_config *cfg);

/* Application can pass the dp_key and get back one of the selected DP in return.
 * Over the period of time dp_key will have multiple keys and this API will
 * go through all the config to return one of the first matching DP ID.
*/
uint32_t select_dp_for_key(struct dp_key *);

/**
 * Whenever a upf registers with a CP, the entry is scanned through
 * a list of appl_config list of dp's. If the DP exists in the appl_config
 * list, it returns 1, otherwise it returns 0
 */
uint8_t resolve_upf_context_to_dpInfo(struct cfg_upf_context *upf, char *hostname, struct in_addr s1u_sgw_ip);

/**
 * Given dpId, what is the s1u's IP address of dp (as stored in the apl_config list)
 */
struct in_addr fetch_s1u_sgw_ip(uint32_t dpId);

/**
 * Given dpId, what is the DNS Primary IP address of dp. If DP does not have config then 
 * pass DNS config from global scope 
 */
struct in_addr fetch_dns_primary_ip(uint32_t dpId, bool *present);


/**
 * Given dpId, what is the DNS Secondary IP address of dp. If DP does not have config then 
 * pass DNS config from global scope 
 */
struct in_addr fetch_dns_secondary_ip(uint32_t dpId, bool *present);

/**
 * Given dpId, what is upf's context (as stored in the apl_config list)
 */
struct cfg_upf_context *fetch_upf_context(uint32_t dpId);

/**
 * Given dpId, return dpInfo 
 */
struct dp_info *fetch_dp_context(uint32_t dpId);

/* Parse and create static ip pool */
char*
parse_create_static_ip_pool(struct ip_table **addr_pool, const char *entry);

/**
 * Given dpId, fetch configured MTU. If not configured, return default
 */
uint16_t fetch_dp_ip_mtu(uint32_t dpId);


#endif
