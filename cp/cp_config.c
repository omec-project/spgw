/*
 * Copyright 2019-present Open Networking Foundation
 * Copyright (c) 2019 Sprint
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <rte_common.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_cfgfile.h>
#include "cp_config.h"
#include "cp_stats.h"
#include "monitor_config.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "cp.h"
#include <rte_log.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "assert.h"


//#define RTE_LOGTYPE_CP RTE_LOGTYPE_USER4

#define GLOBAL_ENTRIES			"GLOBAL"
#define APN_ENTRIES				"APN_CONFIG"
#define NAMESERVER_ENTRIES		"NAMESERVER_CONFIG"
#define IP_POOL_ENTRIES			"IP_POOL_CONFIG"
#define CACHE_ENTRIES			"CACHE"
#define APP_ENTRIES				"APP"
#define OPS_ENTRIES				"OPS"

#define CP_TYPE					"CP_TYPE"
#define CP_LOGGER				"CP_LOGGER"
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

extern char* config_update_base_folder; 
extern bool native_config_folder;

const char *primary_dns = "8.8.8.8";
const char *secondary_dns = "8.8.8.4";	

void
config_cp_ip_port(pfcp_config_t *pfcp_config)
{
	int32_t i = 0;
	int32_t num_ops_entries = 0;
	int32_t num_app_entries = 0;
	int32_t num_cache_entries = 0;
	int32_t num_ip_pool_entries = 0;
	int32_t num_apn_entries = 0;
	int32_t num_global_entries = 0;

	struct rte_cfgfile_entry *global_entries = NULL;
	struct rte_cfgfile_entry *apn_entries = NULL;
	struct rte_cfgfile_entry *ip_pool_entries = NULL;
	struct rte_cfgfile_entry *cache_entries = NULL;
	struct rte_cfgfile_entry *app_entries = NULL;
	struct rte_cfgfile_entry *ops_entries = NULL;


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

		}else if (strncmp(S11_IPS, global_entries[i].name,
					strlen(S11_IPS)) == 0) {

			/* TODO - ajay get rid of ip address. Get hostname/servicename  if required */
			/* TODO - ajay mme address is not required to be configured at SPGW */
			inet_aton(global_entries[i].value,
					&(pfcp_config->s11_ip));

			fprintf(stderr, "CP: S11_IP      : %s\n",
					inet_ntoa(pfcp_config->s11_ip));

		}else if (strncmp(S11_PORTS, global_entries[i].name,
					strlen(S11_PORTS)) == 0) {

			pfcp_config->s11_port =
					(uint16_t)atoi(global_entries[i].value);

			fprintf(stderr, "CP: S11_PORT    : %d\n",
					pfcp_config->s11_port);

		} else if (strncmp(S5S8_IPS, global_entries[i].name,
					strlen(S5S8_IPS)) == 0) {

			inet_aton(global_entries[i].value,
					&(pfcp_config->s5s8_ip));

			fprintf(stderr, "CP: S5S8_IP     : %s\n",
					inet_ntoa(pfcp_config->s5s8_ip));

		} else if (strncmp(S5S8_PORTS, global_entries[i].name,
					strlen(S5S8_PORTS)) == 0) {

			pfcp_config->s5s8_port =
				(uint16_t)atoi(global_entries[i].value);

			fprintf(stderr, "CP: S5S8_PORT   : %d\n",
					pfcp_config->s5s8_port);

		} else if (strncmp(PFCP_IPS , global_entries[i].name,
					strlen(PFCP_IPS)) == 0) {

			inet_aton(global_entries[i].value,
					&(pfcp_config->pfcp_ip));

			fprintf(stderr, "CP: PFCP_IP     : %s\n",
					inet_ntoa(pfcp_config->pfcp_ip));

		} else if (strncmp(PFCP_PORTS, global_entries[i].name,
					strlen(PFCP_PORTS)) == 0) {

			pfcp_config->pfcp_port =
				(uint16_t)atoi(global_entries[i].value);

			fprintf(stderr, "CP: PFCP_PORT   : %d\n",
					pfcp_config->pfcp_port);

		} else if (strncmp(MME_S11_IPS, global_entries[i].name,
					strlen(MME_S11_IPS)) == 0) {

			inet_aton(global_entries[i].value,
					&(pfcp_config->s11_mme_ip));

			fprintf(stderr, "CP: MME_S11_IP  : %s\n",
					inet_ntoa(pfcp_config->s11_mme_ip));

		} else if (strncmp(MME_S11_PORTS, global_entries[i].name,
					strlen(MME_S11_PORTS)) == 0) {
			pfcp_config->s11_mme_port =
				(uint16_t)atoi(global_entries[i].value);

			fprintf(stderr, "CP: MME_S11_PORT: %d\n", pfcp_config->s11_mme_port);

		} else if (strncmp(UPF_PFCP_IPS , global_entries[i].name,
					strlen(UPF_PFCP_IPS)) == 0) {

			/* ajay - this should be part of static upf config list */
			inet_aton(global_entries[i].value,
					&(pfcp_config->upf_pfcp_ip));

			fprintf(stderr, "CP: UPF_PFCP_IP : %s\n",
					inet_ntoa(pfcp_config->upf_pfcp_ip));

		} else if (strncmp(UPF_PFCP_PORTS, global_entries[i].name,
					strlen(UPF_PFCP_PORTS)) == 0) {

			pfcp_config->upf_pfcp_port =
				(uint16_t)atoi(global_entries[i].value);

			fprintf(stderr, "CP: UPF_PFCP_PORT: %d\n",
					pfcp_config->upf_pfcp_port);

		 } else if (strncmp(CP_LOGGER, global_entries[i].name, strlen(CP_LOGGER)) == 0) {
			 cp_config->cp_logger = (uint8_t)atoi(global_entries[i].value);
			 fprintf(stderr, "CP: CP_LOGGER: %d\n",
					cp_config->cp_logger);
		 }

		/* Parse timer and counter values from cp.cfg */
		if(strncmp(TRANSMIT_TIMER, global_entries[i].name, strlen(TRANSMIT_TIMER)) == 0) {
			pfcp_config->transmit_timer = (int)atoi(global_entries[i].value);
			fprintf(stderr, "CP: TRANSMIT_TIMER: %d\n",
				pfcp_config->transmit_timer);
		}

		if(strncmp(PERIODIC_TIMER, global_entries[i].name, strlen(PERIODIC_TIMER)) == 0) {
			pfcp_config->periodic_timer = (int)atoi(global_entries[i].value);
			fprintf(stderr, "CP: PERIODIC_TIMER: %d\n",
				pfcp_config->periodic_timer);
		}

		if(strncmp(TRANSMIT_COUNT, global_entries[i].name, strlen(TRANSMIT_COUNT)) == 0) {
			pfcp_config->transmit_cnt = (uint8_t)atoi(global_entries[i].value);
			fprintf(stderr, "CP: TRANSMIT_COUNT: %u\n",
				pfcp_config->transmit_cnt);
		}

		/* Parse CP Timer Request Time Out and Retries Values from cp.cfg */
		if(strncmp(REQUEST_TIMEOUT, global_entries[i].name, strlen(REQUEST_TIMEOUT)) == 0){
			if(check_cp_req_timeout_config(global_entries[i].value) == 0) {
				pfcp_config->request_timeout = (int)atoi(global_entries[i].value);
				fprintf(stderr, "CP: REQUEST_TIMEOUT: %d\n",
					pfcp_config->request_timeout);
			} else {
				rte_panic("Error configuring "
					"CP TIMER "REQUEST_TIMEOUT" invalid entry of %s\n", STATIC_CP_FILE);
			}
		}else {
			/* if CP Request Timer Parameter is not present is cp.cfg */
			/* Defualt Request Timerout value */
			/* 5 minute = 300000 milisecond  */
			if(pfcp_config->request_timeout == 0) {
				pfcp_config->request_timeout = 300000;
				fprintf(stderr, "CP: DEFAULT REQUEST_TIMEOUT: %d\n",
					pfcp_config->request_timeout);
			}
		}

		if(strncmp(REQUEST_TRIES, global_entries[i].name, strlen(REQUEST_TRIES)) == 0) {
			if(check_cp_req_tries_config(global_entries[i].value) == 0) {
				pfcp_config->request_tries = (uint8_t)atoi(global_entries[i].value);
				fprintf(stderr, "CP: REQUEST_TRIES: %d\n",
					pfcp_config->request_tries);
			} else {
				rte_panic("Error configuring "
					"CP TIMER "REQUEST_TRIES" invalid entry of %s\n", STATIC_CP_FILE);
			}

		} else {
			/* if CP Request Timer Parameter is not present is cp.cfg */
                        /* Defualt Request Retries value */
			if(pfcp_config->request_tries == 0) {
				pfcp_config->request_tries = 3;
				fprintf(stderr, "CP: DEFAULT REQUEST_TRIES: %d\n",
					pfcp_config->request_tries);
			}
		}
	}

	rte_free(global_entries);

	/* Parse APN and nameserver values. */
	uint16_t apn_idx = 0;
	uint16_t app_nameserver_ip_idx = 0;
	uint16_t ops_nameserver_ip_idx = 0;

	num_apn_entries =
		rte_cfgfile_section_num_entries(file, APN_ENTRIES);

	if (num_apn_entries > 0) {
		/* Allocate the memory. */
		apn_entries = rte_malloc_socket(NULL,
				sizeof(struct rte_cfgfile_entry) *
				num_apn_entries,
				RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (apn_entries == NULL)
		rte_panic("Error configuring"
				"apn entry of %s\n", STATIC_CP_FILE);
	}


	/* Fill the entries in APN list. */
	rte_cfgfile_section_entries(file,
			APN_ENTRIES, apn_entries, num_apn_entries);

	for (i = 0; i < num_apn_entries; ++i) {
		fprintf(stderr, "CP: [%s] = %s\n",
				apn_entries[i].name,
				apn_entries[i].value);

		if (strncmp(APN, apn_entries[i].name,
					strlen(APN)) == 0) {
			/* If key matches */
			if (i < MAX_NUM_APN) {
				char *ptr[3];
				/* Based on default value, set usage type */
				apn_list[i].apn_usage_type = -1;

				parse_apn_args(apn_entries[i].value, ptr);

				apn_list[i].apn_name_label = ptr[0];

			if (ptr[1] != NULL)
				apn_list[i].apn_usage_type = atoi(ptr[1]);

		if (ptr[2] != NULL)
			memcpy(apn_list[i].apn_net_cap, ptr[2], strlen(ptr[2]));

			set_apn_name(&apn_list[i], apn_list[i].apn_name_label);

				int f = 0;
				/* Free the memory allocated by malloc. */

				for (f = 0; f < 3; f++)
					free(ptr[f]);

				apn_idx++;
			}
		}
	}
	rte_free(apn_entries);


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
			pfcp_config->dns_cache.concurrent =
					(uint32_t)atoi(cache_entries[i].value);
		if (strncmp(PERCENTAGE, cache_entries[i].name,
						strlen(CONCURRENT)) == 0)
			pfcp_config->dns_cache.percent =
					(uint32_t)atoi(cache_entries[i].value);
		if (strncmp(INT_SEC, cache_entries[i].name,
						strlen(CONCURRENT)) == 0)
			pfcp_config->dns_cache.sec =
					(((uint32_t)atoi(cache_entries[i].value)) * 1000);
		if (strncmp(QUERY_TIMEOUT, cache_entries[i].name,
		                strlen(QUERY_TIMEOUT)) == 0)
		    pfcp_config->dns_cache.timeoutms =
		            (long)atol(cache_entries[i].value);
		if (strncmp(QUERY_TRIES, cache_entries[i].name,
		                strlen(QUERY_TRIES)) == 0)
		    pfcp_config->dns_cache.tries =
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
			pfcp_config->app_dns.freq_sec =
					(uint8_t)atoi(app_entries[i].value);

		if (strncmp(FILENAME, app_entries[i].name,
						strlen(FILENAME)) == 0)
			strncpy(pfcp_config->app_dns.filename,
					app_entries[i].value,
					strlen(app_entries[i].value));

		if (strncmp(NAMESERVER, app_entries[i].name,
						strlen(NAMESERVER)) == 0) {
			strncpy(pfcp_config->app_dns.nameserver_ip[app_nameserver_ip_idx],
					app_entries[i].value,
					strlen(app_entries[i].value));
			app_nameserver_ip_idx++;
		}
	}

	pfcp_config->app_dns.nameserver_cnt = app_nameserver_ip_idx;

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
			pfcp_config->ops_dns.freq_sec =
					(uint8_t)atoi(ops_entries[i].value);

		if (strncmp(FILENAME, ops_entries[i].name,
						strlen(FILENAME)) == 0)
			strncpy(pfcp_config->ops_dns.filename,
					ops_entries[i].value,
					strlen(ops_entries[i].value));

		if (strncmp(NAMESERVER, ops_entries[i].name,
						strlen(NAMESERVER)) == 0) {
			strncpy(pfcp_config->ops_dns.nameserver_ip[ops_nameserver_ip_idx],
					ops_entries[i].value,
					strlen(ops_entries[i].value));
			ops_nameserver_ip_idx++;
		}
	}

	pfcp_config->ops_dns.nameserver_cnt = ops_nameserver_ip_idx;

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

	return;
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
check_cp_req_tries_config(char *value) {
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
	printf("%s %d \n",__FUNCTION__,__LINE__);
	RTE_LOG_DP(INFO, CP, "Received %s. File %s flags: %x\n",
		   __FUNCTION__, config_file, flags);

	if (native_config_folder == false) {
		/* Move the updated config to standard path */
		static char cmd[256];
		sprintf(cmd, "cp %s %s", config_file, CP_CONFIG_OPT_PATH);
		int ret = system(cmd);
		RTE_LOG_DP(INFO, CP, "system call return value: %d \n", ret);
	}
 
	/* We dont expect quick updates from configmap..One update per interval. Typically 
	 * worst case 60 seconds for 1 config update. Updates are clubbed and dont come frequent 
	 * We re-register to avoid recursive callbacks 
	 */
	watch_config_change(config_file, config_change_cbk);

	/* Lets first parse the current app_config.cfg file  */
	struct app_config *new_cfg;
	new_cfg = (struct app_config *) calloc(1, sizeof(struct app_config));
	if (new_cfg == NULL) {
		rte_exit(EXIT_FAILURE, "Failed to allocate memory for new_cfg!\n");
	}

	init_spgwc_dynamic_config(new_cfg);
	/* Now compare whats changed and update our global application config */

	/* Data plane Selection rules config modification
	 *  Delete - Should not be removed. If removed then it will not delete the existing 
	 *           subscribers associated with that dataplane 
	 * 		    TODO : delete subscribers if DP going away 
	 *  Add - New Rules can be added for existing dataplane or new rules for new dataplane 
	 *        can be added at any time
	 *  Modify Rules : Modifying existing rules may not have any impact on existing subscribers
	 * 
	 *  Correct way to remove any DP would be to make sure all the subscribers associated with 
	 *  that DP are deleted first and then DP is removed. 
	 *  
	 * For now I am just going to switch to new config. Anyway its just selection of DPs 
	 */
	struct app_config *old_config = cp_config->appl_config;
    /* Copy resource pointers from old config to new config */
	struct dp_info *dpNew; 
	struct dp_info *dpOld; 
	LIST_FOREACH(dpNew, &new_cfg->dpList, dpentries) {
		LIST_FOREACH(dpOld, &old_config->dpList, dpentries) {
			if(dpOld->dpId == dpNew->dpId) {
                		dpNew->s1u_sgw_ip = dpOld->s1u_sgw_ip;
				dpNew->upf = dpOld->upf;
				break;
			}
		}	
	}

	cp_config->appl_config = new_cfg; /* switch to new config */ 
	struct dp_info *np; 
	np = LIST_FIRST(&old_config->dpList);
	while (np != NULL) {
		LIST_REMOVE(np, dpentries);
		free(np);
		np = LIST_FIRST(&old_config->dpList);
	}
	free(old_config);

	/* Everytime we add new config we need to add code here. How to react to config change  */
}

