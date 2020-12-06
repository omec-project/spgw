// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _GTPV2_INTERFACE_H
#define _GTPV2_INTERFACE_H 

#include <stdint.h>
#include "gtpv2_ie.h"
#include "gtpv2_msg_struct.h"
#include "ue.h"
#include "sm_struct.h"
#include "cp_proc.h"

void init_gtp(void);

void init_gtp_msg_handlers(void);

void init_gtp_msg_threads(void);

/**
 * @brief  : Function to build GTP-U echo request
 * @param  : echo_pkt rte_mbuf pointer
 * @param  : gtpu_seqnb, sequence number
 * @return : void
 */
void
build_gtpv2_echo_request(gtpv2c_header_t *echo_pkt, uint16_t gtpu_seqnb);

/**
 * @brief  : Utility to send or dump gtpv2c messages
 * @param  : gtpv2c_tx, gtpv2c message transmission buffer to response message
 * @param  : gtpv2c_pyld_len, gtpv2c message length
 * @param  : dest_addr, ip address of destination
 * @param  : dest_addr_len, destination address length
 * @return : Returns nothing
 */
void
gtpv2c_send(int gtpv2c_if_id, uint8_t *gtpv2c_tx_buf,
			uint16_t gtpv2c_pyld_len, struct sockaddr *dest_addr,
			socklen_t dest_addr_len);


/**
 * @brief  : parses gtpv2c message and populates parse_release_access_bearer_request_t
 *           structure
 * @param  : gtpv2c_rx
 *           buffer containing received release access bearer request message
 * @param  : release_access_bearer_request
 *           structure to contain parsed information from message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
#if 0
int
parse_release_access_bearer_request(gtpv2c_header_t *gtpv2c_rx,
		rel_acc_ber_req *rel_acc_ber_req_t);
#endif


int
process_delete_bearer_resp(del_bearer_rsp_t *db_rsp , uint8_t flag);

/**
 * @brief  : Handles processing of create bearer request  messages
 * @param  : cb_req
 *           message reception  buffer containing the request message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_create_bearer_request(create_bearer_req_t *cb_req);

/**
 * @brief  : Handles the processing of bearer resource commands received by the
 *           control plane.
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing bearer resource command message
 * @param  : gtpv2c_tx
 *           gtpv2c message transmission buffer to contain any triggered message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_bearer_resource_command(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx);



int
process_delete_bearer_request(del_bearer_req_t *db_req , uint8_t flag);

/**
 * @brief  : parses gtpv2c message and populates parse_sgwc_s5s8_create_session_response_t structure
 * @param  : gtpv2c_rx
 *           buffer containing create bearer response message
 * @param  : csr
 *           data structure to contain required information elements from create
 *           create session response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */

int
parse_sgwc_s5s8_create_session_response(gtpv2c_header_t *gtpv2c_rx,
		sgwc_s5s8_create_session_response_t *csr);

/**
 * @brief  : Handles processing of sgwc s5s8 create session response messages
 * @param  : gtpv2c_rx
 *           gtpc2c message reception  buffer containing the response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_sgwc_s5s8_create_session_response(gtpv2c_header_t *gtpv2c_rx);

/**
 * @brief  : Handles processing of sgwc s11 create bearer response messages
 * @param  : gtpv2c_rx
 *           gtpc2c message reception  buffer containing the response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */

int
process_sgwc_s11_create_bearer_response(gtpv2c_header_t *gtpv2c_rx);

