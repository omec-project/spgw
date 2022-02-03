// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "spgw_cpp_wrapper.h"
#include "spgw_config_struct.h"
#include "string.h"
int main()
{
    int count = 2;
    while(count)
    {
        count--;
        spgw_config_profile_t *new_config;
        new_config = parse_subscriber_profiles_c("../config/subscriber_mapping.json");
        switch_config(new_config);

        {
            printf("****TEST 1 ****\n");
            sub_selection_keys_t key = {0};
            key.imsi.is_valid = true;
            key.imsi.from_imsi = 208014567891201;
            sub_config_t *sub_profile;
            sub_profile = match_sub_selection(&key);
            printf("sub_profile = %p \n",sub_profile);
            if(sub_profile != NULL)
            {
                printf("Up profile = %s \n",sub_profile->up_profile->user_plane_profile_name);
                printf("APN profile = %s \n",sub_profile->apn_profile->apn_profile_name);
                printf("Access profile  = %s \n",sub_profile->access_profile->access_profile_name);
                printf("Qos profile = %s \n",sub_profile->qos_profile->qos_profile_name);
            }
        }
#if TEST_FILTER
        {
            printf("****TEST 2 ****\n");
            sub_config_t *sub_profile;
            sub_selection_keys_t key = {0};
            key.imsi.is_valid = true;
            key.imsi.from_imsi = 301111111111111;
            key.plmn.is_valid = true;
            key.plmn.tac = 1;
            uint8_t plmn[3] = {0x02, 0xf8, 0x01};
            memcpy(&key.plmn.plmn[0], &plmn[0], 3);
            sub_profile = match_sub_selection(&key);
            printf("sub_profile = %p \n",sub_profile);
            if(sub_profile != NULL)
            {
                printf("Up profile = %s \n",sub_profile->up_profile->user_plane_profile_name);
                printf("APN profile = %s \n",sub_profile->apn_profile->apn_profile_name);
                printf("Access profile  = %s \n",sub_profile->access_profile->access_profile_name);
                printf("Qos profile = %s \n",sub_profile->qos_profile->qos_profile_name);
            }
        }
        {
            printf("****TEST 3 ****\n");
            sub_selection_keys_t key = {0};
            key.imsi.is_valid = true;
            key.imsi.from_imsi = 301111111111111;
            sub_config_t *sub_profile;
            sub_profile = match_sub_selection(&key);
            printf("sub_profile = %p \n",sub_profile);
            if(sub_profile != NULL)
            {
                printf("Up profile = %s \n",sub_profile->up_profile->user_plane_profile_name);
                printf("APN profile = %s \n",sub_profile->apn_profile->apn_profile_name);
                printf("Access profile  = %s \n",sub_profile->access_profile->access_profile_name);
                printf("Qos profile = %s \n",sub_profile->qos_profile->qos_profile_name);
            }
        }
#endif
    }
    return 0;
}
