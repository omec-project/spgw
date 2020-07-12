// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <unistd.h>
#include <locale.h>
#include <signal.h>

#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_common.h>
#include <rte_ether.h>
#include <rte_mbuf.h>
#include <rte_branch_prediction.h>
#include <sys/time.h>

#include "clogger.h"
#include "up_main.h"
#include "epc_arp.h"
#include "pfcp_up_set_ie.h"
#include "pfcp_up_association.h"
#include "up_peer_struct.h"
#include "timer.h"
#include "up_peer.h"

#include "gw_adapter.h"
#include "gtpu.h"

#define OFFSET 		2208988800ULL


/* Generate new pcap for s1u port */
#ifdef PCAP_GEN
extern pcap_dumper_t *pcap_dumper_west;
extern pcap_dumper_t *pcap_dumper_east;
#endif /* PCAP_GEN */

extern unsigned int fd_array[2];
extern uint16_t cp_comm_port;
//#ifndef STATIC_ARP
static struct sockaddr_in dest_addr[2];
//#endif /* STATIC_ARP */

/**
 * rte hash handler.
 */
/* 2 hash handles, one for S1U and another for SGI */
extern struct rte_hash *arp_hash_handle[NUM_SPGW_PORTS];

/**
 * @brief  : memory pool for queued data pkts.
 */
static char *echo_mpoolname = {
	"echo_mpool",
};

int32_t conn_cnt = 0;
static uint16_t gtpu_seqnb	= 0;
static uint16_t gtpu_sgwu_seqnb	= 0;
static uint16_t gtpu_sx_seqnb	= 1;

/**
 * @brief  : Connection hash params.
 */
static struct rte_hash_parameters
	conn_hash_params = {
			.name = "CONN_TABLE",
			.entries = NUM_CONN,
			.reserved = 0,
			.key_len =
					sizeof(uint32_t),
			.hash_func = rte_jhash,
			.hash_func_init_val = 0
};

/**
 * rte hash handler.
 *
 * hash handles connection for S1U, SGI and PFCP
 */
struct rte_hash *conn_hash_handle;


const char
*eth_addr(struct ether_addr *eth_h)
{

	static char *str;
	sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			eth_h->addr_bytes[0],
			eth_h->addr_bytes[1],
			eth_h->addr_bytes[2],
			eth_h->addr_bytes[3],
			eth_h->addr_bytes[4],
			eth_h->addr_bytes[5]);
	return str;

}

/**
 * @brief  : Print ethernet address
 * @param  : eth_h, ethernet address
 * @param  : type, source or destination
 * @return : Returns nothing
 */
static void
print_eth(struct ether_addr *eth_h, uint8_t type)
{
	if (type == 0) {
	clLog(clSystemLog, eCLSeverityDebug,"\n  ETH:  src=%02X:%02X:%02X:%02X:%02X:%02X",
			eth_h->addr_bytes[0],
			eth_h->addr_bytes[1],
			eth_h->addr_bytes[2],
			eth_h->addr_bytes[3],
			eth_h->addr_bytes[4],
			eth_h->addr_bytes[5]);
	} else if (type == 1) {
	clLog(clSystemLog, eCLSeverityDebug,"\n  ETH:  dst=%02X:%02X:%02X:%02X:%02X:%02X",
			eth_h->addr_bytes[0],
			eth_h->addr_bytes[1],
			eth_h->addr_bytes[2],
			eth_h->addr_bytes[3],
			eth_h->addr_bytes[4],
			eth_h->addr_bytes[5]);
	}

}

/**
 * @brief  : Send arp send
 * @param  : conn_data, peer node connection information
 * @return : Returns 0 in case of success , -1 otherwise
 */
