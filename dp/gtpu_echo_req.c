// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <rte_log.h>
#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include "ipv4.h"
#include "gtpu.h"
#include "util.h"
#include "clogger.h"

#define IP_PROTO_UDP   17
#define UDP_PORT_GTPU  2152
#define GTPU_OFFSET    50

#define GTPu_VERSION	0x20
#define GTPu_PT_FLAG	0x10
#define GTPu_E_FLAG		0x04
#define GTPu_S_FLAG		0x02
#define GTPu_PN_FLAG	0x01

#define PKT_SIZE    54

/**
 * @brief  : Maintains data of gtpu header
 */
typedef struct gtpuHdr_s {
	uint8_t version_flags;
	uint8_t msg_type;
	uint16_t tot_len;
	uint32_t teid;
	uint16_t seq_no;		/**< Optional fields if E, S or PN flags set */
} __attribute__((__packed__)) gtpuHdr_t;

/**
 * @brief  : Function to set checksum of IPv4 and UDP header
 * @param  : echo_pkt rte_mbuf pointer
 * @return : Returns nothing
 */
static void set_checksum(struct rte_mbuf *echo_pkt) {
	struct ipv4_hdr *ipv4hdr = get_mtoip(echo_pkt);
	ipv4hdr->hdr_checksum = 0;
	struct udp_hdr *udphdr = get_mtoudp(echo_pkt);
	udphdr->dgram_cksum = 0;
	udphdr->dgram_cksum = rte_ipv4_udptcp_cksum(ipv4hdr, udphdr);
	ipv4hdr->hdr_checksum = rte_ipv4_cksum(ipv4hdr);
}

/**
 * @brief  : Encapsulate gtpu header
 * @param  : m, rte_mbuf pointer
 * @param  : gtpu_seqnb, sequence number
 * @param  : type, message type
 * @return : Returns nothing
 */
static __inline__ void encap_gtpu_hdr(struct rte_mbuf *m, uint16_t gtpu_seqnb, uint8_t type)
{
	uint32_t teid = 0;
	uint16_t len = rte_pktmbuf_data_len(m) - (ETH_HDR_LEN + IPV4_HDR_LEN + UDP_HDR_LEN);

	len -= GTPU_HDR_LEN;

	gtpuHdr_t  *gtpu_hdr = (gtpuHdr_t*)(rte_pktmbuf_mtod(m, unsigned char *) +
		ETH_HDR_LEN + IPV4_HDR_LEN + UDP_HDR_LEN);

	gtpu_hdr->version_flags = (GTPU_VERSION << 5) | (GTP_PROTOCOL_TYPE_GTP << 4) | (GTP_FLAG_SEQNB);
	gtpu_hdr->msg_type = type;
	gtpu_hdr->teid = htonl(teid);
	gtpu_hdr->seq_no = htons(gtpu_seqnb);
	gtpu_hdr->tot_len = htons(len);

}


/**
 * @brief  : Create and initialize udp header
 * @param  : m, rte_mbuf pointer
 * @param  : entry, peer node information
 * @return : Returns nothing
 */
static __inline__ void create_udp_hdr(struct rte_mbuf *m, peerData_t *entry)
{
	uint16_t len = rte_pktmbuf_data_len(m)- ETH_HDR_LEN - IPV4_HDR_LEN;

	struct udp_hdr *udp_hdr = (struct udp_hdr*)(rte_pktmbuf_mtod(m, unsigned char *) +
				ETH_HDR_LEN + IPV4_HDR_LEN);
	udp_hdr->src_port = htons(UDP_PORT_GTPU);
	udp_hdr->dst_port = htons(UDP_PORT_GTPU);
	udp_hdr->dgram_len = htons(len);
	udp_hdr->dgram_cksum = 0;
}


/**
 * @brief  : Create and initialize ipv4 header
 * @param  : m, rte_mbuf pointer
 * @param  : entry, peer node information
 * @return : Returns nothing
 */
