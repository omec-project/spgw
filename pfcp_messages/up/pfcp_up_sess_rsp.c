// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "up_main.h"
#include "epc_arp.h"
#include "pfcp_enum.h"
#include "pfcp_up_set_ie.h"
#include "clogger.h"

extern struct rte_hash *arp_hash_handle[NUM_SPGW_PORTS];

void
fill_pfcp_session_est_resp(pfcp_sess_estab_rsp_t *pfcp_sess_est_resp,
			uint8_t cause, int offend, struct in_addr dp_comm_ip,
			pfcp_sess_estab_req_t *pfcp_session_request)
{
	/*take seq no from sess establishment request when this function is called somewhere*/
	uint32_t seq  = 0;
	uint32_t node_value = 0;

	//memset(pfcp_sess_est_resp, 0, sizeof(pfcp_sess_estab_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_est_resp->header),
			PFCP_SESSION_ESTABLISHMENT_RESPONSE, HAS_SEID, seq);

	set_node_id(&(pfcp_sess_est_resp->node_id), dp_comm_ip.s_addr);
	set_cause(&(pfcp_sess_est_resp->cause), cause);

	if(cause == CONDITIONALIEMISSING || cause == MANDATORYIEMISSING) {
		set_offending_ie(&(pfcp_sess_est_resp->offending_ie), offend);
	}

	if(REQUESTACCEPTED == cause) {
		uint64_t up_seid = pfcp_session_request->header.seid_seqno.has_seid.seid;;
		set_fseid(&(pfcp_sess_est_resp->up_fseid), up_seid, node_value);
	}

	if(pfcp_ctxt.cp_supported_features & CP_LOAD) {
		set_lci(&(pfcp_sess_est_resp->load_ctl_info));
	}

	if(pfcp_ctxt.cp_supported_features & CP_OVRL) {
		set_olci(&(pfcp_sess_est_resp->ovrld_ctl_info));
	}

	/* TODO: Need to add condition for below
	char sgwu_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(dp_comm_ip), sgwu_addr, INET_ADDRSTRLEN);
	unsigned long sgwu_value = inet_addr(sgwu_addr);
	set_fq_csid( &(pfcp_sess_est_resp->sgw_u_fqcsid), sgwu_value);

	char pgwu_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(dp_comm_ip), pgwu_addr, INET_ADDRSTRLEN);
	unsigned long pgwu_value = inet_addr(pgwu_addr);
	set_fq_csid( &(pfcp_sess_est_resp->pgw_u_fqcsid), pgwu_value); */


	if(RULECREATION_MODIFICATIONFAILURE == cause) {
		set_failed_rule_id(&(pfcp_sess_est_resp->failed_rule_id));
	}
}

void
fill_pfcp_session_modify_resp(pfcp_sess_mod_rsp_t *pfcp_sess_modify_resp,
		pfcp_sess_mod_req_t *pfcp_session_mod_req, uint8_t cause, int offend)
{
	uint32_t seq  = 1;
	memset(pfcp_sess_modify_resp, 0, sizeof(pfcp_sess_mod_rsp_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_modify_resp->header),
			PFCP_SESSION_MODIFICATION_RESPONSE, HAS_SEID, seq);

	set_cause(&(pfcp_sess_modify_resp->cause), cause);

	if(cause == CONDITIONALIEMISSING
			|| cause == MANDATORYIEMISSING) {
		set_offending_ie(&(pfcp_sess_modify_resp->offending_ie), offend);
	}

	//created_bar
	// Need to do
	if(cause == REQUESTACCEPTED){
		if(pfcp_session_mod_req->create_pdr_count > 0 &&
				pfcp_session_mod_req->create_pdr[0].pdi.local_fteid.ch){
			set_created_pdr_ie(&(pfcp_sess_modify_resp->created_pdr));
		}
	}

	if( pfcp_ctxt.cp_supported_features & CP_LOAD )
		set_lci(&(pfcp_sess_modify_resp->load_ctl_info));

	if( pfcp_ctxt.cp_supported_features & CP_OVRL )
		set_olci(&(pfcp_sess_modify_resp->ovrld_ctl_info));

	if(cause == RULECREATION_MODIFICATIONFAILURE){
		set_failed_rule_id(&(pfcp_sess_modify_resp->failed_rule_id));
	}

	// filling of ADURI
	// Need to do
	if(pfcp_session_mod_req->pfcpsmreq_flags.qaurr ||
			pfcp_session_mod_req->query_urr_count){
		set_additional_usage(&(pfcp_sess_modify_resp->add_usage_rpts_info));
	}

	// filling of CRTEP
	// Need to do
	if( pfcp_ctxt.up_supported_features & UP_PDIU )
		set_created_traffic_endpoint(&(pfcp_sess_modify_resp->createdupdated_traffic_endpt));

}

void
fill_pfcp_sess_del_resp(pfcp_sess_del_rsp_t *
		pfcp_sess_del_resp, uint8_t cause, int offend)
{

	uint32_t seq  = 1;
	memset(pfcp_sess_del_resp, 0, sizeof(pfcp_sess_del_rsp_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_del_resp->header), PFCP_SESSION_DELETION_RESPONSE,
			HAS_SEID, seq);

	set_cause(&(pfcp_sess_del_resp->cause), cause);

	if(cause == CONDITIONALIEMISSING ||
			cause == MANDATORYIEMISSING) {

		set_offending_ie(&(pfcp_sess_del_resp->offending_ie), offend);
	}

	if( pfcp_ctxt.cp_supported_features & CP_LOAD )
		set_lci(&(pfcp_sess_del_resp->load_ctl_info));

	if( pfcp_ctxt.cp_supported_features & CP_OVRL )
		set_olci(&(pfcp_sess_del_resp->ovrld_ctl_info));
}

int sess_modify_with_endmarker(far_info_t *far)
{
	struct sess_info_endmark edmk;
	struct arp_entry_data *ret_arp_data = NULL;
	int ret = 0;

	/*Retrieve the destination MAC*/
	edmk.dst_ip = htonl(far->frwdng_parms.outer_hdr_creation.ipv4_address);
	edmk.teid = far->frwdng_parms.outer_hdr_creation.teid;

	ret = rte_hash_lookup_data(arp_hash_handle[S1U_PORT_ID],
			&edmk.dst_ip, (void **)&ret_arp_data);

	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityDebug, "IP is not resolved for sending endmarker:%u\n",
				edmk.dst_ip);
		return -1;
	}

	memcpy(&(edmk.destination_MAC), &(ret_arp_data->eth_addr) , sizeof(struct ether_addr));
	edmk.dst_port = ret_arp_data->port;
	edmk.source_MAC = app.s1u_ether_addr;
	edmk.src_ip = app.s1u_ip;
	build_endmarker_and_send(&edmk);
	return 0;
}
