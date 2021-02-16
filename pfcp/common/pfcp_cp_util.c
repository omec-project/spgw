// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <sys/time.h>
#include <rte_hash.h>
#include <rte_errno.h>
#include <rte_debug.h>
#include <rte_jhash.h>
#include <rte_lcore.h>
#include <rte_hash_crc.h>

#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_messages.h"
#include "cp_log.h"
#include "cp_config.h"
#include "cp_config_defs.h"
#include "cp_io_poll.h"
#include "tables/tables.h"
#include "util.h"
#include "pfcp_cp_association.h"
#include "cdnshelper.h"

#define FAILED_ENB_FILE "logs/failed_enb_queries.log"

#define QUERY_RESULT_COUNT 16


/**
 * @brief  : Add canonical result entry in upflist hash
 * @param  : res , result
 * @param  : res_count , total entries in result
 * @param  : imsi_val , imsi value
 * @param  : imsi_len , imsi length
 * @return : Returns upf count in case of success , 0 if could not get list , -1 otherwise
 */
static int
add_canonical_result_upflist_entry(canonical_result_t *res,
		uint8_t res_count, uint64_t *imsi_val, uint16_t imsi_len)
{
	upfs_dnsres_t *upf_list = (upfs_dnsres_t *)calloc(1, sizeof(upfs_dnsres_t));
	if (NULL == upf_list) {
		LOG_MSG(LOG_ERROR, "Failure to allocate memory for upf list "
				"structure: %s ", rte_strerror(rte_errno));
		return -1;
	}

	uint8_t upf_count = 0;

	for (int i = 0; i < res_count; i++) {
		for (int j = 0; j < res[i].host2_info.ipv4host_count; j++) {
			inet_aton(res[i].host2_info.ipv4_hosts[j],
					&upf_list->upf_ip[upf_count]);
			memcpy(upf_list->upf_fqdn[upf_count], res[i].cano_name2,
					strlen((char *)res[i].cano_name2));
			upf_count++;
		}
	}

	if (upf_count == 0) {
		LOG_MSG(LOG_ERROR, "Could not get collocated candidate list. ");
		return 0;
	}

	upf_list->upf_count = upf_count;

	upflist_by_ue_hash_entry_add(imsi_val, imsi_len, upf_list);

	return upf_count;
}

/**
 * @brief  : Add dns result in upflist hash
 * @param  : res , dns result
 * @param  : res_count , total entries in result
 * @param  : imsi_val , imsi value
 * @param  : imsi_len , imsi length
 * @return : Returns upf count in case of success , 0 if could not get list , -1 otherwise
 */
static int
add_dns_result_upflist_entry(dns_query_result_t *res,
		uint8_t res_count, uint64_t *imsi_val, uint16_t imsi_len)
{
	upfs_dnsres_t *upf_list = (upfs_dnsres_t *)calloc(1, sizeof(upfs_dnsres_t));
	if (NULL == upf_list) {
		LOG_MSG(LOG_ERROR, "Failure to allocate memeory for upf list "
				"structure: %s ", rte_strerror(rte_errno));
		return -1;
	}

	uint8_t upf_count = 0;

	for (int i = 0; i < res_count; i++) {
		for (int j = 0; j < res[i].ipv4host_count; j++) {
			inet_aton(res[i].ipv4_hosts[j],
					&upf_list->upf_ip[upf_count]);
			memcpy(upf_list->upf_fqdn[upf_count], res[i].hostname,
					strlen(res[i].hostname));
			upf_count++;
		}
	}

	if (upf_count == 0) {
		LOG_MSG(LOG_ERROR, "Could not get SGW-U list using DNS query ");
		return 0;
	}

	upf_list->upf_count = upf_count;

	upflist_by_ue_hash_entry_add(imsi_val, imsi_len, upf_list);

	return upf_count;
}

