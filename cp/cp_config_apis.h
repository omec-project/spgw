// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __CP_CONFIG_H__
#define __CP_CONFIG_H__

#include "gw_adapter.h"
#include "cstats.h"
#include "cp_config.h"
#include "upf_struct.h"
#include "spgw_config_struct.h"

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


sub_profile_t*
get_subscriber_profile(sub_selection_keys_t*);

upf_context_t*
get_upf_context(user_plane_profile_t *upf_profile); 


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


struct in_addr native_linux_name_resolve(const char *name);
#endif
