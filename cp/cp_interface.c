// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include "assert.h"

#include <rte_common.h>
#include <rte_eal.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_jhash.h>
#include <rte_cfgfile.h>
#include <rte_debug.h>

#include "util.h"
#include "cp_interface.h"
#include "cp_io_poll.h"
#include "clogger.h"
#include "cp_config_defs.h"
#include "ipc_api.h"
#include "gtpv2_internal.h"
#include "cp_config.h"

#define PCAP_TTL           (64)
#define PCAP_VIHL          (0x0045)


#ifdef USE_AF_PACKET
#include <libmnl/libmnl.h>
#endif
#ifdef SGX_CDR
	#define DEALERIN_IP "dealer_in_ip"
	#define DEALERIN_PORT "dealer_in_port"
	#define DEALERIN_MRENCLAVE "dealer_in_mrenclave"
	#define DEALERIN_MRSIGNER "dealer_in_mrsigner"
	#define DEALERIN_ISVSVN "dealer_in_isvsvn"
	#define DP_CERT_PATH "dp_cert_path"
	#define DP_PKEY_PATH "dp_pkey_path"
#endif /* SGX_CDR */

#ifdef TIMER_STATS
#ifdef AUTO_ANALYSIS
extern void print_perf_statistics(void);
#endif /* AUTO_ANALYSIS */
#endif /* TIMER_STATS */

extern pcap_dumper_t *pcap_dumper;
extern struct rte_hash *heartbeat_recovery_hash;

struct rte_hash *node_id_hash;

