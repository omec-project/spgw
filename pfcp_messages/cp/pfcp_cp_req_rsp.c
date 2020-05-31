/*
 * Copyright (c) 2019 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <byteswap.h>

#include "pfcp_cp_util.h"
#include "dp_ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "gw_adapter.h"
#include "clogger.h"

#include "pfcp.h"
#include "sm_arr.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "cp_config_new.h"

uint16_t dp_comm_port;
uint16_t cp_comm_port;

struct in_addr dp_comm_ip;

/*
 * UDP Socket
 */
extern udp_sock_t my_sock;

extern struct rte_hash *heartbeat_recovery_hash;

extern struct sockaddr_in upf_pfcp_sockaddr;

extern pfcp_config_t pfcp_config;
extern int s5s8_fd;
extern socklen_t s5s8_sockaddr_len;
extern socklen_t s11_mme_sockaddr_len;
extern struct sockaddr_in s5s8_recv_sockaddr;

/**
 * @brief  : Process incoming heartbeat request and send response
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
process_heartbeat_request(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{
	int encoded = 0;
	int decoded = 0;
	uint8_t pfcp_msg[1024]= {0};

	RTE_SET_USED(decoded);

	memset(pfcp_msg, 0, 1024);
	pfcp_hrtbeat_req_t *pfcp_heartbeat_req = malloc(sizeof(pfcp_hrtbeat_req_t));
	pfcp_hrtbeat_rsp_t  pfcp_heartbeat_resp = {0};
	decoded = decode_pfcp_hrtbeat_req_t(buf_rx, pfcp_heartbeat_req);
	fill_pfcp_heartbeat_resp(&pfcp_heartbeat_resp);
	pfcp_heartbeat_resp.header.seid_seqno.no_seid.seq_no = pfcp_heartbeat_req->header.seid_seqno.no_seid.seq_no;

	encoded = encode_pfcp_hrtbeat_rsp_t(&pfcp_heartbeat_resp,  pfcp_msg);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_msg;
	pfcp_hdr->message_len = htons(encoded - 4);

#ifdef USE_REST
	/* Reset the periodic timers */
	process_response((uint32_t)peer_addr->sin_addr.s_addr);
#endif /* USE_REST */

	if ( pfcp_send(my_sock.sock_fd, pfcp_msg, encoded, peer_addr) < 0 ) {
		clLog(clSystemLog, eCLSeverityDebug, "Error sending in heartbeat request: %i\n",errno);
	} else {
		update_cli_stats(peer_addr->sin_addr.s_addr,
						PFCP_HEARTBEAT_RESPONSE,SENT,SX);
	}
	free(pfcp_heartbeat_req);
	return 0;
}

/**
 * @brief  : Process hearbeat response message
 * @param  : buf_rx holds data from incoming request
 * @param  : peer_addr used to pass address of peer node
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
process_heartbeat_response(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{

#ifdef USE_REST
	process_response((uint32_t)peer_addr->sin_addr.s_addr);
#endif /*USE_REST*/

	pfcp_hrtbeat_rsp_t pfcp_hearbeat_resp = {0};
	decode_pfcp_hrtbeat_rsp_t(buf_rx, &pfcp_hearbeat_resp);
	uint32_t *recov_time ;

	int ret = rte_hash_lookup_data(heartbeat_recovery_hash , &peer_addr->sin_addr.s_addr ,
			(void **) &(recov_time));

	if (ret == -ENOENT) {
		clLog(clSystemLog, eCLSeverityDebug, "No entry found for the heartbeat!!\n");

	} else {
		/*TODO: Restoration part to be added if recovery time is found greater*/
		uint32_t update_recov_time = 0;
		update_recov_time =  (pfcp_hearbeat_resp.rcvry_time_stmp.rcvry_time_stmp_val);

		if(update_recov_time > *recov_time) {

			ret = rte_hash_add_key_data (heartbeat_recovery_hash,
					&peer_addr->sin_addr.s_addr, &update_recov_time);

			ret = rte_hash_lookup_data(heartbeat_recovery_hash , &peer_addr->sin_addr.s_addr,
					(void **) &(recov_time));
		}
	}

	return 0;
}