static
uint8_t arp_req_send(peerData_t *conn_data)
{

	clLog(clSystemLog, eCLSeverityDebug, "Sendto:: ret_arp_data->ip= %s\n",
				inet_ntoa(*(struct in_addr *)&conn_data->dstIP));

	if (fd_array[conn_data->portId] > 0) {
		/* setting sendto destination addr */
		dest_addr[conn_data->portId].sin_family = AF_INET;
		//dest_addr[portId].sin_addr.s_addr = htonl(dstIp);
		dest_addr[conn_data->portId].sin_addr.s_addr = conn_data->dstIP;
		dest_addr[conn_data->portId].sin_port = htons(SOCKET_PORT);

		char *tmp_buf = (char *)malloc((512) * sizeof(char) + 1);

		int k = 0;
		for(k = 0; k < 512; k++) {
			tmp_buf[k] = 'v';
		}
		tmp_buf[512] = 0;

		if ((sendto(fd_array[conn_data->portId], tmp_buf, strlen(tmp_buf), 0, (struct sockaddr *)
					&dest_addr[conn_data->portId], sizeof(struct sockaddr_in))) < 0) {
			perror("send failed");
			return -1;
		}
	}

	return 0;
}

void timerCallback( gstimerinfo_t *ti, const void *data_t )
{
	peerData_t *md = (peerData_t*)data_t;
	struct rte_mbuf *pkt = rte_pktmbuf_alloc(echo_mpool);
	struct sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = md->dstIP;
	dest_addr.sin_port = htons(cp_comm_port);

	CLIinterface it;

	if (( app.spgw_cfg == (SGWU || SAEGWU )) && md->portId == S1U_PORT_ID)
	{
		it = S1U;
	}
	 if (app.spgw_cfg == SGWU && md->portId == SGI_PORT_ID)
	{
		it = S5S8;
	}
	if (app.spgw_cfg == PGWU && md->portId == S1U_PORT_ID)
	{
		it = S5S8;
	}


	clLog(clSystemLog, eCLSeverityDebug, "%s - %s:%s:%u.%s (%dms) has expired\n", getPrintableTime(),
	      md->name, inet_ntoa(*(struct in_addr *)&md->dstIP), md->portId,
	      ti == &md->pt ? "Periodic_Timer" :
	      ti == &md->tt ? "Transmit_Timer" : "unknown",
	      ti->ti_ms );

	if (md->itr_cnt == md->itr) {
		/* Stop transmit timer for specific Peer Node */
		stopTimer( &md->tt );
		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
		/* Deinit transmit timer for specific Peer Node */
		deinitTimer( &md->tt );
		/* Deinit transmit timer for specific Peer Node */
		deinitTimer( &md->pt );

		clLog(clSystemLog, eCLSeverityDebug, "Stopped Periodic/transmit timer, peer node %s is not reachable\n",
				inet_ntoa(*(struct in_addr *)&md->dstIP));

		update_peer_status(md->dstIP,FALSE);
		delete_cli_peer(md->dstIP);

		/* VS: Flush eNB sessions */
		if ((md->portId == S1U_PORT_ID) || (md->portId == SGI_PORT_ID)) {
			/* TODO: Future Enhancement */
			/* flush_eNB_session(md); */
			del_entry_from_hash(md->dstIP);
		} else if (md->portId == SX_PORT_ID) {
			delete_entry_heartbeat_hash(&dest_addr);
#ifdef USE_CSID
			up_del_pfcp_peer_node_sess(md->dstIP, SX_PORT_ID);
#endif /* USE_CSID */

		}

		return;
	}

	if (md->activityFlag == 1) {
		clLog(clSystemLog, eCLSeverityDebug, "Channel is active for NODE :%s, No need to send echo to it's peer node ..!!\n",
							inet_ntoa(*(struct in_addr *)&md->dstIP));
		md->activityFlag = 0;
		md->itr_cnt = 0;

		/* Stop Timer transmit timer for specific Peer Node */
		stopTimer( &md->tt );
		/* Stop Timer periodic timer for specific Peer Node */
		stopTimer( &md->pt );
		/* VS: Restet Periodic Timer */
		if ( startTimer( &md->pt ) == false)
			clLog(clSystemLog, eCLSeverityCritical, "Periodic Timer failed to start...\n");
		return;
	}

	//clLog(clSystemLog, eCLSeverityDebug,"MAC:%u\n", md->dst_eth_addr.addr_bytes[0]);
	if (md->portId == SX_PORT_ID) {
		clLog(clSystemLog, eCLSeverityDebug, "Send PFCP HeartBeat Request ..!!\n");
		/* VS:TODO: Defined this part after merging sx heartbeat*/
		//process_pfcp_heartbeat_req(md->dst_ip, up_time); /* TODO: Future Enhancement */
		if (ti == &md->pt){
			gtpu_sx_seqnb = get_pfcp_sequence_number(PFCP_HEARTBEAT_REQUEST, gtpu_sx_seqnb);;
		}
		process_pfcp_heartbeat_req(&dest_addr, gtpu_sx_seqnb);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
			update_peer_timeouts(md->dstIP,md->itr_cnt);
		}
		/* TODO: */
		if (ti == &md->pt) {
			if ( startTimer( &md->tt ) == false)
				clLog(clSystemLog, eCLSeverityCritical, "Transmit Timer failed to start..\n");

			/* Stop periodic timer for specific Peer Node */
			stopTimer( &md->pt );
		}

		return;
	}

	if (md->dst_eth_addr.addr_bytes[0] == 0)
	{
		int ret;
		struct arp_entry_data *ret_data = NULL;
		ret = rte_hash_lookup_data(arp_hash_handle[md->portId],
						&(md->dstIP), (void **)&ret_data);
		if (ret < 0) {
			clLog(clSystemLog, eCLSeverityDebug, "ARP is not resolved for NODE :%s\n",
						inet_ntoa(*(struct in_addr *)&md->dstIP));
			if ( (arp_req_send(md)) < 0)
				clLog(clSystemLog, eCLSeverityCritical, "Failed to send ARP request to Node:%s\n",
						inet_ntoa(*(struct in_addr *)&md->dstIP));
			return;
		}

		ether_addr_copy(&ret_data->eth_addr, &md->dst_eth_addr);

		if (md->portId == S1U_PORT_ID) {
			if (ti == &md->pt)
				gtpu_seqnb++;
			build_echo_request(pkt, md, gtpu_seqnb);
		} else if(md->portId == SGI_PORT_ID) {
			if (ti == &md->pt)
				gtpu_sgwu_seqnb++;
			build_echo_request(pkt, md, gtpu_sgwu_seqnb);
		}

	} else {
		if (md->portId == S1U_PORT_ID) {
			if (ti == &md->pt)
				gtpu_seqnb++;
			build_echo_request(pkt, md, gtpu_seqnb);
		} else if(md->portId == SGI_PORT_ID) {
			if (ti == &md->pt)
				gtpu_sgwu_seqnb++;
			build_echo_request(pkt, md, gtpu_sgwu_seqnb);
		}

	}

	//struct rte_mbuf *mt = pkt;
	//struct ipv4_hdr *ipv4_hdr_t = (struct ipv4_hdr*)(rte_pktmbuf_mtod(mt, unsigned char*) + 14);
	//clLog(clSystemLog, eCLSeverityDebug,"**** portId:%u : Enqueu IP DST:%s\n", md->portId, inet_ntoa(*(struct in_addr *)&ipv4_hdr_t->dst_addr));

	if (md->portId == S1U_PORT_ID) {
		clLog(clSystemLog, eCLSeverityDebug, "Pkts enqueue for S1U port..!!\n");

		if (rte_ring_enqueue(shared_ring[S1U_PORT_ID], pkt) == -ENOBUFS) {
			//rte_pktmbuf_free(pkt1);
			clLog(clSystemLog, eCLSeverityCritical, "%s::Can't queue pkt- ring full..."
					" Dropping pkt\n", __func__);
		} else
		{
			update_cli_stats(md->dstIP,GTPU_ECHO_REQUEST,SENT,it);
		}

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
			update_peer_timeouts(md->dstIP,md->itr_cnt);
		}

	} else if(md->portId == SGI_PORT_ID) {
		clLog(clSystemLog, eCLSeverityDebug, "Pkts enqueue for SGI port..!!\n");

		if (rte_ring_enqueue(shared_ring[SGI_PORT_ID], pkt) == -ENOBUFS) {
			//rte_pktmbuf_free(pkt1);
			clLog(clSystemLog, eCLSeverityCritical, "%s::Can't queue pkt- ring full..."
					" Dropping pkt\n", __func__);
		} else
		{
			update_cli_stats(md->dstIP,GTPU_ECHO_REQUEST,SENT,it);
		}

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
			update_peer_timeouts(md->dstIP,md->itr_cnt);
		}
	}
	/* TODO: */
	if (ti == &md->pt) {
		if ( startTimer( &md->tt ) == false)
			clLog(clSystemLog, eCLSeverityCritical, "Transmit Timer failed to start..\n");

		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
	}
}