/**
 * @brief  : Handles the processing of create bearer response messages received by the
 *           control plane.
 *
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing create bearer response
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_create_bearer_response(gtpv2c_header_t *gtpv2c_rx);

/**
 * @brief  : Handles the processing of create session request messages received by the
 *           control plane
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing the create session request message
 * @param  : gtpv2c_s11_tx
 *           gtpc2c message transmission buffer to contain s11 response message
 * @param  : gtpv2c_s5s8_tx
 *           gtpc2c message transmission buffer to contain s5s8 response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_s11_tx, gtpv2c_header_t *gtpv2c_s5s8_tx);

/**
 * @brief  : Handles the processing of pgwc create session request messages
 *
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing the create session request message
 * @param  : upf_ipv4, upf id address
 * @param  : proc, procedure type
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_pgwc_s5s8_create_session_request(gtpv2c_header_t *gtpv2c_rx,
		struct in_addr *upf_ipv4, uint8_t proc);

/**
 * @brief  : Handles the generation of sgwc s5s8 create session request messages
 * @param  : gtpv2c_s11_rx
 *           gtpc2c message reception  buffer containing s11 request message
 * @param  : gtpv2c_s5s8_tx
 *           gtpc2c message transmission buffer to contain s5s8 response message
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : pdn
 *           PDN Connection data structure pertaining to the session to be created
 * @param  : bearer
 *           Default EPS Bearer corresponding to the PDN Connection to be created
 * @param  : sgwu_fqdn
 *           SGWU fqdn to be sent to PGWC
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
gen_sgwc_s5s8_create_session_request(gtpv2c_header_t *gtpv2c_s11_rx,
		gtpv2c_header_t *gtpv2c_s5s8_tx,
		uint32_t sequence, pdn_connection_t *pdn,
		eps_bearer_t *bearer, char *sgwu_fqdn);

/**
 * @brief  : Handles the processing of delete bearer response messages received by the
 *           control plane.
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing delete bearer response
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_delete_bearer_response(gtpv2c_header_t *gtpv2c_rx);

/**
 * @brief  : Handles the processing of delete session request messages received by the
 *           control plane.
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing delete session request message
 * @param  : gtpv2c_s11_tx
 *           gtpc2c message transmission buffer to contain s11 response message
 * @param  : gtpv2c_s5s8_tx
 *           gtpc2c message transmission buffer to contain s5s8 response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_delete_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_s11_tx, gtpv2c_header_t *gtpv2c_s5s8_tx);

/**
 * @brief  : Handles the generation of sgwc s5s8 delete session request messages
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing delete session request message
 * @param  : gtpv2c_tx
 *           gtpv2c message buffer to contain delete session response message
 * @param  : pgw_gtpc_del_teid
 *           Default pgw_gtpc_del_teid to be deleted on PGW
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : del_ebi
 *           Id of EPS Bearer to be deleted
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
gen_sgwc_s5s8_delete_session_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx, uint32_t pgw_gtpc_del_teid,
		uint32_t sequence, uint8_t del_ebi);

/**
 * @brief  : Handles processing of sgwc s5s8 delete session response messages
 * @param  : gtpv2c_rx
 *           gtpc2c message reception  buffer containing the response message
 * @param  : gtpv2c_tx
 *           gtpc2c message transmission buffer to contain response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
//int
//process_sgwc_s5s8_delete_session_response(gtpv2c_header_t *gtpv2c_s5s8_rx,
//			gtpv2c_header_t *gtpv2c_s11_tx);

/**
 * @brief  : Handles the processing and reply of gtp echo requests received by the control plane
 * @param  : gtpv2c_rx
 *           gtpv2c buffer received by CP containing echo request
 * @param  : gtpv2c_tx
 *           gtpv2c buffer to transmit from CP containing echo response
 * @return : will return 0 to indicate success
 */
int
process_echo_request(gtpv2c_header_t *gtpv2c_rx, gtpv2c_header_t *gtpv2c_tx);

/**
 * @brief  : Handles the processing of modify bearer request messages received by the
 *           control plane.
 * @param  : gtpv2c_rx
 *           gtpv2c message buffer containing the modify bearer request message
 * @param  : gtpv2c_tx
 *           gtpv2c message transmission buffer to response message
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 *           - < 0 for all other errors
 */