/* TODO: Parse byte_rx to process_pfcp_msg */
int
process_pfcp_msg(uint8_t *buf_rx, struct sockaddr_in *peer_addr)
{
	int ret = 0, bytes_rx = 0;
	pfcp_header_t *pfcp_header = (pfcp_header_t *) buf_rx;

	/* TODO: Move this rx */
	if ((bytes_rx = pfcp_recv(pfcp_rx, 512,
					peer_addr)) < 0) {
		perror("msgrecv");
		return -1;
	}

	msg_info msg = {0};
	if(pfcp_header->message_type == PFCP_HEARTBEAT_REQUEST){

		printf("Heartbit request received from UP %s \n",inet_ntoa(peer_addr->sin_addr));
		update_cli_stats(peer_addr->sin_addr.s_addr,
				pfcp_header->message_type,RCVD,SX);

		ret = process_heartbeat_request(buf_rx, peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat request\n", __func__);
			return -1;
		}
		return 0;
	}else if(pfcp_header->message_type == PFCP_HEARTBEAT_RESPONSE){
		printf("Heartbit response received from UP %s \n",inet_ntoa(peer_addr->sin_addr));
		ret = process_heartbeat_response(buf_rx, peer_addr);
		if(ret != 0){
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp heartbeat response\n", __func__);
			return -1;
		} else {

			update_cli_stats(peer_addr->sin_addr.s_addr,
					PFCP_HEARTBEAT_RESPONSE,RCVD,SX);

		}
		return 0;
	}else {
		// ajay - why this is called as response ? it could be request mesage as well 
		printf("PFCP message %d  received from UP %s \n",pfcp_header->message_type, inet_ntoa(peer_addr->sin_addr));
		/*Reset periodic timers*/
		process_response(peer_addr->sin_addr.s_addr);

		if ((ret = pfcp_pcnd_check(buf_rx, &msg, bytes_rx)) != 0) {
			clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, REJ,SX);
			return -1;
		}

		if(pfcp_header->message_type == PFCP_SESSION_REPORT_REQUEST)
			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, RCVD,SX);
		else
			update_cli_stats(peer_addr->sin_addr.s_addr,
							pfcp_header->message_type, ACC,SX);


		printf("[%s] - %d - Procedure - %d state - %d event - %d. Invoke FSM now  \n",__FUNCTION__, __LINE__,msg.proc, msg.state, msg.event);
		if ((msg.proc < END_PROC) && (msg.state < END_STATE) && (msg.event < END_EVNT)) {
			if (SGWC == cp_config->cp_type) {
			    ret = (*state_machine_sgwc[msg.proc][msg.state][msg.event])(&msg, peer_addr);
			} else if (PGWC == cp_config->cp_type) {
			    ret = (*state_machine_pgwc[msg.proc][msg.state][msg.event])(&msg, peer_addr);
			} else if (SAEGWC == cp_config->cp_type) {
			    ret = (*state_machine_saegwc[msg.proc][msg.state][msg.event])(&msg, peer_addr);
			} else {
				clLog(sxlogger, eCLSeverityCritical, "%s : "
						"Invalid Control Plane Type: %d \n",
						__func__, cp_config->cp_type);
				return -1;
			}

			if (ret) {
				clLog(sxlogger, eCLSeverityCritical, "%s : "
						"State_Machine Callback failed with Error: %d \n",
						__func__, ret);
				return -1;
			}
		} else {
			printf("[%s] - %d - Invalid Procedure - %d state - %d event - %d. \n",__FUNCTION__, __LINE__,msg.proc, msg.state, msg.event);
			clLog(s11logger, eCLSeverityCritical, "%s : "
						"Invalid Procedure or State or Event \n",
						__func__);
			return -1;
		}
	}
	return 0;
}