/**
 * @brief  : Record entries for failed enodeb
 * @param  : endid , enodeb id
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
record_failed_enbid(char *enbid)
{
	FILE *fp = fopen(FAILED_ENB_FILE, "a");

	if (fp == NULL) {
		LOG_MSG(LOG_ERROR, "Could not open %s for writing failed "
				"eNodeB query entry.", FAILED_ENB_FILE);
		return 1;
	}

	fwrite(enbid, sizeof(char), strlen(enbid), fp);
	fwrite("\n", sizeof(char), 1, fp);
	fclose(fp);

	return 0;
}

int
get_upf_list(pdn_connection_t *pdn)
{
	int upf_count = 0;
	ue_context_t *ctxt = NULL;
	char apn_name[MAX_APN_LEN] = {0};

	/* VS: Retrive the UE context */
	ctxt = pdn->context;

	/* Get enodeb id, mnc, mcc from Create Session Request */
	uint32_t enbid = ctxt->uli.ecgi2.eci >> 8;
	char enodeb[11] = {0};
	char mnc[4] = {0};
	char mcc[4] = {0};

	sprintf(enodeb, "%u", enbid);

	if (ctxt->uli.ecgi2.ecgi_mnc_digit_3 == 15)
		sprintf(mnc, "%u%u", ctxt->uli.ecgi2.ecgi_mnc_digit_1,
			ctxt->uli.ecgi2.ecgi_mnc_digit_2);
	else
		sprintf(mnc, "%u%u%u", ctxt->uli.ecgi2.ecgi_mnc_digit_1,
			ctxt->uli.ecgi2.ecgi_mnc_digit_2,
			ctxt->uli.ecgi2.ecgi_mnc_digit_3);

	sprintf(mcc, "%u%u%u", ctxt->uli.ecgi2.ecgi_mcc_digit_1,
				ctxt->uli.ecgi2.ecgi_mcc_digit_2,
				ctxt->uli.ecgi2.ecgi_mcc_digit_3);

	if (!pdn->apn_in_use) {
		return 0;
	}

	/* Get network capabilities from apn configuration file */
	apn_profile_t *apn_requested = pdn->apn_in_use;

	//memcpy(apn_name,(char *)ctxt->apn.apn + 1, apn_requested->apn_name_length -1);
	/* VS: Need to revist this */
    // TODO : BUG - Handle any encoded apn 
	memcpy(apn_name, &(pdn->apn_in_use)->apn_name[1], (pdn->apn_in_use)->apn_name_length-1);

	if (cp_config->cp_type == SAEGWC || cp_config->cp_type == SGWC) {

		void *sgwupf_node_sel = init_enbupf_node_selector(enodeb, mnc, mcc);

		set_desired_proto(sgwupf_node_sel, ENBUPFNODESELECTOR, UPF_X_SXA);
		if(strlen(apn_requested->apn_net_cap) > 0) {
			set_nwcapability(sgwupf_node_sel, apn_requested->apn_net_cap);
		}

		if (apn_requested->apn_usage_type != -1) {
			set_ueusage_type(sgwupf_node_sel,
				apn_requested->apn_usage_type);
		}

		uint16_t sgwu_count = 0;
		dns_query_result_t sgwu_list[QUERY_RESULT_COUNT] = {0};
		process_dnsreq(sgwupf_node_sel, sgwu_list, &sgwu_count);

		if (!sgwu_count) {

			record_failed_enbid(enodeb);
			deinit_node_selector(sgwupf_node_sel);

			/* Query DNS based on lb and hb of tac */
			char lb[8] = {0};
			char hb[8] = {0};

			if (ctxt->uli.tai != 1) {
				LOG_MSG(LOG_ERROR, "Could not get SGW-U list using DNS"
								"query. TAC missing in CSR.");
				return 0;
			}

			sprintf(lb, "%u", ctxt->uli.tai2.tai_tac & 0xFF);
			sprintf(hb, "%u", (ctxt->uli.tai2.tai_tac >> 8) & 0xFF);

			sgwupf_node_sel = init_sgwupf_node_selector(lb, hb, mnc, mcc);

			set_desired_proto(sgwupf_node_sel, SGWUPFNODESELECTOR, UPF_X_SXA);

			if(strlen(apn_requested->apn_net_cap) > 0) {
				set_nwcapability(sgwupf_node_sel, apn_requested->apn_net_cap);
			}

			if (apn_requested->apn_usage_type != -1) {
				set_ueusage_type(sgwupf_node_sel,
					apn_requested->apn_usage_type);
			}

			process_dnsreq(sgwupf_node_sel, sgwu_list, &sgwu_count);

			if (!sgwu_count) {
				LOG_MSG(LOG_ERROR, "Could not get SGW-U list using DNS query ");
				return 0;
			}
		}

		/* SAEGW-C */
		if (pdn->s5s8_pgw_gtpc_ipv4.s_addr == 0) {

			uint16_t pgwu_count = 0;
			dns_query_result_t pgwu_list[QUERY_RESULT_COUNT] = {0};

			void *pwupf_node_sel = init_pgwupf_node_selector(apn_name,
					mnc, mcc);

			set_desired_proto(pwupf_node_sel, PGWUPFNODESELECTOR, UPF_X_SXB);

			if(strlen(apn_requested->apn_net_cap) > 0) {
				set_nwcapability(pwupf_node_sel, apn_requested->apn_net_cap);
			}

			if (apn_requested->apn_usage_type != -1) {
				set_ueusage_type(pwupf_node_sel,
					apn_requested->apn_usage_type);
			}

			process_dnsreq(pwupf_node_sel, pgwu_list, &pgwu_count);

			/* Get colocated candidate list */
			canonical_result_t result[QUERY_RESULT_COUNT] = {0};
			int res_count = get_colocated_candlist(sgwupf_node_sel,
						pwupf_node_sel, result);

			if (!res_count) {
				deinit_node_selector(pwupf_node_sel);
				return 0;
			}

			/* VS: Need to check this */
			upf_count = add_canonical_result_upflist_entry(result, res_count,
							&ctxt->imsi, sizeof(ctxt->imsi));

			deinit_node_selector(pwupf_node_sel);

		} else { /* SGW-C */

			upf_count = add_dns_result_upflist_entry(sgwu_list, sgwu_count,
							&ctxt->imsi, sizeof(ctxt->imsi));
		}

		deinit_node_selector(sgwupf_node_sel);

	} else if (cp_config->cp_type == PGWC) {

		uint16_t pgwu_count = 0;
		dns_query_result_t pgwu_list[QUERY_RESULT_COUNT] = {0};

		void *pwupf_node_sel = init_pgwupf_node_selector(apn_name, mnc, mcc);

		set_desired_proto(pwupf_node_sel, PGWUPFNODESELECTOR, UPF_X_SXB);
		if(strlen(apn_requested->apn_net_cap) > 0) {
			set_nwcapability(pwupf_node_sel, apn_requested->apn_net_cap);
		}

		if (apn_requested->apn_usage_type != -1) {
			set_ueusage_type(pwupf_node_sel,
				apn_requested->apn_usage_type);
		}

		process_dnsreq(pwupf_node_sel, pgwu_list, &pgwu_count);

		/* VS: Need to check this */
		/* Get collocated candidate list */
		if (!strlen((char *)pdn->fqdn)) {
			LOG_MSG(LOG_ERROR, "SGW-U node name missing in CSR. ");
			deinit_node_selector(pwupf_node_sel);
			return 0;
		}

		canonical_result_t result[QUERY_RESULT_COUNT] = {0};
		int res_count = get_colocated_candlist_fqdn(
				(char *)pdn->fqdn, pwupf_node_sel, result);

		if (!res_count) {
			LOG_MSG(LOG_ERROR, "Could not get collocated candidate list. ");
			deinit_node_selector(pwupf_node_sel);
			return 0;
		}

		upf_count = add_canonical_result_upflist_entry(result, res_count,
							&ctxt->imsi, sizeof(ctxt->imsi));

		deinit_node_selector(pwupf_node_sel);
	}

	return upf_count;
}