int
process_modify_bearer_request(gtpv2c_header_t *gtpv2c_rx,
		gtpv2c_header_t *gtpv2c_tx);



/**
 * @brief  : Creates a Downlink Data Notification message
 * @param  : context
 *           the UE context for the DDN
 * @param  : eps_bearer_id
 *           the eps bearer ID to be included in the DDN
 * @param  : sequence
 *           sequence number as described by clause 7.6 3gpp 29.274
 * @param  : gtpv2c_tx
 *           gtpv2c message buffer containing the Downlink Data Notification to
 *           transmit
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to 3gpp
 *           specified cause error value
 */
int
create_downlink_data_notification(ue_context_t *context, uint8_t eps_bearer_id,
		uint32_t sequence, gtpv2c_header_t *gtpv2c_tx);



/**
 * @brief  : Function to build GTP-U echo request
 * @param  : echo_pkt rte_mbuf pointer
 * @param  : gtpu_seqnb, sequence number
 * @return : Returns nothing
 */
void
build_gtpv2_echo_request(gtpv2c_header_t *echo_pkt, uint16_t gtpu_seqnb);

/**
 * @brief  : Process delete request on sgwc for handover scenario
 * @param  : seid, session id
 * @param  : gtpv2c_tx, gtpv2c message transmission buffer to response message
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_sgwc_delete_handover(uint64_t seid,
		    gtpv2c_header_t *gtpv2c_tx);

/**
 * @brief  : creates and sends downlink data notification according to session
 *           identifier
 * @param  : session_id - session identifier pertaining to downlink data packets
 *           arrived at data plane
 * @return : 0 - indicates success, failure otherwise
 */
int
send_ddn_indication(proc_context_t *ctt, uint8_t ebi_index);

void *msg_handler_gtp(void *);

void 
process_delete_bearer_resp_pfcp_timeout(void *data);

void 
process_create_bearer_request_pfcp_timeout(void *data);

void 
process_pgwc_s5s8_create_session_request_pfcp_timeout(void *data);

void 
process_delete_bearer_resp_pfcp_timeout(void *data);

void
process_delete_bearer_request_pfcp_timeout(void *data);


typedef int (*gtp_handler)(msg_info_t **msg, gtpv2c_header_t *); 

extern gtp_handler gtp_msg_handler[256];

int 
handle_echo_request(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int 
handle_echo_response(msg_info_t **msg, gtpv2c_header_t *gtpv2c_rx);

int
handle_create_session_request(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int
handle_modify_bearer_request(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int
handle_delete_session_request(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int 
handle_rab_request(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int
handle_ddn_ack(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

int 
handle_unknown_msg(msg_info_t **msg, gtpv2c_header_t *gtpv2c_s11_rx);

void process_s5s8_upd_bearer_response_pfcp_timeout(void *data);
void process_pgwc_s5s8_delete_session_request_pfcp_timeout(void *data);
void process_sgwc_s5s8_delete_session_request_pfcp_timeout(void *data);
void process_sgwc_s5s8_create_sess_rsp_pfcp_timeout(void *data);
int handle_create_bearer_response_msg(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx);

void process_gtp_msg(void *data, uint16_t event);


/* Slowly delete following block under if 1*/
int handle_create_session_response_msg(msg_info_t *msg, gtpv2c_header_t *gtpv2c_rx);
int validate_gtpv2_message_content(msg_info_t *msg);

void* out_handler_gtp(void *data);
uint32_t get_gtp_sequence(void);

#ifdef FUTURE_NEEDS
int handle_modify_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_session_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_ddn_ack_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_update_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_update_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_create_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_bearer_request_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_bearer_response_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_bearer_cmd_msg(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_delete_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_update_pdn_conn_set_req(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_update_pdn_conn_set_rsp(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
int handle_pgw_restart_notf_ack(gtpv2c_header_t *gtpv2c_rx, msg_info_t *msg);
#endif

#endif

