// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <rte_malloc.h>
#include <rte_common.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_cfgfile.h>
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "assert.h"
#include "cp_config_apis.h"
#include "clogger.h"
#include "cp_init.h"
#include "cp_config.h"
#include "cp_config_defs.h"
#include "monitor_config.h"
#include "gw_adapter.h"
#include "cp_config.h"
#include "cp_config_apis.h"
#include "upf_struct.h"
#include "ue.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "ip_pool.h"
#include "spgw_cpp_wrapper.h"
#include "tables/tables.h"
#include "dns_config.h"
#include "cdnshelper.h"


#define GLOBAL_ENTRIES			"GLOBAL"
#define APN_ENTRIES				"APN_CONFIG"
#define NAMESERVER_ENTRIES		"NAMESERVER_CONFIG"
#define IP_POOL_ENTRIES			"IP_POOL_CONFIG"
#define STATIC_IP_POOL_ENTRIES	"STATIC_IP_POOL_CONFIG"
#define CACHE_ENTRIES			"CACHE"
#define APP_ENTRIES				"APP"
#define OPS_ENTRIES				"OPS"

#define CP_TYPE					"CP_TYPE"
#define GX_CONFIG               "GX_CONFIG"
#define CP_LOGGER				"CP_LOGGER"
#define DNS_ENABLE				"DNS_ENABLE"
#define S11_IPS					"S11_IP"
#define S11_PORTS				"S11_PORT"
#define S5S8_IPS				"S5S8_IP"
#define S5S8_PORTS				"S5S8_PORT"
#define PFCP_IPS				"PFCP_IP"
#define PFCP_PORTS				"PFCP_PORT"
#define MME_S11_IPS				"MME_S11_IP"
#define MME_S11_PORTS			"MME_S11_PORT"
#define UPF_PFCP_IPS			"UPF_PFCP_IP"
#define UPF_PFCP_PORTS			"UPF_PFCP_PORT"
#define PROMETHEUS_PORT			"PROMETHEUS_PORT"
#define APN						"APN"
#define NAMESERVER				"nameserver"
#define IP_POOL_IP				"IP_POOL_IP"
#define IP_POOL_MASK			"IP_POOL_MASK"
#define CONCURRENT				"concurrent"
#define PERCENTAGE				"percentage"
#define INT_SEC					"interval_seconds"
#define FREQ_SEC				"frequency_seconds"
#define FILENAME				"filename"
#define QUERY_TIMEOUT           "query_timeout_ms"
#define QUERY_TRIES             "query_tries"

/* Restoration Parameters */
#define TRANSMIT_TIMER			"TRANSMIT_TIMER"
#define PERIODIC_TIMER			"PERIODIC_TIMER"
#define TRANSMIT_COUNT			"TRANSMIT_COUNT"
/* CP Timer Parameter */
#define REQUEST_TIMEOUT 		"REQUEST_TIMEOUT"
#define REQUEST_TRIES			"REQUEST_TRIES"

extern struct ip_table *static_addr_pool;
extern char* config_update_base_folder; 
extern bool native_config_folder;
cp_config_t *cp_config = NULL;
void set_dns_config(void);

void init_config(void) 
{
    /*Global config holder for cp */
    cp_config = (cp_config_t *) calloc(1, sizeof(cp_config_t));

    if (cp_config == NULL) {
        rte_exit(EXIT_FAILURE, "Can't allocate memory for cp_config!\n");
    }
    
    config_cp_ip_port(cp_config);

    if(cp_config->dns_enable) {
        set_dns_config();
    }

    init_cli_module(cp_config->cp_logger);

    /* start - dynamic config */
    cp_config->subscriber_rulebase = (spgw_config_profile_t *) calloc(1, sizeof(spgw_config_profile_t));
    if (cp_config->subscriber_rulebase == NULL) {
        rte_exit(EXIT_FAILURE, "Can't allocate memory for subscriber_rulebase!\n");
    }

    /* Parse initial configuration file */
    cp_config->subscriber_rulebase = parse_subscriber_profiles_c(CP_CONFIG_SUB_RULES);
    set_cp_config(cp_config->subscriber_rulebase);

    char file[128] = {'\0'};
    strcat(file, config_update_base_folder);
    strcat(file, "subscriber_mapping.json");
    clLog(clSystemLog, eCLSeverityDebug, "Config file to monitor %s \n", file);
    register_config_updates(file);

    return;
}

