// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

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
#include "assert.h"
#include "util.h"
#include "cp_interface.h"
#include "cp_io_poll.h"
#include "cp_config_defs.h"
#include "ipc_api.h"
#include "gtpv2_internal.h"
#include "spgw_config_struct.h"
#include "cp_log.h"
#include "upf_struct.h"

#define PCAP_TTL           (64)
#define PCAP_VIHL          (0x0045)

//extern pcap_dumper_t *pcap_dumper;

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

		LOG_MSG(LOG_DEBUG, "NGIC- main.c::gtpv2c_send()""\n\tgtpv2c_if_fd= %d",fd);

	if (bytes_tx != (int) t_tx->buf_len) {
			LOG_MSG(LOG_ERROR, "Transmitted Incomplete Timer Retry Message:"
					"%u of %d tx bytes : %s",
					t_tx->buf_len, bytes_tx, strerror(errno));
		}
	}
}


void
dump_pcap(uint16_t payload_length, uint8_t *tx_buf)
{
    (void)payload_length;
    (void)tx_buf;
#if 0
	static struct pcap_pkthdr pcap_tx_header;
	gettimeofday(&pcap_tx_header.ts, NULL);
	pcap_tx_header.caplen = payload_length
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipHdr)
			+ sizeof(struct udpHdr);
	pcap_tx_header.len = payload_length
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipHdr)
			+ sizeof(struct udpHdr);
	uint8_t dump_buf[MAX_GTPV2C_UDP_LEN
			+ sizeof(struct ether_hdr)
			+ sizeof(struct ipHdr)
			+ sizeof(struct udpHdr)];
	struct ether_hdr *eh = (struct ether_hdr *) dump_buf;

	memset(&eh->d_addr, '\0', sizeof(struct ether_addr));
	memset(&eh->s_addr, '\0', sizeof(struct ether_addr));
	eh->ether_type = htons(ETHER_TYPE_IPv4);

	struct ipHdr *ih = (struct ipHdr *) &eh[1];

	ih->daddr = cp_config->s11_mme_ip.s_addr; 
	ih->saddr = cp_config->s11_ip.s_addr;
	ih->protocol = IPPROTO_UDP;
	ih->ihl = PCAP_VIHL;
	ih->tot_len =
			ntohs(payload_length
				+ sizeof(struct udpHdr)
				+ sizeof(struct ipHdr));
	ih->ttl = PCAP_TTL;

	struct udpHdr *uh = (struct udpHdr *) &ih[1];

	uh->uh_ulen = htons(ntohs(ih->total_length) - sizeof(struct ipHdr));
	uh->uh_dport = htons(GTPC_UDP_PORT);
	uh->uh_sport = htons(GTPC_UDP_PORT);

	void *payload = &uh[1];
	memcpy(payload, tx_buf, payload_length);
	pcap_dump((u_char *) pcap_dumper, &pcap_tx_header,
			dump_buf);
	fflush(pcap_dump_file(pcap_dumper));
#endif
}

