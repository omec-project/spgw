// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <signal.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_hash_crc.h>
#include <errno.h>

#include "clogger.h"
#include "gw_adapter.h"
#include "cp_stats.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "timer.h"
#include "sm_struct.h"
#include "cp_config_apis.h"
#include "cp_config.h"
#include "gtpc_timer.h"
#include "pfcp_timer.h"
#include "cp_interface.h"
#include "pfcp_transactions.h"
#include "gtpv2_internal.h"

#ifdef USE_DNS_QUERY
#include "cdnshelper.h"
#endif /* USE_DNS_QUERY */

int s11_fd = -1;
int s5s8_fd = -1;
int pfcp_fd = -1;
int s11_pcap_fd = -1;

extern pcap_t *pcap_reader;
extern pcap_dumper_t *pcap_dumper;

cp_config_t *cp_config;

udp_sock_t my_sock;
/* MME */
struct sockaddr_in s11_mme_sockaddr;

/* S5S8 */
struct sockaddr_in s5s8_recv_sockaddr;

/* PFCP */
in_port_t pfcp_port;
struct sockaddr_in pfcp_sockaddr;

/* UPF PFCP */
in_port_t upf_pfcp_port;

uint8_t s11_rx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];
struct sockaddr_in s11_sockaddr;
socklen_t s11_mme_sockaddr_len = sizeof(s11_mme_sockaddr);

uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];
struct sockaddr_in s5s8_sockaddr;
socklen_t s5s8_sockaddr_len = sizeof(s5s8_sockaddr);


uint8_t pfcp_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

#ifdef USE_REST
/* ECHO PKTS HANDLING */
uint8_t echo_tx_buf[MAX_GTPV2C_UDP_LEN];
#endif /* USE_REST */

extern uint8_t rstCnt;

#ifdef SYNC_STATS
/**
 * @brief  : Initializes the hash table used to account for statstics of req and resp time.
 * @param  : void
 * @return : void
 */
void
init_stats_hash(void)
{
	struct rte_hash_parameters rte_hash_params = {
			.name = "stats_hash",
	    .entries = STATS_HASH_SIZE,
	    .key_len = sizeof(uint64_t),
	    .hash_func = rte_hash_crc,
	    .hash_func_init_val = 0,
	    .socket_id = rte_socket_id(),
	};

	stats_hash = rte_hash_create(&rte_hash_params);
	if (!stats_hash) {
		rte_panic("%s hash create failed: %s (%u)\n",
				rte_hash_params.name,
		    rte_strerror(rte_errno), rte_errno);
	}
}

#endif /* SYNC_STATS */

static void init_pfcp(void)
{
	int ret;
	pfcp_port = htons(pfcp_config.pfcp_port);

	pfcp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd = pfcp_fd;

	if (pfcp_fd < 0)
		rte_panic("Socket call error : %s", strerror(errno));

	bzero(pfcp_sockaddr.sin_zero,
			sizeof(pfcp_sockaddr.sin_zero));
	pfcp_sockaddr.sin_family = AF_INET;
	pfcp_sockaddr.sin_port = pfcp_port;
	pfcp_sockaddr.sin_addr = pfcp_config.pfcp_ip;

	ret = bind(pfcp_fd, (struct sockaddr *) &pfcp_sockaddr,
			sizeof(struct sockaddr_in));

	clLog(sxlogger, eCLSeverityInfo,  "NGIC- main.c::init_pfcp()" "\n\tpfcp_fd = %d :: "
			"\n\tpfcp_ip = %s : pfcp_port = %d\n",
			pfcp_fd, inet_ntoa(pfcp_config.pfcp_ip),
			ntohs(pfcp_port));
	if (ret < 0) {
		rte_panic("Bind error for %s:%u - %s\n",
				inet_ntoa(pfcp_sockaddr.sin_addr),
				ntohs(pfcp_sockaddr.sin_port),
				strerror(errno));
	}
}


/**
 * @brief  : Initalizes S11 interface if in use
 * @param  : void
 * @return : void
 */
static void init_s11(void)
{
	int ret;
	/* TODO: Need to think*/
	s11_mme_sockaddr.sin_port = htons(cp_config->s11_port);

	if (pcap_reader != NULL && pcap_dumper != NULL)
		return;

	s11_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_s11 = s11_fd;

	if (s11_fd < 0)
		rte_panic("Socket call error : %s", strerror(errno));

	bzero(s11_sockaddr.sin_zero,
			sizeof(s11_sockaddr.sin_zero));
	s11_sockaddr.sin_family = AF_INET;
	s11_sockaddr.sin_port = htons(cp_config->s11_port);
	s11_sockaddr.sin_addr = cp_config->s11_ip;

	ret = bind(s11_fd, (struct sockaddr *) &s11_sockaddr,
			    sizeof(struct sockaddr_in));

	clLog(s11logger, eCLSeverityInfo, "NGIC- main.c::init_s11()"
			"\n\ts11_fd= %d :: "
			"\n\ts11_ip= %s : s11_port= %d\n",
			s11_fd, inet_ntoa(cp_config->s11_ip), cp_config->s11_port);

	if (ret < 0) {
		rte_panic("Bind error for %s:%u - %s\n",
			inet_ntoa(s11_sockaddr.sin_addr),
			ntohs(s11_sockaddr.sin_port),
			strerror(errno));
	}
}