void 
register_config_updates(char *file)
{
	/* I would prefer a complete path than this relative path.
	 * Looks like it may break */
	watch_config_change(file, config_change_cbk);
}

void 
init_spgwc_dynamic_config(struct app_config *cfg )
{
	// Read the config file, parse it and fill passed cfg data structure 
	const char *entry = NULL;
	unsigned int num_dp_selection_rules = 0;
	unsigned int index;
	LIST_INIT(&cfg->dpList);

	struct rte_cfgfile *file = rte_cfgfile_load(APP_CONFIG_FILE, 0);
	if (NULL == file) {
		RTE_LOG_DP(ERR, CP, "App config file is missing, ignore error...\n");
		return;
	}

	entry = rte_cfgfile_get_entry(file, "GLOBAL", "DNS_PRIMARY");
	if (entry == NULL) {
		RTE_LOG_DP(INFO, CP, "DNS_PRIMARY default config is missing. \n");
		entry = primary_dns;
	}
	if (inet_aton(entry, &cfg->dns_p) == 1) {
		set_app_dns_primary(cfg);
		RTE_LOG_DP(INFO, CP, "Global DNS_PRIMARY address is %s \n", inet_ntoa(cfg->dns_p));
	} else {
		// invalid address 
		RTE_LOG_DP(ERR, CP, "Global DNS_PRIMARY address is invalid %s \n", entry);
	}

	entry = rte_cfgfile_get_entry(file, "GLOBAL", "DNS_SECONDARY");
	if (entry == NULL) {
		RTE_LOG_DP(INFO, CP, "DNS_SECONDARY default config is missing. \n");
		entry = secondary_dns;
	}
	if(inet_aton(entry, &cfg->dns_s) == 1) {
		set_app_dns_secondary(cfg);
		RTE_LOG_DP(INFO, CP, "Global DNS_SECONDARY address is %s \n", inet_ntoa(cfg->dns_s));
	} else {
		// invalid address 
		RTE_LOG_DP(ERR, CP, "Global DNS_SECONDARY address is invalid %s \n", entry);
	}
	uint16_t ip_mtu = DEFAULT_IPV4_MTU;
	entry = rte_cfgfile_get_entry(file, "GLOBAL", "IPV4_MTU");
	if (entry == NULL) {
		RTE_LOG_DP(INFO, CP, "Global DP IP_MTU default global config is missing. Use default %d  \n",DEFAULT_IPV4_MTU);
	} else {
		ip_mtu = atoi(entry);
		RTE_LOG_DP(INFO, CP, "Global DP IP_MTU set to  %d  \n",ip_mtu);
	}

	entry = rte_cfgfile_get_entry(file, "GLOBAL", "NUM_DP_SELECTION_RULES");
	if (entry == NULL) {
       		RTE_LOG_DP(ERR, CP, "NUM_DP_SELECTION_RULES missing from app_config.cfg file, abort parsing\n");
       		return;
	}
   	RTE_LOG_DP(ERR, CP, "NUM_DP_SELECTION_RULES %s \n", entry);
	num_dp_selection_rules = atoi(entry);

	for (index = 0; index < num_dp_selection_rules; index++) {
		static char sectionname[64] = {0};
		struct dp_info *dpInfo = NULL;
		dpInfo = (struct dp_info *)calloc(1, sizeof(struct dp_info));

		if (dpInfo == NULL) {
			RTE_LOG_DP(ERR, CP, "Could not allocate memory for dpInfo!\n");
			return;
		}
		snprintf(sectionname, sizeof(sectionname),
			 "DP_SELECTION_RULE_%u", index + 1);
		entry = rte_cfgfile_get_entry(file, sectionname, "DPID");
		if (entry) {
			dpInfo->dpId = atoi(entry);
		} else {
			RTE_LOG_DP(ERR, CP, "DPID not found in the configuration file\n");
		}

		entry = rte_cfgfile_get_entry(file, sectionname, "DPNAME");
		if (entry) {
			strncpy(dpInfo->dpName, entry, DP_SITE_NAME_MAX);
		} else {
			RTE_LOG_DP(ERR, CP, "DPNAME not found in the configuration file\n");
		}
		RTE_LOG_DP(ERR, CP, "DPNAME %s configured \n", dpInfo->dpName);

		struct dp_info *dpOld = NULL;
		LIST_FOREACH(dpOld, &cp_config->appl_config->dpList, dpentries) {
			if ((dpOld->dpId == dpInfo->dpId)) {
				break;
			}
		}
 

		entry = rte_cfgfile_get_entry(file, sectionname, "MCC");
		if (entry) {
			// TODO : handle 2 digit mcc, mnc
			RTE_LOG_DP(ERR, CP, "MCC length %lu found in the configuration file\n", strlen(entry));
			dpInfo->key.mcc_mnc.mcc_digit_1 = (unsigned char )entry[0];
			dpInfo->key.mcc_mnc.mcc_digit_2 = (unsigned char )entry[1];
			dpInfo->key.mcc_mnc.mcc_digit_3 = (unsigned char )entry[2];
			RTE_LOG_DP(ERR, CP, "MCC %d %d %d \n", dpInfo->key.mcc_mnc.mcc_digit_1, dpInfo->key.mcc_mnc.mcc_digit_2, dpInfo->key.mcc_mnc.mcc_digit_3);
		} else {
			RTE_LOG_DP(ERR, CP, "MCC not found in the configuration file\n");
		}

		entry = rte_cfgfile_get_entry(file, sectionname, "MNC");
		if (entry) {
			dpInfo->key.mcc_mnc.mnc_digit_1 = (unsigned char )entry[0];
			dpInfo->key.mcc_mnc.mnc_digit_2 = (unsigned char )entry[1];
			if(strlen(entry) == 2) {
			  dpInfo->key.mcc_mnc.mnc_digit_3 = (unsigned char )0xf;
			} else {
			  dpInfo->key.mcc_mnc.mnc_digit_3 = (unsigned char )entry[2];
			}
			RTE_LOG_DP(ERR, CP, "MNC length %lu found in the configuration file\n", strlen(entry));
			RTE_LOG_DP(ERR, CP, "MNC %d %d %d \n", dpInfo->key.mcc_mnc.mnc_digit_1, dpInfo->key.mcc_mnc.mnc_digit_2, dpInfo->key.mcc_mnc.mnc_digit_3);
		} else {
			RTE_LOG_DP(ERR, CP, "MNC not found in the configuration file\n");
		}

		entry = rte_cfgfile_get_entry(file, sectionname, "TAC");
		if (entry) {
			dpInfo->key.tac = atoi(entry);
		} else {
			RTE_LOG_DP(ERR, CP, "TAC not found in the configuration file\n");
		}
		LIST_INSERT_HEAD(&cfg->dpList, dpInfo, dpentries);

		entry = rte_cfgfile_get_entry(file, sectionname , "DNS_PRIMARY");
		if (entry == NULL) {
			RTE_LOG_DP(INFO, CP, "DP(%s) DNS_PRIMARY default config is missing. \n", dpInfo->dpName);
			entry = primary_dns;
		}

		if (inet_aton(entry, &dpInfo->dns_p) == 1) {
			set_dp_dns_primary(dpInfo);
			RTE_LOG_DP(INFO, CP, "DP(%s) DNS_PRIMARY address is %s \n", dpInfo->dpName, inet_ntoa(dpInfo->dns_p));
		} else {
			//invalid address
			RTE_LOG_DP(ERR, CP, "DP (%s) DNS_PRIMARY address is invalid %s \n",dpInfo->dpName, entry);
		}

		entry = rte_cfgfile_get_entry(file, sectionname , "DNS_SECONDARY");
		if (entry == NULL) {
			RTE_LOG_DP(INFO, CP, "DP(%s) DNS_SECONDARY default config is missing. \n",dpInfo->dpName);
			entry = secondary_dns;
		}
		if (inet_aton(entry, &dpInfo->dns_s) == 1) {
			set_dp_dns_secondary(dpInfo);
			RTE_LOG_DP(INFO, CP, "DP(%s) DNS_SECONDARY address is %s \n", dpInfo->dpName, inet_ntoa(dpInfo->dns_s));
		} else {
			//invalid address
			RTE_LOG_DP(ERR, CP, "DP(%s) DNS_SECONDARY address is invalid %s \n",dpInfo->dpName, entry);
		}

        entry = rte_cfgfile_get_entry(file, sectionname , "IPV4_MTU");
        if (entry == NULL) {
                RTE_LOG_DP(INFO, CP, "DP(%s) IP_MTU default config is missing.  Use  %d  \n",dpInfo->dpName, ip_mtu);
                dpInfo->ip_mtu = ip_mtu;
        } else {
                dpInfo->ip_mtu = atoi(entry);
                RTE_LOG_DP(INFO, CP, "DP(%s) IP_MTU set to  %d \n",dpInfo->dpName, dpInfo->ip_mtu);
        }
 
		bool static_pool_config_change = false;
		bool first_time_pool_config = false;
		entry = rte_cfgfile_get_entry(file, sectionname, "STATIC_IP_POOL");
		if(dpOld != NULL) {
			if(entry == NULL) { 
				if(dpOld->static_pool == NULL) {
					//No old config, no new config.. 
					RTE_LOG_DP(INFO, CP, "DP(%s) STATIC_IP_POOL is not configured \n", dpInfo->dpName);
				} else if (dpOld->static_pool != NULL) {
					// No new config but old config exist 
					static_pool_config_change = true;
					RTE_LOG_DP(ERR, CP, "DP(%s) STATIC_IP_POOL config removal not supported. Old config will be used = %s \n", dpInfo->dpName, dpOld->static_pool);
				}
			} else if (entry != NULL) {  
				if(dpOld->static_pool == NULL) {
					first_time_pool_config = true;
				} else if (dpOld->static_pool != NULL) { 
					if(strcmp(dpOld->static_pool, entry) != 0) {
						static_pool_config_change = true;
						RTE_LOG_DP(ERR, CP, "DP(%s) STATIC_IP_POOL config modification not supported. Old config(%s) New Config (%s). Continue to use old config \n",dpInfo->dpName, dpOld->static_pool, entry);
					} else {
						//no change in the pool config  
						RTE_LOG_DP(INFO, CP, "DP(%s) STATIC_IP_POOL configuration not changed %s \n",dpInfo->dpName, entry);
					}
				}
			}
			//Lets take old static config to new as is 
			dpInfo->static_pool_tree = dpOld->static_pool_tree; // pointer copy 
			dpInfo->static_pool = dpOld->static_pool; // pointer copy
		} else if(entry != NULL){
			first_time_pool_config = true;
			RTE_LOG_DP(INFO, CP, "DP(%s) STATIC_IP_POOL configured  %s \n",dpInfo->dpName, entry);
		}

		if(first_time_pool_config == true && static_pool_config_change == false) {
			// first time edge configuration 
			dpInfo->static_pool = NULL; 
			char *pool_string = parse_create_static_ip_pool(&dpInfo->static_pool_tree, entry);
			if (pool_string != NULL) {
				dpInfo->static_pool = pool_string; 
			} 
			RTE_LOG_DP(INFO, CP, "DP(%s) STATIC_IP_POOL %s initialized  \n", dpInfo->dpName, dpInfo->static_pool);
		}
 	}
	return;
}

