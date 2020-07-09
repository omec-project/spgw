// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW__CPP_WRAPPER__H
#define __SPGW__CPP_WRAPPER__H
#include "spgw_config_struct.h"

spgw_config_profile_t* parse_subscriber_profiles_c(const char *);

/* API to set the config handler in library */
void set_cp_config(spgw_config_profile_t *);

/* When config is modified, this api is used to switch from old to new config
 * and also old config is deleted 
 */
void switch_config(spgw_config_profile_t *);

/* API to be called by application to get the profiles */
sub_profile_t* match_sub_selection(sub_selection_keys_t *key); 

/* API to find matching profile */
apn_profile_t * match_apn_profile(char *, uint16_t len);
#endif
