// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <signal.h>
#include "stdint.h"
#include "cp_interface.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "cp_config.h"
#include "sm_struct.h"
#include "csid_cp_cleanup.h"
#include "csid_api.h"
#include "gtpv2_interface.h"
#include "cp_peer.h"
#include "cp_config_defs.h"
#include "cp_timer.h"
#include "spgw_cpp_wrapper.h"
#include "util.h"
#include "cp_io_poll.h"
#include "spgwStatsPromEnum.h"
#include "cp_events.h"
#include "cp_log.h"

uint8_t echo_tx_buf[MAX_GTPV2C_UDP_LEN];

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
struct peer_data {
    gstimerinfo_t *ti;
    struct sockaddr_in dest_addr;
};

void timerCallback( gstimerinfo_t *ti, const void *data_t )
{
    struct peer_data *temp = calloc(1, sizeof(struct peer_data));
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
	peerData_t *md = (peerData_t*)data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */
	/* setting sendto destination addr */
	temp->dest_addr.sin_family = AF_INET;
	temp->dest_addr.sin_addr.s_addr = md->dstIP;
	temp->dest_addr.sin_port = htons(GTPC_UDP_PORT);
    temp->ti = ti;
    queue_stack_unwind_event(PEER_TIMEOUT, (void *)temp, handle_timeout); 
    return;
}

void handle_timeout(void *data, uint16_t event)
{
    peerData_t *md = NULL;
	uint16_t payload_length;
    struct peer_data *temp = (struct peer_data *)data;
    struct sockaddr_in dest_addr = temp->dest_addr;
    gstimerinfo_t *ti = temp->ti;
    free(temp);
    md = (peerData_t *)get_peer_entry(dest_addr.sin_addr.s_addr); 

    if(md == NULL) {
        LOG_MSG(LOG_ERROR,"Peer (%s) not found. event %d ", inet_ntoa(dest_addr.sin_addr), event);
        return;
    }
	LOG_MSG(LOG_DEBUG, "%s - %s:%s:%u.%s (%dms) has expired", getPrintableTime(),
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


		/* TODO: Flush sessions */
        if (md->portId == SX_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, User Plane node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
            upf_context_t *upf_context = NULL; 
            upf_context = (upf_context_t *)upf_context_entry_lookup(dest_addr.sin_addr.s_addr);
            if(upf_context != NULL)
            {
                upf_context->state = 0;
            }

            invalidate_upf_dns_results(dest_addr.sin_addr.s_addr);

			delete_entry_heartbeat_hash(&dest_addr);
            // invalidate dns results  
#ifdef USE_CSID
			del_peer_node_sess(md->dstIP, SX_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S11_SGW_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, MME node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
#ifdef USE_CSID
			del_pfcp_peer_node_sess(md->dstIP, S11_SGW_PORT_ID);
			del_peer_node_sess(ntohl(md->dstIP), S11_SGW_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_SGWC_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, PGWC node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
#ifdef USE_CSID
			del_pfcp_peer_node_sess(ntohl(md->dstIP), S5S8_SGWC_PORT_ID);
			del_peer_node_sess(md->dstIP, S5S8_SGWC_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_PGWC_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, SGWC node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
#ifdef USE_CSID
			del_pfcp_peer_node_sess(ntohl(md->dstIP), S5S8_PGWC_PORT_ID);
			del_peer_node_sess(md->dstIP, S5S8_PGWC_PORT_ID);
#endif /* USE_CSID */
		}

		del_entry_from_hash(md->dstIP);

		return;
	}

	bzero(&echo_tx_buf, sizeof(echo_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) echo_tx_buf;

	if (md->portId == S11_SGW_PORT_ID) {
		if (ti == &md->pt)
			gtpu_mme_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_mme_seqnb);

        increment_mme_peer_stats(MSG_TX_GTPV2_S11_ECHOREQ, dest_addr.sin_addr.s_addr);

	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
		gtpv2c_send(my_sock.sock_fd_s11, echo_tx_buf, payload_length,
		               (struct sockaddr *) &dest_addr,
		               sizeof(struct sockaddr_in));

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}

	} else if (md->portId == S5S8_SGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_sgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_sgwc_seqnb);
	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                sizeof(struct sockaddr_in));

        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, dest_addr.sin_addr.s_addr);
		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
		}

	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_pgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_pgwc_seqnb);
	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                sizeof(struct sockaddr_in));

        increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, dest_addr.sin_addr.s_addr);
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
        increment_userplane_stats(MSG_TX_PFCP_SXASXB_ECHOREQ, dest_addr.sin_addr.s_addr);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}
	}

	if (ti == &md->pt) {
		if ( startTimer( &md->tt ) < 0)
		{
			LOG_MSG(LOG_ERROR, "Transmit Timer failed to start..");
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
	peerData_t *conn_data = NULL;

	conn_data = (peerData_t *)get_peer_entry(dstIp);

	if ( conn_data == NULL) {

		LOG_MSG(LOG_INFO, " Add entry in conn table :%s",
					inet_ntoa(*((struct in_addr *)&dstIp)));

		/* No conn entry for dstIp
		 * Add conn_data for dstIp at
		 * conn_hash_handle
		 * */

		conn_data = (peerData_t *)calloc(1, sizeof(peerData_t));

		conn_data->portId = portId;
		conn_data->activityFlag = 0;
		conn_data->dstIP = dstIp;
		conn_data->itr = cp_config->transmit_cnt;
		conn_data->itr_cnt = 0;
		conn_data->rcv_time = 0;

		/* Add peer node entry in connection hash table */
		if ((add_peer_entry(dstIp, conn_data)) < 0 ) {
			LOG_MSG(LOG_ERROR, "Failed to add entry in hash table");
		}

		if ( !initpeerData( conn_data, "PEER_NODE", (cp_config->periodic_timer * 1000),
						(cp_config->transmit_timer * 1000)) )
		{
		   LOG_MSG(LOG_ERROR, "%s - initialization of %s failed", getPrintableTime(), conn_data->name );
		   return -1;
		}

		if ( startTimer( &conn_data->pt ) < 0) {
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
		}
		conn_cnt++;

	} else {
		/* TODO: peer node entry already exit in conn table */

		LOG_MSG(LOG_INFO, "Conn entry already exit in conn table :%s",
					inet_ntoa(*((struct in_addr *)&dstIp)));
		if ( startTimer( &conn_data->pt ) < 0)
		{
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
		}
		//conn_cnt++;
	}

	LOG_MSG(LOG_INFO, "Current Active Peer Conn Cnt:%u", conn_cnt);
	return 0;
}



uint8_t process_response(uint32_t dstIp)
{
	peerData_t *conn_data = NULL;

	// TODO : Common peer table is not good..Currently conn_hash_handle is common for all peers
	// e.g. gtp, pfcp, ....
	conn_data = (peerData_t *)get_peer_entry(dstIp);

	if ( conn_data == NULL) {
		LOG_MSG(LOG_ERROR, " Entry not found for NODE :%s",
				inet_ntoa(*(struct in_addr *)&dstIp));
	} else {
		conn_data->itr_cnt = 0;

		/* Stop transmit timer for specific peer node */
		stopTimer( &conn_data->tt );
		/* Stop periodic timer for specific peer node */
		stopTimer( &conn_data->pt );
		/* Reset Periodic Timer */
		if ( startTimer( &conn_data->pt ) == false ) 
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to stop...");

		LOG_MSG(LOG_DEBUG, "Periodic Timer stopped - since activity seen with peer node ");
	}
	return 0;
}