/* AJAY : for now I am using linux call to do the dns resolution...
 * Need to use DNS lib from epc tools */
static
struct in_addr native_linux_name_resolve(const char *name)
{
        printf("Function [%s] - Line - %d \n",__FUNCTION__,__LINE__);
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
        assert(0); /* temporary */
	struct in_addr ip = {0};
        return ip;
}

struct in_addr 
get_upf_ipaddr_for_key(struct dp_key *key)
{
	struct in_addr ip = {0};
#if 0
	RTE_LOG_DP(INFO, CP, "Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", key->mcc_mnc.mcc_digit_1,
		   key->mcc_mnc.mcc_digit_2, key->mcc_mnc.mcc_digit_3, key->mcc_mnc.mnc_digit_1,
		   key->mcc_mnc.mnc_digit_2, key->mcc_mnc.mnc_digit_3, key->tac);
#else
	printf("Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", key->mcc_mnc.mcc_digit_1,
		   key->mcc_mnc.mcc_digit_2, key->mcc_mnc.mcc_digit_3, key->mcc_mnc.mnc_digit_1,
		   key->mcc_mnc.mnc_digit_2, key->mcc_mnc.mnc_digit_3, key->tac);
#endif

	struct dp_info *np; // ajaytodo - add upf address 
	LIST_FOREACH(np, &cp_config->appl_config->dpList, dpentries) {
#if 0
	RTE_LOG_DP(INFO, CP, "dp Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", np->key.mcc_mnc.mcc_digit_1,
		   np->key.mcc_mnc.mcc_digit_2, np->key.mcc_mnc.mcc_digit_3, np->key.mcc_mnc.mnc_digit_1,
		   np->key.mcc_mnc.mnc_digit_2, np->key.mcc_mnc.mnc_digit_3, np->key.tac);
#else
	printf("dp Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", np->key.mcc_mnc.mcc_digit_1,
		   np->key.mcc_mnc.mcc_digit_2, np->key.mcc_mnc.mcc_digit_3, np->key.mcc_mnc.mnc_digit_1,
		   np->key.mcc_mnc.mnc_digit_2, np->key.mcc_mnc.mnc_digit_3, np->key.tac);
#endif
		if(bcmp((void *)(&np->key.mcc_mnc), (void *)(&key->mcc_mnc), 3) != 0)
			continue;
		if(np->key.tac != key->tac)
			continue;
		return native_linux_name_resolve(np->dpName); 
	}
	return ip; 
}