void
config_cp_ip_port(cp_config_t *cp_config)
{
    int32_t i = 0;
    int32_t num_ops_entries = 0;
    int32_t num_app_entries = 0;
    int32_t num_cache_entries = 0;
    int32_t num_ip_pool_entries = 0;
    int32_t num_global_entries = 0;

    struct rte_cfgfile_entry *global_entries = NULL;
    struct rte_cfgfile_entry *ip_pool_entries = NULL;
    struct rte_cfgfile_entry *static_ip_pool_entries = NULL;
    struct rte_cfgfile_entry *cache_entries = NULL;
    struct rte_cfgfile_entry *app_entries = NULL;
    struct rte_cfgfile_entry *ops_entries = NULL;


    /* default valueas */
    cp_config->dns_enable = 0;
    cp_config->gx_enabled = 0;
    cp_config->prom_port = 3082;

    struct rte_cfgfile *file = rte_cfgfile_load(STATIC_CP_FILE, 0);
    if (file == NULL) {
        rte_exit(EXIT_FAILURE, "Cannot load configuration file %s\n",
                STATIC_CP_FILE);
    }

    fprintf(stderr, "CP: PFCP Config Parsing %s\n", STATIC_CP_FILE);

    /* Read GLOBAL seaction values and configure respective params. */
    num_global_entries = rte_cfgfile_section_num_entries(file, GLOBAL_ENTRIES);

    if (num_global_entries > 0) {
        global_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry) *
                num_global_entries,
                RTE_CACHE_LINE_SIZE, rte_socket_id());
    }

    if (global_entries == NULL) {
        rte_panic("Error configuring global entry of %s\n",
                STATIC_CP_FILE);
    }

    rte_cfgfile_section_entries(file, GLOBAL_ENTRIES, global_entries,
            num_global_entries);

    for (i = 0; i < num_global_entries; ++i) {

        /* Parse SGWC, PGWC and SAEGWC values from cp.cfg */
        if(strncmp(CP_TYPE, global_entries[i].name, strlen(CP_TYPE)) == 0) {
            cp_config->cp_type = (uint8_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: CP_TYPE     : %s\n",
                    cp_config->cp_type == SGWC ? "SGW-C" :
                    cp_config->cp_type == PGWC ? "PGW-C" :
                    cp_config->cp_type == SAEGWC ? "SAEGW-C" : "UNKNOWN");

        } 
       /* Parse SGWC, PGWC and SAEGWC values from cp.cfg */
        else if(strncmp(GX_CONFIG, global_entries[i].name, strlen(GX_CONFIG)) == 0) {
            cp_config->gx_enabled = (uint8_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: GX_CONFIG     : %s\n", cp_config->gx_enabled == 1 ? "enabled" : "disabled");

        }else if (strncmp(S11_IPS, global_entries[i].name,
                    strlen(S11_IPS)) == 0) {

            /* TODO mme address is not required to be configured at SPGW */
            inet_aton(global_entries[i].value,
                    &(cp_config->s11_ip));

            fprintf(stderr, "CP: S11_IP      : %s\n",
                    inet_ntoa(cp_config->s11_ip));

        }else if (strncmp(S11_PORTS, global_entries[i].name,
                    strlen(S11_PORTS)) == 0) {

            cp_config->s11_port =
                (uint16_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: S11_PORT    : %d\n",
                    cp_config->s11_port);

        } else if (strncmp(S5S8_IPS, global_entries[i].name,
                    strlen(S5S8_IPS)) == 0) {

            inet_aton(global_entries[i].value,
                    &(cp_config->s5s8_ip));

            fprintf(stderr, "CP: S5S8_IP     : %s\n",
                    inet_ntoa(cp_config->s5s8_ip));

        } else if (strncmp(S5S8_PORTS, global_entries[i].name,
                    strlen(S5S8_PORTS)) == 0) {

            cp_config->s5s8_port =
                (uint16_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: S5S8_PORT   : %d\n",
                    cp_config->s5s8_port);

        } else if (strncmp(PFCP_IPS , global_entries[i].name,
                    strlen(PFCP_IPS)) == 0) {

            inet_aton(global_entries[i].value,
                    &(cp_config->pfcp_ip));

            fprintf(stderr, "CP: PFCP_IP     : %s\n",
                    inet_ntoa(cp_config->pfcp_ip));

        } else if (strncmp(PFCP_PORTS, global_entries[i].name,
                    strlen(PFCP_PORTS)) == 0) {

            cp_config->pfcp_port =
                (uint16_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: PFCP_PORT   : %d\n",
                    cp_config->pfcp_port);

        } else if (strncmp(PROMETHEUS_PORT, global_entries[i].name,
                    strlen(PROMETHEUS_PORT)) == 0) {

            cp_config->prom_port =
                (uint16_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: PROMETHEUS_PORT   : %d\n",
                    cp_config->prom_port);

        } else if (strncmp(UPF_PFCP_PORTS, global_entries[i].name,
                    strlen(UPF_PFCP_PORTS)) == 0) {

            cp_config->upf_pfcp_port =
                (uint16_t)atoi(global_entries[i].value);

            fprintf(stderr, "CP: UPF_PFCP_PORT: %d\n",
                    cp_config->upf_pfcp_port);

        } else if (strncmp(CP_LOGGER, global_entries[i].name, strlen(CP_LOGGER)) == 0) {
            cp_config->cp_logger = (uint8_t)atoi(global_entries[i].value);
            fprintf(stderr, "CP: CP_LOGGER: %d\n",
                    cp_config->cp_logger);
        } else if (strncmp(DNS_ENABLE, global_entries[i].name, strlen(DNS_ENABLE)) == 0) {
            cp_config->dns_enable = (uint8_t)atoi(global_entries[i].value);
            fprintf(stderr, "CP: DNS_ENABLE : %d\n",
                    cp_config->dns_enable);
        }

        /* Parse timer and counter values from cp.cfg */
        if(strncmp(TRANSMIT_TIMER, global_entries[i].name, strlen(TRANSMIT_TIMER)) == 0) {
            cp_config->transmit_timer = (int)atoi(global_entries[i].value);
            fprintf(stderr, "CP: TRANSMIT_TIMER: %d\n",
                    cp_config->transmit_timer);
        }

        if(strncmp(PERIODIC_TIMER, global_entries[i].name, strlen(PERIODIC_TIMER)) == 0) {
            cp_config->periodic_timer = (int)atoi(global_entries[i].value);
            fprintf(stderr, "CP: PERIODIC_TIMER: %d\n",
                    cp_config->periodic_timer);
        }

        if(strncmp(TRANSMIT_COUNT, global_entries[i].name, strlen(TRANSMIT_COUNT)) == 0) {
            cp_config->transmit_cnt = (uint8_t)atoi(global_entries[i].value);
            fprintf(stderr, "CP: TRANSMIT_COUNT: %u\n",
                    cp_config->transmit_cnt);
        }

        /* Parse CP Timer Request Time Out and Retries Values from cp.cfg */
        if(strncmp(REQUEST_TIMEOUT, global_entries[i].name, strlen(REQUEST_TIMEOUT)) == 0){
            if(check_cp_req_timeout_config(global_entries[i].value) == 0) {
                cp_config->request_timeout = (int)atoi(global_entries[i].value);
                fprintf(stderr, "CP: REQUEST_TIMEOUT: %d\n",
                        cp_config->request_timeout);
            } else {
                rte_panic("Error configuring "
                        "CP TIMER "REQUEST_TIMEOUT" invalid entry of %s\n", STATIC_CP_FILE);
            }
        }else {
            /* if CP Request Timer Parameter is not present is cp.cfg */
            /* Defualt Request Timerout value */
            /* 5 minute = 300000 milisecond  */
            if(cp_config->request_timeout == 0) {
                cp_config->request_timeout = 300000;
                fprintf(stderr, "CP: DEFAULT REQUEST_TIMEOUT: %d\n",
                        cp_config->request_timeout);
            }
        }

        if(strncmp(REQUEST_TRIES, global_entries[i].name, strlen(REQUEST_TRIES)) == 0) {
            if(check_cp_req_tries_config(global_entries[i].value) == 0) {
                cp_config->request_tries = (uint8_t)atoi(global_entries[i].value);
                fprintf(stderr, "CP: REQUEST_TRIES: %d\n",
                        cp_config->request_tries);
            } else {
                rte_panic("Error configuring "
                        "CP TIMER "REQUEST_TRIES" invalid entry of %s\n", STATIC_CP_FILE);
            }

        } else {
            /* if CP Request Timer Parameter is not present is cp.cfg */
            /* Defualt Request Retries value */
            if(cp_config->request_tries == 0) {
                cp_config->request_tries = 3;
                fprintf(stderr, "CP: DEFAULT REQUEST_TRIES: %d\n",
                        cp_config->request_tries);
            }
        }
    }

    rte_free(global_entries);
    uint16_t app_nameserver_ip_idx = 0;
    uint16_t ops_nameserver_ip_idx = 0;

    /* Read cache values from cfg seaction. */
    num_cache_entries =
        rte_cfgfile_section_num_entries(file, CACHE_ENTRIES);

    if (num_cache_entries > 0) {
        cache_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry)
                *num_cache_entries,
                RTE_CACHE_LINE_SIZE,
                rte_socket_id());
    }

    if (cache_entries == NULL)
        rte_panic("Error configuring"
                "CACHE entry of %s\n", STATIC_CP_FILE);

    rte_cfgfile_section_entries(file, CACHE_ENTRIES,
            cache_entries,
            num_cache_entries);

    for (i = 0; i < num_cache_entries; ++i) {
        fprintf(stderr, "CP: [%s] = %s\n",
                cache_entries[i].name,
                cache_entries[i].value);
        if (strncmp(CONCURRENT, cache_entries[i].name,
                    strlen(CONCURRENT)) == 0)
            cp_config->dns_cache.concurrent =
                (uint32_t)atoi(cache_entries[i].value);
        if (strncmp(PERCENTAGE, cache_entries[i].name,
                    strlen(CONCURRENT)) == 0)
            cp_config->dns_cache.percent =
                (uint32_t)atoi(cache_entries[i].value);
        if (strncmp(INT_SEC, cache_entries[i].name,
                    strlen(CONCURRENT)) == 0)
            cp_config->dns_cache.sec =
                (((uint32_t)atoi(cache_entries[i].value)) * 1000);
        if (strncmp(QUERY_TIMEOUT, cache_entries[i].name,
                    strlen(QUERY_TIMEOUT)) == 0)
            cp_config->dns_cache.timeoutms =
                (long)atol(cache_entries[i].value);
        if (strncmp(QUERY_TRIES, cache_entries[i].name,
                    strlen(QUERY_TRIES)) == 0)
            cp_config->dns_cache.tries =
                (uint32_t)atoi(cache_entries[i].value);
    }

    rte_free(cache_entries);

    /* Read app values from cfg seaction. */
    num_app_entries =
        rte_cfgfile_section_num_entries(file, APP_ENTRIES);

    if (num_app_entries > 0) {
        app_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry)
                *num_app_entries,
                RTE_CACHE_LINE_SIZE,
                rte_socket_id());
    }

    if (app_entries == NULL)
        rte_panic("Error configuring"
                "APP entry of %s\n", STATIC_CP_FILE);

    rte_cfgfile_section_entries(file, APP_ENTRIES,
            app_entries,
            num_app_entries);

    for (i = 0; i < num_app_entries; ++i) {
        fprintf(stderr, "CP: [%s] = %s\n",
                app_entries[i].name,
                app_entries[i].value);

        if (strncmp(FREQ_SEC, app_entries[i].name,
                    strlen(FREQ_SEC)) == 0)
            cp_config->app_dns.freq_sec =
                (uint8_t)atoi(app_entries[i].value);

        if (strncmp(FILENAME, app_entries[i].name,
                    strlen(FILENAME)) == 0)
            strncpy(cp_config->app_dns.filename,
                    app_entries[i].value,
                    strlen(app_entries[i].value));

        if (strncmp(NAMESERVER, app_entries[i].name,
                    strlen(NAMESERVER)) == 0) {
            strncpy(cp_config->app_dns.nameserver_ip[app_nameserver_ip_idx],
                    app_entries[i].value,
                    strlen(app_entries[i].value));
            app_nameserver_ip_idx++;
        }
    }

    cp_config->app_dns.nameserver_cnt = app_nameserver_ip_idx;

    rte_free(app_entries);

    /* Read ops values from cfg seaction. */
    num_ops_entries =
        rte_cfgfile_section_num_entries(file, OPS_ENTRIES);

    if (num_ops_entries > 0) {
        ops_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry)
                *num_ops_entries,
                RTE_CACHE_LINE_SIZE,
                rte_socket_id());
    }

    if (ops_entries == NULL)
        rte_panic("Error configuring"
                "OPS entry of %s\n", STATIC_CP_FILE);

    rte_cfgfile_section_entries(file, OPS_ENTRIES,
            ops_entries,
            num_ops_entries);

    for (i = 0; i < num_ops_entries; ++i) {
        fprintf(stderr, "CP: [%s] = %s\n",
                ops_entries[i].name,
                ops_entries[i].value);

        if (strncmp(FREQ_SEC, ops_entries[i].name,
                    strlen(FREQ_SEC)) == 0)
            cp_config->ops_dns.freq_sec =
                (uint8_t)atoi(ops_entries[i].value);

        if (strncmp(FILENAME, ops_entries[i].name,
                    strlen(FILENAME)) == 0)
            strncpy(cp_config->ops_dns.filename,
                    ops_entries[i].value,
                    strlen(ops_entries[i].value));

        if (strncmp(NAMESERVER, ops_entries[i].name,
                    strlen(NAMESERVER)) == 0) {
            strncpy(cp_config->ops_dns.nameserver_ip[ops_nameserver_ip_idx],
                    ops_entries[i].value,
                    strlen(ops_entries[i].value));
            ops_nameserver_ip_idx++;
        }
    }

    cp_config->ops_dns.nameserver_cnt = ops_nameserver_ip_idx;

    rte_free(ops_entries);

    /* Read IP_POOL_CONFIG seaction */
    num_ip_pool_entries = rte_cfgfile_section_num_entries
        (file, IP_POOL_ENTRIES);


    if (num_ip_pool_entries > 0) {
        ip_pool_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry) *
                num_ip_pool_entries,
                RTE_CACHE_LINE_SIZE,
                rte_socket_id());
        if (ip_pool_entries == NULL)
            rte_panic("Error configuring ip"
                    "pool entry of %s\n", STATIC_CP_FILE);
    }

    rte_cfgfile_section_entries(file, IP_POOL_ENTRIES,
            ip_pool_entries,
            num_ip_pool_entries);

    for (i = 0; i < num_ip_pool_entries; ++i) {
        fprintf(stderr, "CP: [%s] = %s\n",
                ip_pool_entries[i].name,
                ip_pool_entries[i].value);
        if (strncmp(IP_POOL_IP,
                    ip_pool_entries[i].name,
                    strlen(IP_POOL_IP)) == 0) {
            inet_aton(ip_pool_entries[i].value,
                    &(cp_config->ip_pool_ip));
        } else if (strncmp
                (IP_POOL_MASK, ip_pool_entries[i].name,
                 strlen(IP_POOL_MASK)) == 0) {
            inet_aton(ip_pool_entries[i].value,
                    &(cp_config->ip_pool_mask));
        }
    }

    rte_free(ip_pool_entries);

    /* Read STATIC_IP_POOL_CONFIG seaction */
    num_ip_pool_entries = rte_cfgfile_section_num_entries
        (file, STATIC_IP_POOL_ENTRIES);


    if (num_ip_pool_entries > 0) {
        static_ip_pool_entries = rte_malloc_socket(NULL,
                sizeof(struct rte_cfgfile_entry) *
                num_ip_pool_entries,
                RTE_CACHE_LINE_SIZE,
                rte_socket_id());
        if (static_ip_pool_entries == NULL)
            rte_panic("Error configuring static ip"
                    "pool entry of %s\n", STATIC_CP_FILE);
    }

    rte_cfgfile_section_entries(file, STATIC_IP_POOL_ENTRIES,
            static_ip_pool_entries,
            num_ip_pool_entries);

    bool addr_configured=false;
    bool mask_configured=false;
    for (i = 0; i < num_ip_pool_entries; ++i) {
        fprintf(stderr, "CP: [%s] = %s\n",
                static_ip_pool_entries[i].name,
                ip_pool_entries[i].value);
        if (strncmp(IP_POOL_IP,
                    static_ip_pool_entries[i].name,
                    strlen(IP_POOL_IP)) == 0) {
            inet_aton(static_ip_pool_entries[i].value,
                    &(cp_config->static_ip_pool_ip));
            cp_config->static_ip_pool_ip.s_addr = ntohl(cp_config->static_ip_pool_ip.s_addr);  
        } else if (strncmp
                (IP_POOL_MASK, static_ip_pool_entries[i].name,
                 strlen(IP_POOL_MASK)) == 0) {
            inet_aton(static_ip_pool_entries[i].value,
                    &(cp_config->static_ip_pool_mask));
            cp_config->static_ip_pool_mask.s_addr = ntohl(cp_config->static_ip_pool_mask.s_addr);  
        }
        if(addr_configured && mask_configured) {
            addr_configured = true;
            mask_configured = true;
			static_addr_pool = create_ue_pool(cp_config->static_ip_pool_ip, cp_config->static_ip_pool_mask);
        }
    }

    rte_free(static_ip_pool_entries);
    return;
}

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