static __inline__ void create_ipv4_hdr(struct rte_mbuf *m, peerData_t *entry)
{
	uint16_t len = rte_pktmbuf_data_len(m)- ETH_HDR_LEN;
	struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr*)(rte_pktmbuf_mtod(m, unsigned char*) + ETH_HDR_LEN);
	ipv4_hdr->version_ihl = 0x45;
	ipv4_hdr->type_of_service = 0;
	ipv4_hdr->packet_id = 0x1513;
	ipv4_hdr->fragment_offset = 0;
	ipv4_hdr->time_to_live = 64;
	ipv4_hdr->next_proto_id = IP_PROTO_UDP;
	ipv4_hdr->total_length = htons(len);
	ipv4_hdr->src_addr = entry->srcIP;
	ipv4_hdr->dst_addr = entry->dstIP;
	ipv4_hdr->hdr_checksum = 0;
}


/**
 * @brief  : Create and initialize ether header
 * @param  : m, rte_mbuf pointer
 * @param  : entry, peer node information
 * @return : Returns nothing
 */
static __inline__ void create_ether_hdr(struct rte_mbuf *m, peerData_t *entry)
{
	struct ether_hdr *eth_hdr = (struct ether_hdr*)rte_pktmbuf_mtod(m, void*);
	ether_addr_copy(&entry->dst_eth_addr, &eth_hdr->d_addr);
	ether_addr_copy(&entry->src_eth_addr, &eth_hdr->s_addr);
	eth_hdr->ether_type = 0x08;
}


void build_echo_request(struct rte_mbuf *echo_pkt, peerData_t *entry, uint16_t gtpu_seqnb)
{
	echo_pkt->pkt_len = PKT_SIZE;
	echo_pkt->data_len = PKT_SIZE;


	encap_gtpu_hdr(echo_pkt, gtpu_seqnb, GTPU_ECHO_REQUEST);
	create_udp_hdr(echo_pkt, entry);
	create_ipv4_hdr(echo_pkt, entry);
	create_ether_hdr(echo_pkt, entry);

	/* Set outer IP and UDP checksum, after inner IP and UDP checksum is set.
	 */
	set_checksum(echo_pkt);
}

void build_endmarker_and_send(struct sess_info_endmark *edmk)
{
	static uint16_t seq = 0;
	peerData_t entry;

	entry.dstIP = edmk->dst_ip;
	entry.srcIP = edmk->src_ip;

	memcpy(&(entry.src_eth_addr), &(edmk->source_MAC), sizeof(struct ether_addr));
	memcpy(&(entry.dst_eth_addr), &(edmk->destination_MAC), sizeof(struct ether_addr));

	struct rte_mbuf *endmk_pkt = rte_pktmbuf_alloc(echo_mpool);
	endmk_pkt->pkt_len = PKT_SIZE;
	endmk_pkt->data_len = PKT_SIZE;

	uint16_t len = rte_pktmbuf_data_len(endmk_pkt) - (ETH_HDR_LEN + IPV4_HDR_LEN + UDP_HDR_LEN);

	len -= GTPU_HDR_LEN;

	gtpuHdr_t  *gtpu_hdr = (gtpuHdr_t*)(rte_pktmbuf_mtod(endmk_pkt, unsigned char *) +
			ETH_HDR_LEN + IPV4_HDR_LEN + UDP_HDR_LEN);

	gtpu_hdr->version_flags = (GTPU_VERSION << 5) | (GTP_PROTOCOL_TYPE_GTP << 4) | (GTP_FLAG_SEQNB);
	gtpu_hdr->msg_type = GTPU_END_MARKER_REQUEST;
	gtpu_hdr->teid = htonl(edmk->teid);
	gtpu_hdr->seq_no = htons(++seq);
	gtpu_hdr->tot_len = htons(len);

	create_udp_hdr(endmk_pkt, &entry);
	create_ipv4_hdr(endmk_pkt, &entry);
	create_ether_hdr(endmk_pkt, &entry);

	set_checksum(endmk_pkt);


	if (rte_ring_enqueue(shared_ring[S1U_PORT_ID], endmk_pkt) == -ENOBUFS) {
		rte_pktmbuf_free(endmk_pkt);
		clLog(clSystemLog, eCLSeverityCritical, "%s::Can't queue endmarker pkt- ring full..."
				" Dropping pkt\n", __func__);

	}
}
