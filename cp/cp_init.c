// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <signal.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_hash_crc.h>
#include <errno.h>
#include "cp_init.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "cp_stats.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "cp_timer.h"
#include "sm_struct.h"
#include "cp_config_apis.h"
#include "cp_config.h"
#include "cp_interface.h"
#include "cp_transactions.h"
#include "gtpv2_internal.h"
#include "cp_io_poll.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_interface.h"

#ifdef USE_DNS_QUERY
#include "cdnshelper.h"
#endif /* USE_DNS_QUERY */

int s11_pcap_fd = -1;
extern udp_sock_t my_sock;
extern pcap_t *pcap_reader;
extern pcap_dumper_t *pcap_dumper;

cp_config_t *cp_config;

uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];
struct sockaddr_in s5s8_sockaddr;
socklen_t s5s8_sockaddr_len = sizeof(s5s8_sockaddr);
uint8_t pfcp_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

extern uint8_t rstCnt;

static void 
init_pfcp(void)
{
	int ret;
    int pfcp_fd = -1;
    struct sockaddr_in pfcp_sockaddr;

	pfcp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_pfcp = pfcp_fd;

	if (pfcp_fd < 0)
		rte_panic("Socket call error : %s", strerror(errno));

	bzero(pfcp_sockaddr.sin_zero,
			sizeof(pfcp_sockaddr.sin_zero));
	pfcp_sockaddr.sin_family = AF_INET;
	pfcp_sockaddr.sin_port = htons(cp_config->pfcp_port);
	pfcp_sockaddr.sin_addr = cp_config->pfcp_ip;

	ret = bind(pfcp_fd, (struct sockaddr *) &pfcp_sockaddr,
			sizeof(struct sockaddr_in));

	clLog(sxlogger, eCLSeverityInfo,  "NGIC- main.c::init_pfcp()" "\n\tpfcp_fd = %d :: "
			"\n\tpfcp_ip = %s : pfcp_port = %d\n",
			pfcp_fd, inet_ntoa(cp_config->pfcp_ip),
			cp_config->pfcp_port);
	if (ret < 0) {
		rte_panic("Bind error for %s:%u - %s\n",
				inet_ntoa(pfcp_sockaddr.sin_addr),
				ntohs(pfcp_sockaddr.sin_port),
				strerror(errno));
	}
    my_sock.pfcp_sockaddr = pfcp_sockaddr; 
}


/**
 * @brief  : Initalizes S11 interface if in use
 * @param  : void
 * @return : void
 */
static void init_s11(void)
{
    int s11_fd = -1;
	int ret;
    struct sockaddr_in s11_sockaddr;

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
    my_sock.s11_sockaddr = s11_sockaddr;

}

/**
 * @brief  : Initalizes s5s8_sgwc interface if in use
 * @param  : void
 * @return : void
 */
static void init_s5s8(void)
{
    int s5s8_fd = -1;
	int ret;
    struct sockaddr_in s5s8_recv_sockaddr;

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

    my_sock.s5s8_recv_sockaddr = s5s8_recv_sockaddr;
}

/**
 * @brief  : Initializes Control Plane data structures, packet filters, and calls for the
 *           Data Plane to create required tables
 */
void init_cp(void)
{

    init_pfcp_interface();

	init_pfcp();

    init_gtp_interface();

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

	if((my_sock.sock_fd_s11 > my_sock.sock_fd_pfcp) &&
	   (my_sock.sock_fd_s11 > my_sock.gx_app_sock) &&
	   (my_sock.sock_fd_s11 > my_sock.sock_fd_s5s8)) {
        my_sock.select_max_fd = my_sock.sock_fd_s11 + 1;
    }
	else if((my_sock.sock_fd_pfcp > my_sock.sock_fd_s11) &&
	   (my_sock.sock_fd_pfcp > my_sock.gx_app_sock) &&
	   (my_sock.sock_fd_pfcp > my_sock.sock_fd_s5s8)) {
        my_sock.select_max_fd = my_sock.sock_fd_pfcp + 1;
    }
	else if((my_sock.sock_fd_s5s8 > my_sock.sock_fd_s11) &&
	   (my_sock.sock_fd_s5s8 > my_sock.gx_app_sock) &&
	   (my_sock.sock_fd_s5s8 > my_sock.sock_fd_pfcp)) {
        my_sock.select_max_fd = my_sock.sock_fd_s5s8 + 1;
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

void rest_thread_init(void)
{
	sigset_t sigset;

	/* mask SIGALRM in all threads by default */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGRTMIN);
	sigaddset(&sigset, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	if (!gst_init())
	{
		clLog(clSystemLog, eCLSeverityDebug, "%s - gstimer_init() failed!!\n", getPrintableTime() );
	}
	return;
}