/* Given key find the DP. Once DP is found then return its dpId */
uint32_t
select_dp_for_key(struct dp_key *key)
{
#if 0
	RTE_LOG_DP(INFO, CP, "Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", key->mcc_mnc.mcc_digit_1,
		   key->mcc_mnc.mcc_digit_2, key->mcc_mnc.mcc_digit_3, key->mcc_mnc.mnc_digit_1,
		   key->mcc_mnc.mnc_digit_2, key->mcc_mnc.mnc_digit_3, key->tac);
#else
	printf("Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", key->mcc_mnc.mcc_digit_1,
		   key->mcc_mnc.mcc_digit_2, key->mcc_mnc.mcc_digit_3, key->mcc_mnc.mnc_digit_1,
		   key->mcc_mnc.mnc_digit_2, key->mcc_mnc.mnc_digit_3, key->tac);
#endif

	struct dp_info *np; // ajaytodo - add upf address 
	LIST_FOREACH(np, &cp_config->appl_config->dpList, dpentries) {
#if 0
	RTE_LOG_DP(INFO, CP, "dp Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", np->key.mcc_mnc.mcc_digit_1,
		   np->key.mcc_mnc.mcc_digit_2, np->key.mcc_mnc.mcc_digit_3, np->key.mcc_mnc.mnc_digit_1,
		   np->key.mcc_mnc.mnc_digit_2, np->key.mcc_mnc.mnc_digit_3, np->key.tac);
#else
	printf("dp Key - MCC = %d%d%d MNC %d%d%d TAC = %d\n", np->key.mcc_mnc.mcc_digit_1,
		   np->key.mcc_mnc.mcc_digit_2, np->key.mcc_mnc.mcc_digit_3, np->key.mcc_mnc.mnc_digit_1,
		   np->key.mcc_mnc.mnc_digit_2, np->key.mcc_mnc.mnc_digit_3, np->key.tac);
#endif
		if(bcmp((void *)(&np->key.mcc_mnc), (void *)(&key->mcc_mnc), 3) != 0)
			continue;
		if(np->key.tac != key->tac)
			continue;
		return np->dpId;
	}
	return DPN_ID; /* 0 is invalid DP */ 
}