int
dns_query_lookup(pdn_connection_t *pdn, uint32_t *upf_ip)
{
	upfs_dnsres_t *entry = NULL;

	if (get_upf_list(pdn) == 0){
		 LOG_MSG(LOG_ERROR, "Error:");
		return GTPV2C_CAUSE_REQUEST_REJECTED;
	}

	/* Fill msg->upf_ipv4 address */
	if ((get_upf_ip(pdn->context, &entry, &upf_ip)) != 0) {
		LOG_MSG(LOG_ERROR, "Failed to get upf ip address");
		return GTPV2C_CAUSE_REQUEST_REJECTED;
	}
	memcpy(pdn->fqdn, entry->upf_fqdn[entry->current_upf],
					strlen(entry->upf_fqdn[entry->current_upf]));
	return 0;
}


long
uptime(void)
{
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0) {
		LOG_MSG(LOG_DEBUG, "Error in uptime");
	}
	return s_info.uptime;
}




uint32_t
current_ntp_timestamp(void) 
{

	struct timeval tim;
	uint8_t ntp_time[8] = {0};
	uint32_t timestamp = 0;

	gettimeofday(&tim, NULL);
	time_to_ntp(&tim, ntp_time);

	timestamp |= ntp_time[0] << 24 | ntp_time[1] << 16
								| ntp_time[2] << 8 | ntp_time[3];

	return timestamp;
}

void
time_to_ntp(struct timeval *tv, uint8_t *ntp)
{
	uint64_t ntp_tim = 0;
	uint8_t len = (uint8_t)sizeof(ntp)/sizeof(ntp[0]);
	uint8_t *p = ntp + len;

	int i = 0;

	ntp_tim = tv->tv_usec;
	ntp_tim <<= 32;
	ntp_tim /= 1000000;

	/* Setting the ntp in network byte order */

	for (i = 0; i < len/2; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}

	ntp_tim = tv->tv_sec;
	ntp_tim += OFFSET;

	/* Settting  the fraction of second */

	for (; i < len; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}

}
