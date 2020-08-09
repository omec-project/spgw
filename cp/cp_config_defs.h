
// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _CP_GLOBAL_DEFS_H_
#define _CP_GLOBAL_DEFS_H_

#define IPV4_SIZE 4
#define IPV6_SIZE 16 


#define MAX_NUM_APN   16
#define MAX_NUM_NAMESERVER 8

// default PFCP Ports 
#define SGWU_PFCP_PORT      (8805)
#define PGWU_PFCP_PORT      (8805)
#define SAEGWU_PFCP_PORT    (8805)

#define GTPC_UDP_PORT       (2123)


#define DP_SITE_NAME_MAX		256

#define CP_CONFIG_FOLDER		"../config/"
#define CP_CONFIG_SUB_RULES	    "../config/subscriber_mapping.json"
#define STATIC_CP_FILE          "../config/cp.cfg"
#define METER_PROFILE_FILE      "../config/meter_profile.cfg"
#define PCC_RULE_FILE           "../config/pcc_rules.cfg"
#define SDF_RULE_FILE           "../config/sdf_rules.cfg"
#define ADC_RULE_FILE           "../config/adc_rules.cfg"

#define DEFAULT_STATS_PATH      "./logs/"
#define HEARTBEAT_TIMESTAMP     "../config/hrtbeat_recov_time.txt"
#define RESTART_CNT_FILE        "../config/cp_rstCnt.txt"

#define DEFAULT_IPV4_MTU        (1450)

#define DPN_ID                       (12345)
#endif

