// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "pfcp_up_util.h"
#include "pfcp_up_set_ie.h"
#include "pfcp_enum.h"
#include "pfcp_up_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "../cp_dp_api/vepc_cp_dp_api.h"
#include "clogger.h"
#include "up_main.h"
#include "gw_adapter.h"
#include "up_io_poll.h"

extern udp_sock_t my_sock;
struct in_addr cp_comm_ip;
uint16_t cp_comm_port;

void
fill_pfcp_association_release_resp(pfcp_assn_rel_rsp_t *pfcp_ass_rel_resp)
{
	/*take seq no from assoc release request when it is implemented*/
	uint32_t seq  = 1;
	memset(pfcp_ass_rel_resp, 0, sizeof(pfcp_assn_rel_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_rel_resp->header),
			PFCP_ASSOCIATION_RELEASE_RESPONSE, NO_SEID, seq);

	//TODO filling of node id
	const char* pAddr = "192.168.0.10";
	uint32_t node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_ass_rel_resp->node_id), node_value);

	// TODO : REmove the CAUSE_VALUES_REQUESTACCEPTEDSUCCESS in set_cause
	set_cause(&(pfcp_ass_rel_resp->cause), REQUESTACCEPTED);

}

void
fill_pfcp_association_setup_resp(pfcp_assn_setup_rsp_t *pfcp_ass_setup_resp,
				uint8_t cause )
{
	uint32_t seq  = 1;
	uint32_t node_value = 0;

	memset(pfcp_ass_setup_resp,0,sizeof(pfcp_assn_setup_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_setup_resp->header),
			PFCP_ASSOCIATION_SETUP_RESPONSE, NO_SEID, seq);

	set_node_id(&(pfcp_ass_setup_resp->node_id), node_value);

	set_cause(&(pfcp_ass_setup_resp->cause), cause);

	set_recovery_time_stamp(&(pfcp_ass_setup_resp->rcvry_time_stmp));

	/* As we are not supporting this feature
	set_upf_features(&(pfcp_ass_setup_resp->up_func_feat)); */

	if (app.spgw_cfg == SGWU) {
		pfcp_ass_setup_resp->user_plane_ip_rsrc_info_count = 2; /*for s1u and s5s8 sgwc ips*/
		/*UPF Features IE is added for the ENDMARKER feauture which is supported in SGWU only*/
		set_upf_features(&(pfcp_ass_setup_resp->up_func_feat));
		pfcp_ass_setup_resp->up_func_feat.sup_feat |=  EMPU ;
		pfcp_ass_setup_resp->header.message_len += pfcp_ass_setup_resp->up_func_feat.header.len;


	} else if ((app.spgw_cfg == PGWU) || (app.spgw_cfg == SAEGWU)) {
		pfcp_ass_setup_resp->user_plane_ip_rsrc_info_count = 1; /*for s5s8 pgwc ip*/
	}

	for( int i=0; i < pfcp_ass_setup_resp->user_plane_ip_rsrc_info_count; i++ ){
		set_up_ip_resource_info(&(pfcp_ass_setup_resp->user_plane_ip_rsrc_info[i]),i);
		/* Copy same teid_range in all user plane IP rsrc IEs */
		pfcp_ass_setup_resp->user_plane_ip_rsrc_info[i].teidri =
					pfcp_ass_setup_resp->user_plane_ip_rsrc_info[0].teidri;
		pfcp_ass_setup_resp->user_plane_ip_rsrc_info[i].teid_range =
					pfcp_ass_setup_resp->user_plane_ip_rsrc_info[0].teid_range;
		 pfcp_ass_setup_resp->header.message_len +=
			        pfcp_ass_setup_resp->user_plane_ip_rsrc_info[i].header.len;
	}

	pfcp_ass_setup_resp->header.message_len = pfcp_ass_setup_resp->node_id.header.len +
		pfcp_ass_setup_resp->rcvry_time_stmp.header.len +
		pfcp_ass_setup_resp->cause.header.len;


	pfcp_ass_setup_resp->header.message_len += sizeof(pfcp_ass_setup_resp->header.seid_seqno.no_seid);

}