int
check_cp_req_timeout_config(char *value) {
	unsigned int idx = 0;
	if(value == NULL )
	        return -1;
	/* check string has all digit 0 to 9 */
	for(idx = 0; idx < strlen(value); idx++) {
	        if(isdigit(value[idx])  == 0) {
	                return -1;
	        }
	}
	/* check cp request timer timeout range */
	if((int)atoi(value) >= 1 && (int)atoi(value) <= 1800000 ) {
	        return 0;
	}

	return -1;
}

int
check_cp_req_tries_config(char *value) 
{
	unsigned int idx = 0;
	if(value == NULL )
	        return -1;
	/* check string has all digit 0 to 9 */
	for(idx = 0; idx < strlen(value); idx++) {
	        if(isdigit(value[idx])  == 0) {
	                return -1;
	        }
	}
	/* check cp request timer tries range */
	if((int)atoi(value) >= 1 && (int)atoi(value) <= 20) {
	        return 0;
	}
	return -1;
}

void
parse_apn_args(char *temp, char *ptr[3])
{

	int i;
	char *first = temp;
	char *next = NULL;

	for (i = 0; i < 3; i++) {
		ptr[i] = malloc(100);
		memset(ptr[i], 0, 100);
	}

	for (i = 0; i < 3; i++) {

		if(first!=NULL)
			next = strchr(first, ',');

		if(first == NULL && next == NULL)
		{
			ptr[i] = NULL;
			continue;
		}

		if(*(first) == '\0')  //string ends,fill remaining with NULL
		{
			ptr[i] = NULL;
			continue;
		}

		if(next!= NULL)
		{
			if(next > first) //string is present
			{
				strncpy(ptr[i], first, next - first);

			}
			else if (next == first) //first place is comma
			{
				ptr[i] = NULL;
			}
			first = next + 1;
		} else                //copy last string
		{
			if(first!=NULL)
			{
				strcpy(ptr[i],first);
				first = NULL;
			} else {
				ptr[i] = NULL; //fill remaining ptr with NULL
			}

		}
	}

}

