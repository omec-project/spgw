// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "assert.h"
#include "spgw_config_struct.h"
#include "cp_config_apis.h"
#include "cp_init.h"
#include "cp_config_defs.h"
#include "monitor_config.h"
#include "spgw_cpp_wrapper.h"
#include "dns_config.h"
#include "cp_log.h"
#include "cp_events.h"
#include "cp_io_poll.h"
#include "sm_hand.h"
#include "upf_apis.h"

#ifdef __cplusplus
extern "C" {
#endif
cp_config_t *cp_config = NULL;
#ifdef __cplusplus
}
#endif


void init_config(void) 
{
	LOG_MSG(LOG_INIT, "initializing configuration");

    /*Global config holder for cp */
    cp_config = (cp_config_t *) calloc(1, sizeof(cp_config_t));

    if (cp_config == NULL) {
        assert(0);
    }
    
    char *pod_ip = getenv("POD_IP");
    if(pod_ip != NULL) {
        inet_aton(pod_ip, &(cp_config->s11_ip));
        cp_config->s5s8_ip = cp_config->s11_ip;
        cp_config->pfcp_ip = cp_config->s11_ip;
    }

    config_cp_ip_port(cp_config);


#if 0
    if(cp_config->dns_enable) {
        set_dns_config();
    }
#endif

    /* Parse initial configuration file */
    cp_config->subscriber_rulebase = parse_subscriber_profiles_c(CP_CONFIG_SUB_RULES);

    /* If this env var is defined, monitoring subscriber_mapping.json will be disabled */
    char *disable_config_watcher = getenv("DISABLE_CONFIG_WATCHER");

    char file[128] = {'\0'};
    strcat(file, config_update_base_folder);
    strcat(file, "subscriber_mapping.json");
    if(disable_config_watcher == NULL) {
        LOG_MSG(LOG_INIT,"Config file to monitor %s ", file);
        watch_config_change(file, config_change_cbk);
    } else {
        LOG_MSG(LOG_INIT,"Monitoring %s is disabled ", file);
    }

    char cfgfile[128] = {'\0'};
    strcat(cfgfile, config_update_base_folder);
    strcat(cfgfile, "cp.json");
    LOG_MSG(LOG_INIT,"Config file to monitor %s ", cfgfile);
    watch_config_change(cfgfile, cpconfig_change_cbk);


    // thread to read incoming socker messages from local socket - config change listen 
    pthread_t readerLocal_t;
    pthread_attr_t localattr;
    pthread_attr_init(&localattr);
    pthread_attr_setdetachstate(&localattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&readerLocal_t, &localattr, &msg_handler_local, NULL);
    pthread_attr_destroy(&localattr);

    schedule_pfcp_association(10, NULL);
    return;
}

void* 
msg_handler_local(void *data)
{
    int bytes_rx;
    uint8_t rx_buf[128];
    LOG_MSG(LOG_INIT, "Starting local message handler thread ");
    while(1) {
        bytes_rx = recv(my_sock.sock_fd_local, rx_buf, sizeof(rx_buf), 0);
        if(bytes_rx > 0) {
            LOG_MSG(LOG_INFO, "config read event received %d ", bytes_rx);
            queue_stack_unwind_event(LOCAL_MSG_RECEIVED, (void *)NULL, process_local_msg);
        }
    }
    LOG_MSG(LOG_ERROR,"Exiting local message handler thread %p", data);
    return NULL;
}

void
config_cp_ip_port(cp_config_t *cp_config)
{
    LOG_MSG(LOG_INIT, "CP: S11_IP      : %s", inet_ntoa(cp_config->s11_ip));
    LOG_MSG(LOG_INIT, "CP: S5S8_IP     : %s", inet_ntoa(cp_config->s5s8_ip));
    LOG_MSG(LOG_INIT, "CP: PFCP_IP     : %s", inet_ntoa(cp_config->pfcp_ip));

    /* default valueas */
    cp_config->cp_type = SAEGWC; 
    cp_config->dns_enable = 0;  // disabled by default
    cp_config->gx_enabled = 0;  // disabled by default
    cp_config->urr_enable = 0;  // disabled by default
    cp_config->prom_port = PROMETHEUS_HTTP_PORT;
    cp_config->webserver_port = HTTP_SERVER_PORT; //default webserver_port
    cp_config->pfcp_hb_ts_fail = false; /* Dont detect path failure based on echo timeout */

    parse_cp_json(cp_config, STATIC_CP_JSON_FILE);

    return;
}