#define IFACE_FILE "../config/interface.cfg"
#define SET_CONFIG_IP(ip, file, section, entry) \
do {\
	entry = rte_cfgfile_get_entry(file, section, #ip);\
	if (entry == NULL)\
	rte_panic("%s not found in %s", #ip, IFACE_FILE);\
	if (inet_aton(entry, &ip) == 0)\
	rte_panic("Invalid %s in %s", #ip, IFACE_FILE);\
} while (0)

#define SET_CONFIG_PORT(port, file, section, entry) \
do {\
	entry = rte_cfgfile_get_entry(file, section, #port);\
	if (entry == NULL)\
	rte_panic("%s not found in %s", #port, IFACE_FILE);\
	if (sscanf(entry, "%"SCNu16, &port) != 1)\
	rte_panic("Invalid %s in %s", #port, IFACE_FILE);\
} while (0)

/**
 * @brief  : Util to send or dump gtpv2c messages
 * @param  : fd, interface indentifier
 * @param  : t_tx, buffer to store data for peer node
 * @return : void
 */
void gtpc_timer_retry_send(int fd, peerData_t *t_tx)
{
	int bytes_tx;
	struct sockaddr_in tx_sockaddr;
	tx_sockaddr.sin_addr.s_addr = t_tx->dstIP;
	tx_sockaddr.sin_port = t_tx->dstPort;
	if (pcap_dumper) {
		dump_pcap(t_tx->buf_len, t_tx->buf);
	} else {
		bytes_tx = sendto(fd, t_tx->buf, t_tx->buf_len, 0,
			(struct sockaddr *)&tx_sockaddr, sizeof(struct sockaddr_in));

		clLog(clSystemLog, eCLSeverityDebug, "NGIC- main.c::gtpv2c_send()""\n\tgtpv2c_if_fd= %d\n",fd);

	if (bytes_tx != (int) t_tx->buf_len) {
			clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete Timer Retry Message:"
					"%u of %d tx bytes : %s\n",
					t_tx->buf_len, bytes_tx, strerror(errno));
		}
	}
}

// ajay - use this retry timer  for trans 
void pfcp_timer_retry_send_new(int fd, transData_t *t_tx)
{
    assert(0);
	int bytes_tx;
	struct sockaddr_in tx_sockaddr;
	tx_sockaddr.sin_addr.s_addr = t_tx->dstIP;
	tx_sockaddr.sin_port = t_tx->dstPort;
	if (pcap_dumper) {
		dump_pcap(t_tx->buf_len, t_tx->buf);
	} else {
		bytes_tx = sendto(fd, t_tx->buf, t_tx->buf_len, 0,
			(struct sockaddr *)&tx_sockaddr, sizeof(struct sockaddr_in));

		clLog(clSystemLog, eCLSeverityDebug, "NGIC- main.c::gtpv2c_send()""\n\tgtpv2c_if_fd= %d\n",fd);

	if (bytes_tx != (int) t_tx->buf_len) {
			clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete Timer Retry Message:"
					"%u of %d tx bytes : %s\n",
					t_tx->buf_len, bytes_tx, strerror(errno));
		}
	}
}
/**
 * @brief  : Util to send or dump gtpv2c messages
 * @param  : fd, interface indentifier
 * @param  : t_tx, buffer to store data for peer node
 * @return : void
 */
void
pfcp_timer_retry_send(int fd, peerData_t *t_tx)
{
	int bytes_tx;
	struct sockaddr_in tx_sockaddr;
	tx_sockaddr.sin_addr.s_addr = t_tx->dstIP;
	tx_sockaddr.sin_port = t_tx->dstPort;
	if (pcap_dumper) {
		dump_pcap(t_tx->buf_len, t_tx->buf);
	} else {
		bytes_tx = sendto(fd, t_tx->buf, t_tx->buf_len, 0,
			(struct sockaddr *)&tx_sockaddr, sizeof(struct sockaddr_in));

		clLog(clSystemLog, eCLSeverityDebug, "NGIC- main.c::gtpv2c_send()""\n\tgtpv2c_if_fd= %d\n",fd);

	if (bytes_tx != (int) t_tx->buf_len) {
			clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete Timer Retry Message:"
					"%u of %d tx bytes : %s\n",
					t_tx->buf_len, bytes_tx, strerror(errno));
		}
	}
}

void
dump_pcap(uint16_t payload_length, uint8_t *tx_buf)
{
	static struct pcap_pkthdr pcap_tx_header;
	gettimeofday(&pcap_tx_header.ts, NULL);
	pcap_tx_header.caplen = payload_length
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipv4_hdr)
			+ sizeof(struct udp_hdr);
	pcap_tx_header.len = payload_length
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipv4_hdr)
			+ sizeof(struct udp_hdr);
	uint8_t dump_buf[MAX_GTPV2C_UDP_LEN
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipv4_hdr)
			+ sizeof(struct udp_hdr)];
	struct ether_hdr *eh = (struct ether_hdr *) dump_buf;

	memset(&eh->d_addr, '\0', sizeof(struct ether_addr));
	memset(&eh->s_addr, '\0', sizeof(struct ether_addr));
	eh->ether_type = htons(ETHER_TYPE_IPv4);

	struct ipv4_hdr *ih = (struct ipv4_hdr *) &eh[1];

	ih->dst_addr = cp_config->s11_mme_ip.s_addr; /* ajay correct this */
	ih->src_addr = cp_config->s11_ip.s_addr;
	ih->next_proto_id = IPPROTO_UDP;
	ih->version_ihl = PCAP_VIHL;
	ih->total_length =
			ntohs(payload_length
				+ sizeof(struct udp_hdr)
				+ sizeof(struct ipv4_hdr));
	ih->time_to_live = PCAP_TTL;

	struct udp_hdr *uh = (struct udp_hdr *) &ih[1];

	uh->dgram_len = htons(
	    ntohs(ih->total_length) - sizeof(struct ipv4_hdr));
	uh->dst_port = htons(GTPC_UDP_PORT);
	uh->src_port = htons(GTPC_UDP_PORT);

	void *payload = &uh[1];
	memcpy(payload, tx_buf, payload_length);
	pcap_dump((u_char *) pcap_dumper, &pcap_tx_header,
			dump_buf);
	fflush(pcap_dump_file(pcap_dumper));
}

