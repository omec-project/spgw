// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "spgw_config.h"
extern "C"
{
    #include <stdio.h>
    #include "spgw_cpp_wrapper.h"
    spgw_config_profile_t *parse_subscriber_profiles_c(const char *file)
    {
        spgw_config_profile_t *new_config;
        printf("parse_config [%s] called in the library \n", file);
        new_config = spgwConfig::parse_subscriber_profiles_cpp(file);
        return new_config;
    }

    void set_cp_config(spgw_config_profile_t *new_config)
    {
        spgwConfig::set_cp_config_cpp(new_config);
    }

    void switch_config(spgw_config_profile_t *new_config)
    {
        spgwConfig::switch_config_cpp(new_config);
    }

    sub_profile_t *match_sub_selection(sub_selection_keys_t *key)
    {
        return spgwConfig::match_sub_selection_cpp(key);
    }
    
    apn_profile_t* match_apn_profile(char *apn, uint16_t len)
    {
        return spgwConfig::match_apn_profile_cpp(apn, len);
    }

}
