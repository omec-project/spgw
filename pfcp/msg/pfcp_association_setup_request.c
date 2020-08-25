// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp_ies.h"
#include "pfcp_cp_interface.h"
#include "gw_adapter.h"
#include "pfcp_messages_decoder.h"
#include "pfcp_messages_encoder.h"
#include "clogger.h"
#include "pfcp_association_setup_proc.h"
#include "sm_struct.h"
#include "gw_adapter.h"
#include "upf_struct.h"
#include "netinet/in.h"
#include "pfcp_cp_set_ie.h"
#include "cp_config.h"
#include "pfcp_enum.h"
#include "tables/tables.h"
#include "cp_peer.h"
#include "pfcp_cp_util.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"

static void
fill_pfcp_association_setup_rsp(pfcp_assn_setup_rsp_t *pfcp_ass_setup_rsp, uint32_t seq)
{
	char node_addr[INET_ADDRSTRLEN] = {0};

	memset(pfcp_ass_setup_rsp, 0, sizeof(pfcp_assn_setup_rsp_t)) ;

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_ass_setup_rsp->header),
			PFCP_ASSOCIATION_SETUP_RESPONSE, NO_SEID, seq);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), node_addr, INET_ADDRSTRLEN);

	unsigned long node_value = inet_addr(node_addr);
	set_node_id(&(pfcp_ass_setup_rsp->node_id), node_value);

	set_recovery_time_stamp(&(pfcp_ass_setup_rsp->rcvry_time_stmp));

	set_cause(&(pfcp_ass_setup_rsp->cause), REQUESTACCEPTED);

    return;
}

int 
handle_pfcp_association_setup_request_msg(msg_info_t *msg, pfcp_header_t *pfcp_rx)
{
    struct sockaddr_in *peer_addr = &msg->peer_addr;
 
    /*Decode the received msg and stored into the struct. */
    int decoded = decode_pfcp_assn_setup_req_t((uint8_t *)pfcp_rx,
            &msg->pfcp_msg.pfcp_ass_req);

    clLog(sxlogger, eCLSeverityDebug, "Decoded bytes [%d]\n", decoded);
    if(decoded <= 0) 
    {
        clLog(clSystemLog, eCLSeverityCritical, "%s: Failed to process pfcp precondition check\n", __func__);

        increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }
    increment_userplane_stats(MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ, peer_addr->sin_addr.s_addr);

    uint32_t seq_num = msg->pfcp_msg.pfcp_ass_req.header.seid_seqno.no_seid.seq_no; 

    upf_context_t *upf_context = NULL; 
    upf_context_entry_lookup(peer_addr->sin_addr.s_addr, &upf_context);
    if(upf_context == NULL) {
        clLog(clSystemLog, eCLSeverityCritical, "Received PFCP association setup request from UPF %s ",inet_ntoa(peer_addr->sin_addr));
        create_upf_context(peer_addr->sin_addr.s_addr, &upf_context);
    }
    upf_context->state = PFCP_ASSOC_RESP_RCVD_STATE;

    upf_context->up_supp_features =
        msg->pfcp_msg.pfcp_ass_req.up_func_feat.sup_feat;

    switch (cp_config->cp_type)
    {
        case SGWC :
            if (msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].ipv4_address;

            if( msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[1].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[1].src_intfc ==
                    SOURCE_INTERFACE_VALUE_CORE )
                upf_context->s5s8_sgwu_ip =
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[1].ipv4_address;
            break;

        case PGWC :
            if (msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s5s8_pgwu_ip =
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

        case SAEGWC :
            if( msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].assosi == 1 &&
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].src_intfc ==
                    SOURCE_INTERFACE_VALUE_ACCESS )
                upf_context->s1u_ip =
                    msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].ipv4_address;
            break;

    }

    /* teid_range from first user plane ip IE is used since, for same CP ,
     * DP will assigne single teid_range , So all IE's will have same value for teid_range*/
    /* Change teid base address here */
    if(msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].teidri != 0){
        /* Requirement : This data should go in the upf context */
        set_base_teid(msg->pfcp_msg.pfcp_ass_req.user_plane_ip_rsrc_info[0].teid_range);
    }


    /* Adding ip to cp  heartbeat when dp returns the association response*/
    add_ip_to_heartbeat_hash(peer_addr,
            msg->pfcp_msg.pfcp_ass_req.rcvry_time_stmp.rcvry_time_stmp_val);

    if ((add_node_conn_entry((uint32_t)peer_addr->sin_addr.s_addr,
                    SX_PORT_ID)) != 0) {

        clLog(clSystemLog, eCLSeverityCritical, "Failed to add connection entry for SGWU/SAEGWU");
    }
    pfcp_assn_setup_rsp_t pfcp_ass_setup_rsp = {0};
    fill_pfcp_association_setup_rsp(&pfcp_ass_setup_rsp, seq_num);

    uint8_t pfcp_msg[256] = {0};
    int encoded = encode_pfcp_assn_setup_rsp_t(&pfcp_ass_setup_rsp, pfcp_msg);

    pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
    header->message_len = htons(encoded - 4);

    if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &upf_context->upf_sockaddr) < 0 ) {
        clLog(clSystemLog, eCLSeverityDebug,"Error sending\n\n");
        return -1;
    } else {
        increment_userplane_stats(MSG_TX_PFCP_SXASXB_ASSOCSETUPRSP, GET_UPF_ADDR(upf_context));
        upf_context->state = PFCP_ASSOC_RESP_RCVD_STATE;
    }
    return 0;
}