uint8_t
resolve_upf_context_to_dpInfo(struct cfg_upf_context *upf, char *hostname, struct in_addr s1u_sgw_ip)
{
	struct dp_info *dp;
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if (!strcmp(hostname, dp->dpName)) {
			dp->upf = upf;
			dp->s1u_sgw_ip = s1u_sgw_ip;
			upf->dpId = dp->dpId;
			return 1;
		}
	}
	return 0;
}

struct in_addr
fetch_s1u_sgw_ip(uint32_t dpId)
{
	struct dp_info *dp;
	struct in_addr a = { .s_addr = 0 };
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if (dpId == dp->dpId) {
			return dp->s1u_sgw_ip;
		}
	}

	rte_panic("Could not find s1u ip address for dpid: %u\n", dpId);
	rte_exit(EXIT_FAILURE, "Could not find s1u ip address for dpid: %u\n", dpId);
	/* control should never reach here */
	RTE_SET_USED(a);
	return a;
}

struct dp_info *
fetch_dp_context(uint32_t dpId)
{
	struct dp_info *dp;
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if (dpId == dp->dpId) {
			return dp;
		}
	}
	rte_panic("Could not find DP for dpid: %u\n", dpId);
	/* control should never reach here */
	return NULL;
}

struct cfg_upf_context *
fetch_upf_context(uint32_t dpId)
{
	struct dp_info *dp;
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if (dpId == dp->dpId) {
			return dp->upf;
		}
	}

	rte_panic("Could not find upf_context for dpid: %u\n", dpId);
	/* control should never reach here */
	return NULL;
}