void
config_change_cbk(char *config_file, uint32_t flags)
{
    clLog(clSystemLog, eCLSeverityCritical, "Config change trigger function - %s" 
                " - file %s and flags = %x \n", __FUNCTION__, config_file, flags);
	if (native_config_folder == false) {
		/* Move the updated config to standard path */
		char cmd[256];
		sprintf(cmd, "cp %s %s", config_file, CP_CONFIG_SUB_RULES);
		int ret = system(cmd);
        clLog(clSystemLog, eCLSeverityDebug, "System call return value %d", ret);
    }
 
	/* We dont expect quick updates from configmap..One update per interval. Typically 
	 * worst case 60 seconds for 1 config update. Updates are clubbed and dont come frequent 
	 * We re-register to avoid recursive callbacks 
	 */
	watch_config_change(config_file, config_change_cbk);

	cp_config->subscriber_rulebase = parse_subscriber_profiles_c(CP_CONFIG_SUB_RULES);
    
    switch_config(cp_config->subscriber_rulebase);
    
}

void 
register_config_updates(char *file)
{
	/* I would prefer a complete path than this relative path.
	 * Looks like it may break */
	watch_config_change(file, config_change_cbk);
}

/* Requirement: 
 * For now I am using linux system call to do the service name dns resolution...
 * 3gpp based DNS lookup of NRF support would be required to locate UPF. 
 */