/**
 * @brief  : Initalizes s5s8_sgwc interface if in use
 * @param  : void
 * @return : void
 */
static void init_s5s8(void)
{
	int ret;
	/* TODO: Need to think*/
	s5s8_recv_sockaddr.sin_port = htons(cp_config->s5s8_port);

	if (pcap_reader != NULL && pcap_dumper != NULL)
		return;

	s5s8_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_s5s8 = s5s8_fd;

	if (s5s8_fd < 0)
		rte_panic("Socket call error : %s", strerror(errno));

	bzero(s5s8_sockaddr.sin_zero,
			sizeof(s5s8_sockaddr.sin_zero));
	s5s8_sockaddr.sin_family = AF_INET;
	s5s8_sockaddr.sin_port = htons(cp_config->s5s8_port);
	s5s8_sockaddr.sin_addr = cp_config->s5s8_ip;

	ret = bind(s5s8_fd, (struct sockaddr *) &s5s8_sockaddr,
			    sizeof(struct sockaddr_in));

	clLog(s5s8logger, eCLSeverityInfo, "NGIC- main.c::init_s5s8_sgwc()"
			"\n\ts5s8_fd= %d :: "
			"\n\ts5s8_ip= %s : s5s8_port= %d\n",
			s5s8_fd, inet_ntoa(cp_config->s5s8_ip),
			htons(cp_config->s5s8_port));

	if (ret < 0) {
		rte_panic("Bind error for %s:%u - %s\n",
			inet_ntoa(s5s8_sockaddr.sin_addr),
			ntohs(s5s8_sockaddr.sin_port),
			strerror(errno));
	}
}

#ifdef CP_DP_TABLE_CONFIG
static void initialize_tables_on_dp(void)
{
	struct dp_id dp_id = { .id = DPN_ID };

	sprintf(dp_id.name, SDF_FILTER_TABLE);
	if (sdf_filter_table_create(dp_id, SDF_FILTER_TABLE_SIZE))
		rte_panic("sdf_filter_table creation failed\n");

	sprintf(dp_id.name, ADC_TABLE);
	if (adc_table_create(dp_id, ADC_TABLE_SIZE))
		rte_panic("adc_table creation failed\n");

	sprintf(dp_id.name, PCC_TABLE);
	if (pcc_table_create(dp_id, PCC_TABLE_SIZE))
		rte_panic("pcc_table creation failed\n");

	sprintf(dp_id.name, METER_PROFILE_SDF_TABLE);
	if (meter_profile_table_create(dp_id, METER_PROFILE_SDF_TABLE_SIZE))
		rte_panic("meter_profile_sdf_table creation failed\n");

	sprintf(dp_id.name, SESSION_TABLE);

	if (session_table_create(dp_id, LDB_ENTRIES_DEFAULT))
		rte_panic("session_table creation failed\n");

}
#endif

void init_dp_rule_tables(void)
{

#ifdef CP_DP_TABLE_CONFIG
	initialize_tables_on_dp();
#endif

	init_packet_filters();
	parse_adc_rules();

}
/**
 * @brief  : Initializes Control Plane data structures, packet filters, and calls for the
 *           Data Plane to create required tables
 */
void init_cp(void)
{

	init_pfcp();

	/* AJAY : passing correct spgw service config in container image */
	switch (cp_config->cp_type) {
	case SGWC:
		init_s11();
        break;
	case PGWC:
		init_s5s8();
		break;
	case SAEGWC:
		init_s11();
		break;
	default:
		rte_panic("main.c::init_cp()-"
				"Unknown spgw_cfg= %u\n", cp_config->cp_type);
		break;
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		rte_exit(EXIT_FAILURE, "Error:can't catch SIGINT\n");

	if (signal(SIGSEGV, sig_handler) == SIG_ERR)
		rte_exit(EXIT_FAILURE, "Error:can't catch SIGSEGV\n");

	create_ue_hash();

	create_upf_context_hash();

	create_gx_context_hash();

	create_upf_by_ue_hash();

}

uint8_t update_rstCnt(void)
{
	FILE *fp;
	int tmp;

	if ((fp = fopen(RESTART_CNT_FILE,"rw+")) == NULL){
		if ((fp = fopen(RESTART_CNT_FILE,"w")) == NULL)
			clLog(clSystemLog, eCLSeverityCritical,"Error! creating cp_rstCnt.txt file");
	}

	if (fscanf(fp,"%u", &tmp) < 0) {
		/* Cur pos shift to initial pos */
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%u\n", ++rstCnt);
		fclose(fp);
		return rstCnt;

	}
	/* Cur pos shift to initial pos */
	fseek(fp, 0, SEEK_SET);

	rstCnt = tmp;
	fprintf(fp, "%d\n", ++rstCnt);

	clLog(clSystemLog, eCLSeverityDebug, "Updated restart counter Value of rstcnt=%u\n", rstCnt);
	fclose(fp);

	return rstCnt;
}

void recovery_time_into_file(uint32_t recov_time)
{
    FILE *fp = NULL;

    if ((fp = fopen(HEARTBEAT_TIMESTAMP, "w+")) == NULL) {
        clLog(clSystemLog, eCLSeverityCritical, "Unable to open heartbeat recovery file..\n");

    } else {
        fseek(fp, 0, SEEK_SET);
        fprintf(fp, "%u\n", recov_time);
        fclose(fp);
    }
}