struct in_addr
fetch_dns_primary_ip(uint32_t dpId, bool *present)
{
	struct dp_info *dp;
	struct in_addr dns_p = { .s_addr = 0 };
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if ((dpId == dp->dpId) && (dp->flags & CONFIG_DNS_PRIMARY)) {
			*present = true;
			return dp->dns_p;
		}
	}
	*present = get_app_primary_dns(cp_config->appl_config, &dns_p);
	return dns_p;
}

struct in_addr
fetch_dns_secondary_ip(uint32_t dpId, bool *present)
{
	struct dp_info *dp;
	struct in_addr dns_s = { .s_addr = 0 };
	LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
		if ((dpId == dp->dpId) && (dp->flags & CONFIG_DNS_SECONDARY)) {
			*present = true;
			return dp->dns_s;
		}
	}
	*present = get_app_secondary_dns(cp_config->appl_config, &dns_s);
	return dns_s;
}

uint16_t
fetch_dp_ip_mtu(uint32_t dpId)
{
       struct dp_info *dp;
       LIST_FOREACH(dp, &cp_config->appl_config->dpList, dpentries) {
               if ((dpId == dp->dpId)) {
                       return dp->ip_mtu;
               }
       }
       return DEFAULT_IPV4_MTU; /* Lets not crash. Return default */
}


