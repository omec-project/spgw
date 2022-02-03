// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_enum.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "vepc_cp_dp_api.h"
#include "cp_log.h"
#include "pfcp.h"
#include "spgw_config_struct.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "cp_transactions.h"
#include "cp_peer.h"
#include "gen_utils.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
#include "assert.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"

void
fill_pfcp_association_release_req(pfcp_assn_rel_req_t *pfcp_ass_rel_req)
{
	uint32_t seq  = 1;
	memset(pfcp_ass_rel_req, 0, sizeof(pfcp_assn_rel_req_t)) ;

	/*filing of pfcp header*/
	seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_RELEASE_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_rel_req->header),
			PFCP_ASSOCIATION_RELEASE_REQUEST, NO_SEID, seq);
	/*filling of node id*/
	char pAddr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_ass_rel_req->node_id), node_value);
}

void
fill_pfcp_association_update_req(pfcp_assn_upd_req_t *pfcp_ass_update_req)
{
	uint32_t seq  = 1;

	memset(pfcp_ass_update_req, 0, sizeof(pfcp_assn_upd_req_t)) ;

	seq = get_pfcp_sequence_number(PFCP_ASSOCIATION_UPDATE_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_update_req->header),
			 PFCP_ASSOCIATION_UPDATE_REQUEST, NO_SEID, seq);

	char peer_addr[INET_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), peer_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(peer_addr);
	set_node_id(&(pfcp_ass_update_req->node_id), node_value);

	set_upf_features(&(pfcp_ass_update_req->up_func_feat));

	set_cpf_features(&(pfcp_ass_update_req->cp_func_feat));

	set_pfcp_ass_rel_req(&(pfcp_ass_update_req->up_assn_rel_req));

	set_graceful_release_period(&(pfcp_ass_update_req->graceful_rel_period));

}


/**
 * @brief  : This function fills in values to pfd context ie
 * @param  : pfd_contxt is pointer to structure of pfd context ie
 * @return : This function dose not return anything
 */
static void
set_pfd_context(pfcp_pfd_context_ie_t *pfd_conxt)
{

	pfcp_set_ie_header(&pfd_conxt->header, PFCP_IE_PFD_CONTEXT,
			(pfd_conxt->pfd_contents[0].header.len + sizeof(pfcp_ie_header_t)));
	pfd_conxt->pfd_contents_count = 1;

}

/**
 * @brief  : This function fills in values to pfd application id ie
 * @param  : app_id is pointer to structure of pfd application id ie
 * @return : This function dose not return anything
 */
static void
set_pfd_application_id(pfcp_application_id_ie_t *app_id)
{
	//REVIEW: Remove this hardcoded value
	pfcp_set_ie_header(&app_id->header, PFCP_IE_APPLICATION_ID, 8);
	memcpy(app_id->app_ident, "_app_1  ", 8);

}

/**
 * @brief  : This function fills pfd app id and pfd context
 * @param  : app_id_pfds_t is pointer to structure of  ie
 * @param  : len denotes total length of ie
 * @return : This function dose not return anything
 */
static void
set_app_ids_pfds(pfcp_app_ids_pfds_ie_t *app_ids_pfds_t , uint16_t len)
{
	/* Fill app id */
	set_pfd_application_id(&app_ids_pfds_t->application_id);
	app_ids_pfds_t->pfd_context_count = 1;

	/* Fill pfd context */
	for(int i = 0; i < app_ids_pfds_t->pfd_context_count; ++i){
		set_pfd_context(&app_ids_pfds_t->pfd_context[i]);
		len = app_ids_pfds_t->pfd_context[i].header.len
			+ app_ids_pfds_t->application_id.header.len
			+ sizeof(pfcp_ie_header_t)
			+ sizeof(pfcp_ie_header_t);
	}
	/* set app id pfds header  */
	pfcp_set_ie_header(&app_ids_pfds_t->header, PFCP_IE_APP_IDS_PFDS, len);
}


void
fill_pfcp_pfd_mgmt_req(pfcp_pfd_mgmt_req_t *pfcp_pfd_req, uint16_t len)
{

	uint32_t seq  = 0;
	seq = get_pfcp_sequence_number(PFCP_PFD_MGMT_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_pfd_req->header),
			PFCP_PFD_MGMT_REQUEST, NO_SEID, seq);
	pfcp_pfd_req->app_ids_pfds_count = 1;

	for(int i=0; i < pfcp_pfd_req->app_ids_pfds_count; ++i){
		set_app_ids_pfds(&pfcp_pfd_req->app_ids_pfds[i], len);
	}
}

void
fill_pfcp_node_report_req(pfcp_node_rpt_req_t *pfcp_node_rep_req)
{
	uint32_t seq  = 1;
	char node_addr[INET_ADDRSTRLEN] = {0} ;
	memset(pfcp_node_rep_req, 0, sizeof(pfcp_node_rpt_req_t)) ;

	seq = get_pfcp_sequence_number(PFCP_NODE_REPORT_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_node_rep_req->header),
			PFCP_NODE_REPORT_REQUEST, NO_SEID, seq);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), node_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(node_addr);
	set_node_id(&(pfcp_node_rep_req->node_id), node_value);

	set_node_report_type(&(pfcp_node_rep_req->node_rpt_type));

	set_user_plane_path_failure_report(&(pfcp_node_rep_req->user_plane_path_fail_rpt));
}

void
fill_pfcp_heartbeat_req(pfcp_hrtbeat_req_t *pfcp_heartbeat_req, uint32_t seq)
{

	memset(pfcp_heartbeat_req, 0, sizeof(pfcp_hrtbeat_req_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_heartbeat_req->header),
			PFCP_HEARTBEAT_REQUEST,	NO_SEID, seq);

	set_recovery_time_stamp(&(pfcp_heartbeat_req->rcvry_time_stmp));
	seq++;
}


int process_pfcp_heartbeat_req(struct sockaddr_in *peer_addr, uint32_t seq)
{
	uint8_t pfcp_msg[250]={0};
	int encoded = 0;

	pfcp_hrtbeat_req_t pfcp_heartbeat_req  = {0};
	pfcp_hrtbeat_rsp_t *pfcp_hearbeat_resp = (pfcp_hrtbeat_rsp_t*)malloc(sizeof(pfcp_hrtbeat_rsp_t));

	memset(pfcp_hearbeat_resp,0,sizeof(pfcp_hrtbeat_rsp_t));
	fill_pfcp_heartbeat_req(&pfcp_heartbeat_req, seq);

	encoded = encode_pfcp_hrtbeat_req_t(&pfcp_heartbeat_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, peer_addr);
     
	return 0;

}
