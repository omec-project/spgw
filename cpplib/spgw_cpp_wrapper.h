// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW__CPP_WRAPPER__H
#define __SPGW__CPP_WRAPPER__H
#include "spgw_config_struct.h"
spgw_config_profile_t* parse_subscriber_profiles_c(const char *);
void switch_config(spgw_config_profile_t *);

/* API to be called by application to get the profiles */
sub_profile_t* match_sub_selection(sub_selection_keys_t *key); 
#endif