/* Parse the entry and create IP pool tree */
char*
parse_create_static_ip_pool(struct ip_table **addr_pool, const char *entry)
{
	char err_string[128];
	char *pool=NULL;
	*addr_pool = NULL;

	do {
		pool= (char *)calloc(1, 128); 

		if (pool == NULL) {
      			sprintf(err_string, " Memory allocation failed ");
			break;
		}
		strcpy(pool, entry); 
		RTE_LOG_DP(ERR, CP, "STATIC_IP_POOL %s parsing started \n", pool);

		const char token[2] = "/";
		char *network_str = strtok(pool, token);
		if (network_str == NULL) {
			sprintf(err_string, " STATIC_IP_POOL in bad format. It should be in a.b.c.d/mask format ");
			free(pool);
			break;
		}
		RTE_LOG_DP(ERR, CP, "STATIC_IP_POOL Network %s \n", network_str);

		struct in_addr network;
		if (inet_aton(network_str, &network) == 0) {
			sprintf(err_string, " Network %s in bad format ",  network_str);
			free(pool);
			break;
		}
		network.s_addr = ntohl(network.s_addr); // host order

		char *mask_str = strtok(NULL, token);
		if (mask_str == NULL) {
			sprintf(err_string, ". No mask configured ");
			free(pool);
			break;
		}

		uint32_t mask;
		mask = atoi(mask_str);
		if (mask > 23 && mask <=32 ) {
			; // ajay: todo *addr_pool = create_ue_pool(network, mask);
		} else {
			sprintf(err_string, " Bad Mask. Mask should be from /24 to /32 only - Its %u ", mask);
			free(pool);
			break;
		}
		RTE_LOG_DP(ERR, CP, "STATIC_IP_POOL %s configured successfully \n", pool);
		strcpy(pool, entry); /* recopy entry into pool. we need pool in a.b.c.d/mask format  */ 
		return pool;
	} while (0);
	RTE_LOG_DP(ERR, CP, "STATIC_IP_POOL %s Parsing failed. Error - %s  \n", entry, err_string);
	return NULL;
}