void
dp_flush_session(uint32_t ip_addr, uint64_t sess_id)
{
	int ret = 0;
	peerData_t *conn_data = NULL;

	clLog(clSystemLog, eCLSeverityDebug, "Flush sess entry from connection table of ip:%s, sess_id:%lu\n",
				inet_ntoa(*(struct in_addr *)&ip_addr), sess_id);

	/* VS: TODO */
	ret = rte_hash_lookup_data(conn_hash_handle,
				&ip_addr, (void **)&conn_data);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityDebug, "Entry not found for NODE :%s\n",
							inet_ntoa(*(struct in_addr *)&ip_addr));
		return;

	} else {
		/* VS: Delete sess id from connection table */
		for(uint32_t cnt = 0; cnt < conn_data->sess_cnt; cnt++) {
			if (sess_id == conn_data->sess_id[cnt]) {
				for(uint32_t pos = cnt; pos < (conn_data->sess_cnt - 1); pos++ )
					conn_data->sess_id[pos] = conn_data->sess_id[pos + 1];

				conn_data->sess_cnt--;
				clLog(clSystemLog, eCLSeverityDebug, "Session Deleted from connection table sid:%lu\n",
						sess_id);
			}
		}
	}

	/* VS: Why they remove this part */
	//if (conn_data->sess_cnt == 0) {
	//	/* Stop Timer for specific eNB */
	//	stopTimer( &conn_data->tt );
	//	stopTimer( &conn_data->pt );

	//	deinitTimer( &conn_data->tt );
	//	deinitTimer( &conn_data->pt );

	//	del_entry_from_hash(ip_addr);
	//	/* rte_free(conn_data); */
	//	conn_data = NULL;

	//	clLog(clSystemLog, eCLSeverityDebug, "Current Active Conn Cnt:%u\n", conn_cnt);
	//	clLog(clSystemLog, eCLSeverityDebug, "Flushed the Timer Entry..!!\n");
	//}

}

