// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __CP_CONFIG_H__
#define __CP_CONFIG_H__

#include "spgw_config_struct.h"
#ifdef __cplusplus
extern "C" {
#endif

void init_config(void); 

/**
 * @brief  : parse the SGWU/PGWU/SAEGWU IP from config file
 * @param  : cp_config, config file path
 * @return : Returns nothing
 */
void
config_cp_ip_port(cp_config_t *cp_config);

/* Callback function which is received when config file is updated 
 * may be through helm Charts or any other means. 
 */
void config_change_cbk(char *config_file, uint32_t flags);
void cpconfig_change_cbk(char *config_file, uint32_t flags);

extern char* config_update_base_folder; 
extern bool native_config_folder;
#ifdef __cplusplus
}
#endif
#endif
