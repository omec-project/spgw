// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __CP_CONFIG_H__
#define __CP_CONFIG_H__

#include "cstats.h"
#include "cp_config.h"
#include "upf_struct.h"

/**
 * @brief  : parse the SGWU/PGWU/SAEGWU IP from config file
 * @param  : cp_config, config file path
 * @return : Returns nothing
 */
void
config_cp_ip_port(cp_config_t *cp_config);

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


extern struct app_config *appl_config;

/*
 * Set flag that Primary DNS config is available at Edge Site level. This should be called 
 * when primary DNS address is set in the PGW app level 
 */
void set_dp_dns_primary(struct dp_info *dp); 
void set_dp_dns_secondary(struct dp_info *dp); 


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

/* Application can pass the dp_key and get back one of the selected DPname in return.
*/
upf_context_t *get_upf_context_for_key(struct dp_key *, dp_info_t **dpInfo);


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
 * Given dpId, fetch configured MTU. If not configured, return default
 */
uint16_t fetch_dp_ip_mtu(uint32_t dpId);

/* Callback function which is received when config file is updated 
 * may be through helm Charts or any other means. 
 */
void config_change_cbk(char *config_file, uint32_t flags);

/**
 * Register for the watcher for the config update
 * @param file
 * filename
 *
 * @return
 * Void
 */
void register_config_updates(char *file);


/**
 * @brief  : assigns the ip pool variable from parsed c-string
 * @param  : ip_str
 *           ip address c-string from command line
 * @return : Returns nothing
 */
void
set_ip_pool_ip(const char *ip_str);


/**
 * @brief  : assigns the ip pool mask variable from parsed c-string
 * @param  : ip_str
 *           ip address c-string from command line
 * @return : Returns nothing
 */
void
set_ip_pool_mask(const char *ip_str);


#endif
