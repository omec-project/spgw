// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include <signal.h>
#include "stdint.h"
#include "cp_interface.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_association.h"
#include "spgw_config_struct.h"
#include "sm_struct.h"
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
#include "upf_apis.h"

uint8_t echo_tx_buf[MAX_GTPV2C_UDP_LEN];

int32_t conn_cnt = 0;

/* Sequence number allocation for echo request */
static uint16_t gtpc_mme_seqnb	= 0;
static uint16_t gtpc_sgwc_seqnb	= 0;
static uint16_t gtpc_pgwc_seqnb	= 0;
static uint16_t pfcp_seqnb	= 1;


void 
timerCallback( gstimerinfo_t *ti, const void *data_t )
{
    struct peer_data *temp = (struct peer_data *)calloc(1, sizeof(struct peer_data));
	peerData_t *md = (peerData_t*)data_t;
	/* setting sendto destination addr */
	temp->dest_addr.sin_family = AF_INET;
	temp->dest_addr.sin_addr.s_addr = md->dstIP;
	temp->dest_addr.sin_port = htons(GTPC_UDP_PORT);
    temp->ti = ti;
    queue_stack_unwind_event(PEER_TIMEOUT, (void *)temp, handle_timeout_event); 
    return;
}

void 
handle_timeout_event(void *data, uint16_t event)
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
	LOG_MSG(LOG_NEVER, "%s:%s:%u.%s (%dms) has expired", 
		md->name, inet_ntoa(*(struct in_addr *)&md->dstIP), md->portId,
		ti == &md->pt ? "Periodic_Timer" :
		ti == &md->tt ? "Transmit_Timer" : "unknown",
		ti->ti_ms );

    // No response for configured number of consecutive echo/heartbeat requests
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
            upf_down_event(dest_addr.sin_addr.s_addr);
		}

		/* Flush sessions */
		if (md->portId == S11_SGW_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, MME node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
		}

		/* Flush sessions */
		if (md->portId == S5S8_SGWC_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, PGWC node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
		}

		/* Flush sessions */
		if (md->portId == S5S8_PGWC_PORT_ID) {
			LOG_MSG(LOG_ERROR, "Stopped Periodic/transmit timer, SGWC node %s is not reachable",
				inet_ntoa(*(struct in_addr *)&md->dstIP));
		}

		del_entry_from_hash(md->dstIP);

		return;
	}

	bzero(&echo_tx_buf, sizeof(echo_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) echo_tx_buf;

	if (md->portId == S11_SGW_PORT_ID) {
		if (ti == &md->pt)
			gtpc_mme_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpc_mme_seqnb);

        increment_mme_peer_stats(MSG_TX_GTPV2_S11_ECHOREQ, dest_addr.sin_addr.s_addr);

	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
		gtpv2c_send(my_sock.sock_fd_s11, echo_tx_buf, payload_length,
		               (struct sockaddr *) &dest_addr,
		               sizeof(struct sockaddr_in));

		if (ti == &md->tt) {
			(md->itr_cnt)++;
		}

	} else if (md->portId == S5S8_SGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpc_sgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpc_sgwc_seqnb);
	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                sizeof(struct sockaddr_in));

        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, dest_addr.sin_addr.s_addr);
		if (ti == &md->tt) {
			(md->itr_cnt)++;
		}

	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpc_pgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpc_pgwc_seqnb);
	    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
		gtpv2c_send(my_sock.sock_fd_s5s8, echo_tx_buf, payload_length,
		                (struct sockaddr *) &dest_addr,
		                sizeof(struct sockaddr_in));

        increment_pgw_peer_stats(MSG_TX_GTPV2_S5S8_ECHOREQ, dest_addr.sin_addr.s_addr);
		if (ti == &md->tt) {
			(md->itr_cnt)++;
		}

	} else if (md->portId == SX_PORT_ID) {
		/* TODO: Defined this part after merging sx heartbeat*/
		/* process_pfcp_heartbeat_req(md->dst_ip, up_time); */ /* TODO: Future Enhancement */

		dest_addr.sin_port = htons(cp_config->pfcp_port);
		//dest_addr.sin_port = htons(8805);

		if (ti == &md->pt){
			pfcp_seqnb = get_pfcp_sequence_number(PFCP_HEARTBEAT_REQUEST, pfcp_seqnb);;
		}

		process_pfcp_heartbeat_req(&dest_addr, pfcp_seqnb);
        increment_userplane_stats(MSG_TX_PFCP_SXASXB_ECHOREQ, dest_addr.sin_addr.s_addr);

		if (ti == &md->tt) {
			(md->itr_cnt)++;
		}
	}

	if (ti == &md->pt) {
		if ( startTimer( &md->tt ) < 0) {
			LOG_MSG(LOG_ERROR, "Transmit Timer failed to start..");
		}
		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
	}
	return;
}

uint8_t 
add_node_conn_entry(uint32_t dstIp, uint8_t portId)
{
	peerData_t *conn_data = NULL;

	conn_data = (peerData_t *)get_peer_entry(dstIp);

	if ( conn_data == NULL) {
		LOG_MSG(LOG_INFO, "Add peer entry in conn table :%s", inet_ntoa(*((struct in_addr *)&dstIp)));

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

	    conn_data->name = "PEER_NODE";

	    if ( !gst_timer_init( &conn_data->pt, ttInterval, timerCallback, cp_config->periodic_timer*1000, conn_data )) {
		   LOG_MSG(LOG_ERROR, "Peer - %s - initialization of periodic timer failed", 
                            inet_ntoa(*((struct in_addr *)&dstIp)));
        }

	    if (!gst_timer_init( &conn_data->tt, ttInterval, timerCallback, cp_config->transmit_timer*1000, conn_data )) {
		   LOG_MSG(LOG_ERROR, "Peer - %s - initialization of transmit timer failed", 
                            inet_ntoa(*((struct in_addr *)&dstIp)));
        }

		if ( startTimer( &conn_data->pt ) < 0) {
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
		}
		conn_cnt++;
	} else {
		/* TODO: peer node entry already exit in conn table */

		LOG_MSG(LOG_DEBUG1, "Peer Connection entry already exist in conn table :%s",
					inet_ntoa(*((struct in_addr *)&dstIp)));
		if ( startTimer( &conn_data->pt ) < 0) {
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to start %s ", inet_ntoa(*((struct in_addr *)&dstIp)));
		}
	}

	LOG_MSG(LOG_DEBUG1, "Current Active Peer Conn Cnt:%u", conn_cnt);
	return 0;
}

uint8_t 
process_response(uint32_t dstIp)
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

		LOG_MSG(LOG_NEVER, "Periodic Timer stopped - since activity seen with peer node %s",inet_ntoa(*(struct in_addr *)&dstIp));

		/* Reset Periodic Timer */
		if ( startTimer( &conn_data->pt ) == false ) { 
			LOG_MSG(LOG_ERROR, "Periodic Timer failed to start for %s ",inet_ntoa(*(struct in_addr *)&dstIp));
        }
	}
	return 0;
}