void
flush_eNB_session(peerData_t *data_t)
{

	/* VS: Flush DP session table */
	for(uint32_t cnt = 0; cnt < data_t->sess_cnt; cnt++) {
		struct session_info sess_info = {0};
		struct dp_id dp = {0};

		RTE_SET_USED(dp);
		sess_info.sess_id = data_t->sess_id[cnt];

		clLog(clSystemLog, eCLSeverityDebug, "%s: Sess ID's :%lu\n", __func__, sess_info.sess_id);

		//dp_session_delete(dp, &sess_info);

		/* TODO: VS send delete session request to peer control node  */
		//fill_pfcp_sess_del_req();
	}

	/* VS: delete entry from connection hash table */
	del_entry_from_hash(data_t->dstIP); // ajay - what to do for this ?
	rte_free(data_t);
	data_t = NULL;

}

/**
 * @brief  : Check if session exist or not
 * @param  : sess_id, session id
 * @param  : conn_data, peer node information
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
check_sess_id_present(uint64_t sess_id, peerData_t *conn_data)
{
	int sess_exist = 0;
	for(uint32_t cnt = 0; cnt < conn_data->sess_cnt; cnt++) {
		if (sess_id == conn_data->sess_id[cnt]) {
			sess_exist = 1;
		}
	}
	return sess_exist;
}

bool initpeerData( peerData_t *md, const char *name, int ptms, int ttms )
{
	md->name = name;

	if ( !gst_timer_init( &md->pt, ttInterval, timerCallback, ptms, md ) )
		return False;

	return gst_timer_init( &md->tt, ttInterval, timerCallback, ttms, md );
}

uint8_t add_node_conn_entry(uint32_t dstIp, uint64_t sess_id, uint8_t portId)
{

	int ret;
	struct arp_entry_data *ret_conn_data = NULL;
	peerData_t *conn_data = NULL;

	CLIinterface it = SGI;

	ret = rte_hash_lookup_data(conn_hash_handle,
				&dstIp, (void **)&conn_data);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityDebug, "Add entry in conn table :%s, up_seid:%lu\n",
					inet_ntoa(*((struct in_addr *)&dstIp)), sess_id);

		/* No conn entry for dstIp
		 * Add conn_data for dstIp at
		 * conn_hash_handle
		 * */

		conn_data = rte_malloc_socket(NULL,
						sizeof(peerData_t),
						RTE_CACHE_LINE_SIZE, rte_socket_id());

		if (portId == S1U_PORT_ID) {
			if (app.spgw_cfg == PGWU) {
				it = S5S8;
				conn_data->src_eth_addr = app.s5s8_pgwu_ether_addr;
				conn_data->srcIP = app.s5s8_pgwu_ip;
			} else {
				it = S1U;
				conn_data->src_eth_addr = app.s1u_ether_addr;
				conn_data->srcIP = app.s1u_ip;
			}

		} else if (portId == SGI_PORT_ID) {
			if (app.spgw_cfg == SGWU) {
				it = S5S8;
				conn_data->src_eth_addr = app.s5s8_sgwu_ether_addr;
				conn_data->srcIP = app.s5s8_sgwu_ip;
			} else {
				it = SGI;
				conn_data->src_eth_addr = app.sgi_ether_addr;
				conn_data->srcIP = app.sgi_ip;
			}
		}

		if (portId == SX_PORT_ID)
		{
			it = SX;

		}

		conn_data->portId = portId;
		conn_data->activityFlag = 0;
		conn_data->dstIP = dstIp;
		//conn_data->dstIP = htonl(dstIp);
		conn_data->itr = app.transmit_cnt;
		conn_data->itr_cnt = 0;
		conn_data->sess_cnt = 0;
		conn_data->sess_id[conn_data->sess_cnt] = sess_id;

		if ( sess_id > 0)
			conn_data->sess_cnt++;

		if ((portId == S1U_PORT_ID) || (portId == SGI_PORT_ID)) {
			ret = rte_hash_lookup_data(arp_hash_handle[portId],
							&conn_data->dstIP, (void **)&ret_conn_data);

			if (ret < 0) {
				if ( (arp_req_send(conn_data)) < 0)
					clLog(clSystemLog, eCLSeverityCritical, "Failed to send ARP request to Node:%s\n",
							inet_ntoa(*(struct in_addr *)&conn_data->dstIP));
			} else {
				clLog(clSystemLog, eCLSeverityDebug, "ARP Entry found for %s\n",
						inet_ntoa(*((struct in_addr *)&dstIp)));
				ether_addr_copy(&ret_conn_data->eth_addr, &conn_data->dst_eth_addr);
			}
		}


		/* VS: Add peer node entry in connection hash table */
		if ((rte_hash_add_key_data(conn_hash_handle,
				&dstIp, conn_data)) < 0 ) {
			clLog(clSystemLog, eCLSeverityCritical, "Failed to add entry in hash table");
		}

		if ( !initpeerData( conn_data, "PEER_NODE", (app.periodic_timer * 1000), (app.transmit_timer * 1000)) )
		{
		   clLog(clSystemLog, eCLSeverityCritical, "%s - initialization of %s failed\n", getPrintableTime(), conn_data->name );
		   return -1;
		}

		/*CLI:add peer for sgwu/pgwu*/
		add_cli_peer(dstIp,it);
		update_peer_status(dstIp,TRUE);

		if ( startTimer( &conn_data->pt ) ==false)
			clLog(clSystemLog, eCLSeverityCritical, "Periodic Timer failed to start...\n");

		conn_cnt++;



	} else {
		/* VS: eNB entry already exit in conn table */
		clLog(clSystemLog, eCLSeverityDebug, "Conn entry already exit in conn table :%s\n",
					inet_ntoa(*((struct in_addr *)&dstIp)));

		conn_data->sess_id[conn_data->sess_cnt] = sess_id;

		if (sess_id > 0) {
			if(check_sess_id_present(sess_id, conn_data) == 0){
				conn_data->sess_cnt++;
			}
		}
	}

	clLog(clSystemLog, eCLSeverityDebug, "Current Active Conn Cnt:%u\n", conn_cnt);
	return 0;



}