struct in_addr native_linux_name_resolve(const char *name)
{
    struct in_addr ip = {0};
    printf("Function [%s] - Line - %d - %s - \n",__FUNCTION__,__LINE__,name);
    struct addrinfo hints;
    struct addrinfo *result=NULL, *rp=NULL;
    int err;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    err = getaddrinfo(name, NULL, &hints, &result);
    if (err != 0)
    {
        // Keep trying ...May be SGW is not yet deployed
        // We shall be doing this once timer library is integrated
        printf("getaddrinfo: %s\n", gai_strerror(err));
    }
    else
    {
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            if(rp->ai_family == AF_INET)
            {
                struct sockaddr_in *addrV4 = (struct sockaddr_in *)rp->ai_addr;
                printf("gw address received from DNS response %s\n", inet_ntoa(addrV4->sin_addr));
                return addrV4->sin_addr;
            }
        }
    }
    printf("Function [%s] - Line - %d - %s - name resolution failed \n",__FUNCTION__,__LINE__,name);
    return ip;
}

sub_profile_t *
get_subscriber_profile(sub_selection_keys_t *key)
{
    sub_profile_t *sub_profile = match_sub_selection(key);
    if(sub_profile == NULL)
    {
        return NULL;
    }
    return sub_profile;
}

upf_context_t*
get_upf_context(user_plane_profile_t *upf_profile) 
{
    struct in_addr ip = {0};
    if(upf_profile == NULL) {
        return NULL;
    }

    if(upf_profile->upf_addr == 0) {
        ip = native_linux_name_resolve(upf_profile->user_plane_service); 
        upf_profile->upf_addr = ip.s_addr; 
    }

    if(upf_profile->upf_addr != 0) {
        upf_context_t *upf_context = NULL;
        upf_context_entry_lookup(upf_profile->upf_addr, &upf_context);
        if(upf_context == NULL) {
            create_upf_context(upf_profile->upf_addr, &upf_context);
        }
        return upf_context;
    }
    printf("DNS Resolution failed Profile - %s and Service name  %s \n",upf_profile->user_plane_profile_name, upf_profile->user_plane_service);
	return NULL; 
}
