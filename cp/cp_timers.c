// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <signal.h>
#include "stdint.h"
#include "clogger.h"
#include "cp_interface.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "gw_adapter.h"
#include "sm_struct.h"
#include "csid_cp_cleanup.h"
#include "csid_api.h"
#include "gtpv2_interface.h"
#include "cp_timers.h"
#include "cp_config_defs.h"
#include "timer.h"


extern uint8_t echo_tx_buf[MAX_GTPV2C_UDP_LEN];
extern socklen_t s11_mme_sockaddr_len;
extern socklen_t s5s8_sockaddr_len;
extern udp_sock_t my_sock;

int32_t conn_cnt = 0;

/* Sequence number allocation for echo request */
static uint16_t gtpu_mme_seqnb	= 0;
static uint16_t gtpu_sgwc_seqnb	= 0;
static uint16_t gtpu_pgwc_seqnb	= 0;
static uint16_t gtpu_sx_seqnb	= 1;


/**
 * rte hash handler.
 *
 * hash handles connection for S1U, SGI and PFCP
 */
struct rte_hash *conn_hash_handle;

void timerCallback( gstimerinfo_t *ti, const void *data_t )
{
	uint16_t payload_length;
	struct sockaddr_in dest_addr;
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
	peerData_t *md = (peerData_t*)data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */


	/* setting sendto destination addr */
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = md->dstIP;
	dest_addr.sin_port = htons(GTPC_UDP_PORT);

	CLIinterface it = S5S8;
	if (md->portId == S11_SGW_PORT_ID)
	{
		it = S11;
	} else if (md->portId == SX_PORT_ID)
	{
		it = SX;
	} else if (md->portId == S5S8_SGWC_PORT_ID || md->portId == S5S8_PGWC_PORT_ID)
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

		if (md->portId == S11_SGW_PORT_ID)
		{
			clLog(s11logger, eCLSeverityDebug, "MME status : Inactive\n");
		}

		if (md->portId == SX_PORT_ID)
		{
			clLog(sxlogger, eCLSeverityDebug, " SGWU/SPGWU/PGWU status : Inactive\n");
		}
		if (md->portId == S5S8_SGWC_PORT_ID)
		{
			clLog(s5s8logger, eCLSeverityDebug, "PGWC status : Inactive\n");
		}
		if (md->portId == S5S8_PGWC_PORT_ID)
		{
			clLog(s5s8logger, eCLSeverityDebug, "SGWC status : Inactive\n");
		}

		/* TODO: Flush sessions */
		if (md->portId == SX_PORT_ID) {
			delete_entry_heartbeat_hash(&dest_addr);
#ifdef USE_CSID
			del_peer_node_sess(md->dstIP, SX_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S11_SGW_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(md->dstIP, S11_SGW_PORT_ID);
			del_peer_node_sess(ntohl(md->dstIP), S11_SGW_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_SGWC_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(ntohl(md->dstIP), S5S8_SGWC_PORT_ID);
			del_peer_node_sess(md->dstIP, S5S8_SGWC_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_PGWC_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(ntohl(md->dstIP), S5S8_PGWC_PORT_ID);
			del_peer_node_sess(md->dstIP, S5S8_PGWC_PORT_ID);
#endif /* USE_CSID */
		}

		del_entry_from_hash(md->dstIP); // ajay - what to do this ?

		return;
	}

	bzero(&echo_tx_buf, sizeof(echo_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) echo_tx_buf;

	if (md->portId == S11_SGW_PORT_ID) {
		if (ti == &md->pt)
			gtpu_mme_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_mme_seqnb);
	} else if (md->portId == S5S8_SGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_sgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_sgwc_seqnb);
	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_pgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_pgwc_seqnb);
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
			+ sizeof(gtpv2c_tx->gtpc);

	if (md->portId == S11_SGW_PORT_ID) {
		gtpv2c_send(my_sock.sock_fd_s11, echo_tx_buf, payload_length,
		               (struct sockaddr *) &dest_addr,
		               s11_mme_sockaddr_len);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}

	} else if (md->portId == S5S8_SGWC_PORT_ID) {

		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                s5s8_sockaddr_len);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
		}

	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                s5s8_sockaddr_len);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
		}

	} else if (md->portId == SX_PORT_ID) {
		/* TODO: Defined this part after merging sx heartbeat*/
		/* process_pfcp_heartbeat_req(md->dst_ip, up_time); */ /* TODO: Future Enhancement */

		dest_addr.sin_port = htons(cp_config->pfcp_port);
		//dest_addr.sin_port = htons(8805);

		if (ti == &md->pt){
			gtpu_sx_seqnb = get_pfcp_sequence_number(PFCP_HEARTBEAT_REQUEST, gtpu_sx_seqnb);;
		}

		process_pfcp_heartbeat_req(&dest_addr, gtpu_sx_seqnb);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}

	}

	/*CLI:update echo/hbt req sent count*/
	if (md->portId != SX_PORT_ID)
	{
		update_cli_stats(md->dstIP,GTP_ECHO_REQ,SENT,it);
	} else {
		update_cli_stats(md->dstIP,PFCP_HEARTBEAT_REQUEST,SENT,it);
	}

	if(ti == &md->tt)
	{
		update_peer_timeouts(md->dstIP,md->itr_cnt);
	}


	if (ti == &md->pt) {
		if ( startTimer( &md->tt ) < 0)
		{
			clLog(clSystemLog, eCLSeverityCritical, "Transmit Timer failed to start..\n");
		}

		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
	}
	return;
}

