// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW_CONFIG_STRUCT__
#define __SPGW_CONFIG_STRUCT__
#include "stdint.h"
#include "stdbool.h"
typedef struct imsi_selection
{
    bool      is_valid;
    uint64_t  from_imsi;
    uint64_t  to_imsi;
}imsi_selection_t;

typedef struct serving_plmn
{
    bool      is_valid;
    uint8_t   plmn[3];
    uint16_t  tac;  
}serving_plmn_t;

typedef struct apn_key 
{
    bool      is_valid;
    char requested_apn[64];
} apn_key_t;
typedef struct sub_selection_keys
{
    imsi_selection_t imsi;
    serving_plmn_t plmn; 
    apn_key_t      apn;
} sub_selection_keys_t;

typedef struct access_profile
{
    char access_profile_name[64];

}access_profile_t;

typedef struct qos_profile
{
    char qos_profile_name[64];
    uint32_t apn_ambr_ul;
    uint32_t apn_ambr_dl;
}qos_profile_t;

typedef struct user_plane_profile
{
    char user_plane_profile_name[64];
    char user_plane_service[64];
    uint32_t upf_addr; /* run time information */
}user_plane_profile_t;

#define MAX_NETCAP_LEN               (64)
typedef struct apn_profile 
{
    char apn_profile_name[64];
    char apn_name[64];
    uint8_t apn_name_length;
    int apn_usage_type;
    char apn_net_cap[MAX_NETCAP_LEN];
    bool gx_enabled;
    uint32_t dns_primary;
    uint32_t dns_secondary;
    uint16_t mtu;
} apn_profile_t;

struct sub_selection_rule
{
    uint32_t rule_priority;
    sub_selection_keys_t *keys;
    char* selected_apn_profile;
    char* selected_user_plane_profile;
    char* selected_qos_profile;
    char* selected_access_profile[4]; /* 4 access profiles per subscriber */

};
typedef struct sub_selection_rule sub_selection_rule_t;

/* return structrue for subscriber profile search */
typedef struct sub_profile
{
    user_plane_profile_t    *up_profile;
    apn_profile_t           *apn_profile;
    access_profile_t        *access_profile;
    qos_profile_t           *qos_profile;
}sub_profile_t;

/* return value to parse config */
typedef struct spgw_config_profile
{
   void *config; /* C code should not typecast this to any structure. */ 
}spgw_config_profile_t;

// typical cyle - parse, set_new_config... library to keep one pointer to config at any time. 

#endif
