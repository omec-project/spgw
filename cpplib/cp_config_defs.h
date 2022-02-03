
// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#ifndef _CP_GLOBAL_DEFS_H_
#define _CP_GLOBAL_DEFS_H_

#define IPV4_SIZE 4
#define IPV6_SIZE 16 


#define MAX_NUM_APN   16

// default PFCP Ports 
#define SGWU_PFCP_PORT      (8805)
#define PGWU_PFCP_PORT      (8805)
#define SAEGWU_PFCP_PORT    (8805)

#define GTPC_UDP_PORT       (2123)
#define HTTP_SERVER_PORT    (8080)
#define PROMETHEUS_HTTP_PORT (9089)


#define DP_SITE_NAME_MAX		256

#define CP_CONFIG_FOLDER		"../config/"
#define CP_CONFIG_SUB_RULES	    "../config/subscriber_mapping.json"
#define STATIC_CP_JSON_FILE     "../config/cp.json"

#define DEFAULT_STATS_PATH      "./logs/"
#define HEARTBEAT_TIMESTAMP     "../config/hrtbeat_recov_time.txt"
#define RESTART_CNT_FILE        "../config/cp_rstCnt.txt"

#define DPN_ID                       (12345)
#endif