#if 0
/**
 * @brief  : Set dns configurations parameters
 * @param  : void
 * @return : void
 */
void
set_dns_config(void)
{
	set_dnscache_refresh_params(cp_config->dns_cache.concurrent,
			cp_config->dns_cache.percent, cp_config->dns_cache.sec);

	set_dns_retry_params(cp_config->dns_cache.timeoutms,
			cp_config->dns_cache.tries);

	/* set OPS dns config */
	for (uint32_t i = 0; i < cp_config->ops_dns.nameserver_cnt; i++)
	{
		set_nameserver_config(cp_config->ops_dns.nameserver_ip[i],
				DNS_PORT, DNS_PORT, NS_OPS);
	}

	apply_nameserver_config(NS_OPS);
	init_save_dns_queries(NS_OPS, cp_config->ops_dns.filename,
			cp_config->ops_dns.freq_sec);
	load_dns_queries(NS_OPS, cp_config->ops_dns.filename);

	/* set APP dns config */
	for (uint32_t i = 0; i < cp_config->app_dns.nameserver_cnt; i++)
		set_nameserver_config(cp_config->app_dns.nameserver_ip[i],
				DNS_PORT, DNS_PORT, NS_APP);

	apply_nameserver_config(NS_APP);
	init_save_dns_queries(NS_APP, cp_config->app_dns.filename,
			cp_config->app_dns.freq_sec);
	load_dns_queries(NS_APP, cp_config->app_dns.filename);
}
#endif

void
config_change_cbk(char *config_file, uint32_t flags)
{
    LOG_MSG(LOG_INIT, "Config change trigger " 
                " - file %s and flags = %x ", config_file, flags);
	if (native_config_folder == false) {
		/* Move the updated config to standard path */
		char cmd[256];
		sprintf(cmd, "cp %s %s", config_file, CP_CONFIG_SUB_RULES);
		int ret = system(cmd);
        LOG_MSG(LOG_INIT, "System call return value %d", ret);
    }
 
	/* We dont expect quick updates from configmap..One update per interval. Typically 
	 * worst case 60 seconds for 1 config update. Updates are clubbed and dont come frequent 
	 * We re-register to avoid recursive callbacks 
	 */
	watch_config_change(config_file, config_change_cbk);

    spgw_config_profile_t *subscriber_rulebase = parse_subscriber_profiles_c(CP_CONFIG_SUB_RULES);

    if(subscriber_rulebase == NULL) {
        LOG_MSG(LOG_INIT, "subscriber_rulebase null ");
        return;
    }
    cp_config->subscriber_rulebase = subscriber_rulebase;
}

void
cpconfig_change_cbk(char *config_file, uint32_t flags)
{
    LOG_MSG(LOG_INIT, "Config change trigger function - " 
                " file %s and flags = %x ", config_file, flags);
	if (native_config_folder == false) {
		/* Move the updated config to standard path */
		char cmd[256];
		sprintf(cmd, "cp %s %s", config_file, STATIC_CP_JSON_FILE);
		int ret = system(cmd);
        LOG_MSG(LOG_DEBUG, "System call return value %d", ret);
    }
 
    parse_cp_json(cp_config, STATIC_CP_JSON_FILE);
	/* We dont expect quick updates from configmap..One update per interval. Typically 
	 * worst case 60 seconds for 1 config update. Updates are clubbed and dont come frequent 
	 * We re-register to avoid recursive callbacks 
	 */
	watch_config_change(config_file, cpconfig_change_cbk);

    return;
}