void
echo_table_init(void)
{

	/* Create conn_hash for maintain each port peer connection details */
	/* Create arp_hash for each port */
	conn_hash_params.socket_id = rte_socket_id();
	conn_hash_handle =
			rte_hash_create(&conn_hash_params);
	if (!conn_hash_handle) {
		rte_panic("%s::"
				"\n\thash create failed::"
				"\n\trte_strerror= %s; rte_errno= %u\n",
				conn_hash_params.name,
				rte_strerror(rte_errno),
				rte_errno);
	}

	/* Create echo_pkt TX mmempool for each port */
	echo_mpool = rte_pktmbuf_pool_create(
			echo_mpoolname,
			NB_ECHO_MBUF, 32,
			0, RTE_MBUF_DEFAULT_BUF_SIZE,
			rte_socket_id());
	if (echo_mpool == NULL) {
		rte_panic("rte_pktmbuf_pool_create failed::"
				"\n\techo_mpoolname= %s;"
				"\n\trte_strerror= %s\n",
				echo_mpoolname,
				rte_strerror(abs(errno)));
		return;
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
		clLog(clSystemLog, eCLSeverityCritical, "%s - gstimer_init() failed!!\n", getPrintableTime() );
		//return 1;
	}
}

uint8_t process_response(uint32_t dstIp)
{
	int ret = 0;
	peerData_t *conn_data = NULL;

	// TODO : BUG COmmon peer table is not good..Currently conn_hash_handle is common for all peers
	// e.g. gtp, pfcp, ....
	ret = rte_hash_lookup_data(conn_hash_handle,
			&dstIp, (void **)&conn_data);

	if ( ret < 0) {
		clLog(clSystemLog, eCLSeverityDebug, " Entry not found for NODE :%s\n",
				inet_ntoa(*(struct in_addr *)&dstIp));
	} else {
		conn_data->itr_cnt = 0;

		update_peer_timeouts(conn_data->dstIP,0);

		/* Stop transmit timer for specific peer node */
		stopTimer( &conn_data->tt );
		/* Stop periodic timer for specific peer node */
		stopTimer( &conn_data->pt );
		/* Reset Periodic Timer */
		if ( startTimer( &conn_data->pt ) == false ) 
			clLog(clSystemLog, eCLSeverityCritical, "Periodic Timer failed to start...\n");

	}
	return 0;
}

void del_entry_from_hash(uint32_t ipAddr)
{

	int ret = 0;

	clLog(clSystemLog, eCLSeverityDebug, " Delete entry from connection table of ip:%s\n",
			inet_ntoa(*(struct in_addr *)&ipAddr));

	/* Delete entry from connection hash table */
	ret = rte_hash_del_key(conn_hash_handle,
			&ipAddr);

	if (ret == -ENOENT)
		clLog(clSystemLog, eCLSeverityDebug, "key is not found\n");
	if (ret == -EINVAL)
		clLog(clSystemLog, eCLSeverityDebug, "Invalid Params: Failed to del from hash table\n");
	if (ret < 0)
		clLog(clSystemLog, eCLSeverityDebug, "VS: Failed to del entry from hash table\n");

	//conn_cnt--; ajay

}