bool initpeerData( peerData_t *md, const char *name, int ptms, int ttms )
{
	md->name = name;

	if ( !gst_timer_init( &md->pt, ttInterval, timerCallback, ptms, md ) )
		return False;

	return gst_timer_init( &md->tt, ttInterval, timerCallback, ttms, md );
}

uint8_t add_node_conn_entry(uint32_t dstIp, uint8_t portId)
{
	int ret;
	peerData_t *conn_data = NULL;

	ret = rte_hash_lookup_data(conn_hash_handle,
				&dstIp, (void **)&conn_data);

	if ( ret < 0) {

		clLog(clSystemLog, eCLSeverityDebug, " Add entry in conn table :%s\n",
					inet_ntoa(*((struct in_addr *)&dstIp)));

		/* No conn entry for dstIp
		 * Add conn_data for dstIp at
		 * conn_hash_handle
		 * */

		conn_data = rte_malloc_socket(NULL,
						sizeof(peerData_t),
						RTE_CACHE_LINE_SIZE, rte_socket_id());

		conn_data->portId = portId;
		conn_data->activityFlag = 0;
		conn_data->dstIP = dstIp;
		conn_data->itr = cp_config->transmit_cnt;
		conn_data->itr_cnt = 0;
		conn_data->rcv_time = 0;

		/* Add peer node entry in connection hash table */
		if ((rte_hash_add_key_data(conn_hash_handle,
				&dstIp, conn_data)) < 0 ) {
			clLog(clSystemLog, eCLSeverityCritical, "Failed to add entry in hash table");
		}

		if ( !initpeerData( conn_data, "PEER_NODE", (cp_config->periodic_timer * 1000),
						(cp_config->transmit_timer * 1000)) )
		{
		   clLog(clSystemLog, eCLSeverityCritical, "%s - initialization of %s failed\n", getPrintableTime(), conn_data->name );
		   return -1;
		}

		/*CLI: when add node conn entry called then always peer status will be true*/
		update_peer_status(dstIp,TRUE);

		/* clLog(clSystemLog, eCLSeverityDebug,"Timers PERIODIC:%d, TRANSMIT:%d, COUNT:%u\n",
		 *cp_config->periodic_timer, cp_config->transmit_timer, cp_config->transmit_cnt);
		 */

		if ( startTimer( &conn_data->pt ) < 0) {
			clLog(clSystemLog, eCLSeverityCritical, "Periodic Timer failed to start...\n");
			}
		conn_cnt++;

	} else {
		/* TODO: peer node entry already exit in conn table */

		clLog(clSystemLog, eCLSeverityDebug, "Conn entry already exit in conn table :%s\n",
					inet_ntoa(*((struct in_addr *)&dstIp)));
		if ( startTimer( &conn_data->pt ) < 0)
		{
			clLog(clSystemLog, eCLSeverityCritical, "Periodic Timer failed to start...\n");
		}
		//conn_cnt++;
	}

	clLog(clSystemLog, eCLSeverityDebug, "Current Active Conn Cnt:%u\n", conn_cnt);
	return 0;
}
#ifdef USE_REST
/**
 * @brief  : Function to initialize/create shared ring, ring_container and mem_pool to
 *           inter-communication between DL and iface core.
 * @param  : No param
 * @return : Returns nothing
 */

static 
void echo_table_init(void)
{
    struct rte_hash_parameters
        conn_hash_params = {
            .name = "CONN_TABLE",
            .entries = NUM_CONN,
            .reserved = 0,
            .key_len = sizeof(uint32_t),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0
        };

    /* Create conn_hash for maintain each port peer connection details */
    /* Create arp_hash for each port */
    conn_hash_params.socket_id = rte_socket_id();
    conn_hash_handle = rte_hash_create(&conn_hash_params);
    if (!conn_hash_handle) {
        rte_panic("%s::"
                "\n\thash create failed::"
                "\n\trte_strerror= %s; rte_errno= %u\n",
                conn_hash_params.name,
                rte_strerror(rte_errno),
                rte_errno);
    }
    return;
}

void rest_thread_init(void)
{
	echo_table_init();

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
#endif /* USE_REST */

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
		if ( startTimer( &conn_data->pt ) == false ) /* Changed - Ajay */
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