/*
 * Set flag that Primary DNS config is available at Edge Site level. This should be called 
 * when primary DNS address is set in the PGW app level 
 */
void set_dp_dns_primary(struct dp_info *dp) 
{
  dp->flags |= CONFIG_DNS_PRIMARY;
}

/*
 * Set flag that secondary DNS config is available at Edge Site level. This should be called 
 * when secondary DNS address is set in the PGW app level 
 */
void set_dp_dns_secondary(struct dp_info *dp) 
{
  dp->flags |= CONFIG_DNS_SECONDARY;
}

/*
 * Set flag that primary DNS config is available at App level. This should be called 
 * when primary DNS address is set in the PGW app level 
 */

void set_app_dns_primary(struct app_config *app) 
{
	app->flags |= CONFIG_DNS_PRIMARY;
}

/*
 *  Get primary DNS address if available in dns_p and return true.
 *  In case address is not available then return false
 */

bool get_app_primary_dns(struct app_config *app, struct in_addr *dns_p)
{
	if (app->flags & CONFIG_DNS_PRIMARY) {
		*dns_p = app->dns_p;
		return true;
	}
	return false;
}

/*
 * Set flag that secondary DNS config is available at App level. This should be called 
 * when secondary DNS address is set in the PGW app level 
 */

void set_app_dns_secondary(struct app_config *app) 
{
	app->flags |= CONFIG_DNS_SECONDARY;
}

/*
 *  Get secondary DNS address if available in dns_p and return true.
 *  In case address is not available then return false
 */

bool get_app_secondary_dns(struct app_config *app, struct in_addr *dns_s)
{
	if (app->flags & CONFIG_DNS_SECONDARY) {
		*dns_s = app->dns_s;
		return true;
	}
	return false;
}

