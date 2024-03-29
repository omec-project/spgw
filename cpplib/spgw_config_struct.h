// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SPGW_CONFIG_STRUCT__
#define __SPGW_CONFIG_STRUCT__
#include "stdbool.h"
#include "stdint.h"
#include <sys/queue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/limits.h>
#include "cp_config_defs.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct imsi_selection
{
    bool      is_valid;
    uint64_t  from_imsi;
    uint64_t  to_imsi;
}imsi_selection_t;

enum cfg_action
{
       DISABLE_UPF = 01,
};

typedef struct config_callback
{
    enum cfg_action  action;
    char             upf_service_name[64]; 
}config_callback_t;

typedef void (*cfg_func_ptr)(config_callback_t *);

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
    uint8_t  qci;
    uint8_t  arp;
}qos_profile_t;

typedef struct user_plane_profile
{
    char user_plane_profile_name[64];
    char user_plane_service[64];
    bool     global_address; /* true : control plane allocates address, false : upf allocates address */
}user_plane_profile_t;

typedef struct user_plane_service_names {
    char user_plane_service[64];
    bool global_address;
}user_plane_service_names_t;

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

/* return structrue for subscriber config. No pointers, only values */
typedef struct sub_config
{
    uint32_t                dns_primary;
    uint32_t                dns_secondary;
    uint16_t                mtu;
    bool                    global_address; /* true : control plane allocates address, false : upf allocates address */
    char                    user_plane_service[64];
    uint32_t                apn_ambr_ul;
    uint32_t                apn_ambr_dl;
    uint8_t                 qci;
    uint8_t                 arp;
}sub_config_t;

/* return value to parse config */
typedef struct spgw_config_profile
{
   void *config; /* C code should not typecast this to any structure. */ 
}spgw_config_profile_t;

struct t2tMsg 
{
    uint16_t event;
    void     *data;
};

typedef struct ue_pool_dynamic
{
    void *dynamic_pool;
}ue_pool_dynamic_t;

/*
- * Define type of Control Plane (CP)
- * SGWC - Serving GW Control Plane
- * PGWC - PDN GW Control Plane
- * SAEGWC - Combined SAEGW Control Plane
- */
enum cp_mode
{
       SGWC = 01,
       PGWC = 02,
       SAEGWC = 03,
};

typedef struct cp_config
{
	/* CP Configuration : SGWC=01; PGWC=02; SAEGWC=03 */
	enum cp_mode cp_type;

	/* Control-Plane IPs and Ports Params. */
	struct in_addr s11_ip;
	uint16_t s11_port;

	/* MME Params. */
	struct in_addr s11_mme_ip;

	uint16_t s5s8_port;
	struct in_addr s5s8_ip;

	struct in_addr pfcp_ip;
	uint16_t pfcp_port;

	uint16_t prom_port;

    uint16_t webserver_port;

	/* RESTORATION PARAMETERS */
	uint8_t transmit_cnt;
	int transmit_timer;
	int periodic_timer;

	/* CP Timer Parameters */
	uint8_t request_tries;
	int request_timeout;    /* Request time out in milisecond */

    spgw_config_profile_t *subscriber_rulebase;

	/* IP_POOL_CONFIG Params */
	struct in_addr ip_pool_ip;
	struct in_addr ip_pool_mask;

	/* STATIC_IP_POOL_CONFIG Params */
	struct in_addr static_ip_pool_ip;
	struct in_addr static_ip_pool_mask;

    uint32_t  gx_enabled;
    uint32_t  urr_enable;
    bool      pfcp_hb_ts_fail;
    uint32_t  upfdnstimeout;
}cp_config_t;

extern cp_config_t *cp_config;


#ifdef __cplusplus
}
#endif
#endif