/* Fill pfd mgmt response */
void
fill_pfcp_pfd_mgmt_resp(pfcp_pfd_mgmt_rsp_t *pfd_resp, uint8_t cause_val, int offending_id)
{
	memset(pfd_resp, 0, sizeof(pfcp_pfd_mgmt_rsp_t));

	set_pfcp_header(&pfd_resp->header, PFCP_PFD_MANAGEMENT_RESPONSE, 0);

	pfcp_set_ie_header(&pfd_resp->cause.header, PFCP_IE_CAUSE,
			sizeof(pfd_resp->cause.cause_value));
	pfd_resp->cause.cause_value = cause_val;

	pfcp_set_ie_header(&pfd_resp->offending_ie.header, PFCP_IE_OFFENDING_IE,
			sizeof(pfd_resp->offending_ie.type_of_the_offending_ie));
	pfd_resp->offending_ie.type_of_the_offending_ie = (uint16_t)offending_id;
}

void
fill_pfcp_association_update_resp(pfcp_assn_upd_rsp_t *pfcp_asso_update_resp)
{
	/*take seq no from assoc update request when it is implemented*/
	uint32_t seq  = 1;
	uint32_t node_value = 0;

	memset(pfcp_asso_update_resp, 0, sizeof(pfcp_assn_upd_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_asso_update_resp->header),
			PFCP_ASSOCIATION_UPDATE_RESPONSE, NO_SEID, seq);

	set_node_id(&(pfcp_asso_update_resp->node_id),node_value);

	// filling of cause
	// TODO : REmove the CAUSE_VALUES_REQUESTACCEPTEDSUCCESS in set_cause
	set_cause(&(pfcp_asso_update_resp->cause), REQUESTACCEPTED);

	set_upf_features(&(pfcp_asso_update_resp->up_func_feat));

}

void
fill_pfcp_node_report_resp(pfcp_node_rpt_rsp_t *pfcp_node_rep_resp)
{
	/*take seq no from node report request when it is implemented*/
	uint32_t seq  = 1;
	uint32_t node_value = 0;

	memset(pfcp_node_rep_resp, 0, sizeof(pfcp_node_rpt_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_node_rep_resp->header),
			PFCP_NODE_REPORT_RESPONSE, NO_SEID,seq);

	set_node_id(&(pfcp_node_rep_resp->node_id), node_value);

	//set cause
	// TODO : REmove the CAUSE_VALUES_REQUESTACCEPTEDSUCCESS in set_cause
	set_cause(&(pfcp_node_rep_resp->cause), REQUESTACCEPTED);

	//set offending ie
	//TODO: Remove NODE_ID with actual offend ID
	set_offending_ie(&(pfcp_node_rep_resp->offending_ie), PFCP_IE_NODE_ID);

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
void
fill_pfcp_heartbeat_resp(pfcp_hrtbeat_rsp_t *pfcp_heartbeat_resp)
{

	uint32_t seq  = 1;
	memset(pfcp_heartbeat_resp, 0, sizeof(pfcp_hrtbeat_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_heartbeat_resp->header),
			PFCP_HEARTBEAT_RESPONSE, NO_SEID, seq);

	set_recovery_time_stamp(&(pfcp_heartbeat_resp->rcvry_time_stmp));
}

int process_pfcp_heartbeat_req(struct sockaddr_in *peer_addr, uint32_t seq)
{
	uint8_t pfcp_msg[250]={0};
	int encoded = 0;

	pfcp_hrtbeat_req_t pfcp_heartbeat_req  = {0};
	pfcp_hrtbeat_rsp_t *pfcp_hearbeat_resp =
						malloc(sizeof(pfcp_hrtbeat_rsp_t));

	memset(pfcp_hearbeat_resp,0,sizeof(pfcp_hrtbeat_rsp_t));
	fill_pfcp_heartbeat_req(&pfcp_heartbeat_req, seq);

	encoded = encode_pfcp_hrtbeat_req_t(&pfcp_heartbeat_req, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, peer_addr) < 0 ) {
					clLog(clSystemLog, eCLSeverityDebug, "Error sending: %i\n",errno);
	}
	else
	{
		update_cli_stats(peer_addr->sin_addr.s_addr,
				PFCP_HEARTBEAT_REQUEST,SENT,SX);
	}

	return 0;

}
