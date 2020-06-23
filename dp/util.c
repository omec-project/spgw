// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <arpa/inet.h>

#include "util.h"

void
construct_udp_hdr(struct rte_mbuf *m, uint16_t len,
		  uint16_t sport, uint16_t dport)
{
	struct udp_hdr *udp_hdr;

	udp_hdr = get_mtoudp(m);
	udp_hdr->src_port = htons(sport);
	udp_hdr->dst_port = htons(dport);
	udp_hdr->dgram_len = htons(len);

	/* update Udp checksum */
	udp_hdr->dgram_cksum = 0;
}
