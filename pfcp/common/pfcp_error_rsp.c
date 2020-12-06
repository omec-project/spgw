// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "pfcp_error_rsp.h"
#include "sm_struct.h"
#include "pfcp_messages.h"
#include "ue.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_proc.h"
#include "trans_struct.h"
#include "rte_common.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_cp_set_ie.h"
#include "clogger.h"
#include "cp_io_poll.h"
#include "spgw_cpp_wrapper.h"
#include "gw_adapter.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_interface.h"

extern uint8_t pfcp_tx_buf[MAX_GTPV2C_UDP_LEN];

void 
get_error_session_report_info(msg_info_t *msg, pfcp_err_rsp_info_t *rsp_info) 
{

	switch(msg->msg_type) {
        case PFCP_SESSION_REPORT_REQUEST: {
	        ue_context_t *context = NULL;
            pfcp_sess_rpt_req_t *pfcp_sess_rep_req = &msg->pfcp_msg.pfcp_sess_rep_req;
            rsp_info->seq = pfcp_sess_rep_req->header.seid_seqno.has_seid.seq_no;
            rsp_info->peer_addr = msg->peer_addr; 
            uint64_t sess_id = pfcp_sess_rep_req->header.seid_seqno.has_seid.seid;
            if (get_sess_entry_seid(sess_id, &context) != 0) {
                rsp_info->dp_seid = 0; 
            } else {
                uint8_t ebi_index = UE_BEAR_ID(sess_id) - 5; 
                pdn_connection_t *pdn = GET_PDN(context, ebi_index);
                rsp_info->dp_seid = pdn->dp_seid;
            }
            break;
        }

        // real -ve response from mme
        // DDN timeout 
		case GTP_DOWNLINK_DATA_NOTIFICATION_ACK: {
            proc_context_t *paging_proc = (proc_context_t *)msg->proc_context;
            pdn_connection_t *pdn = paging_proc->pdn_context;
            transData_t *pfcp_trans = paging_proc->pfcp_trans;
            rsp_info->peer_addr = pfcp_trans->peer_sockaddr; 
			rsp_info->dp_seid = pdn->dp_seid; 
			rsp_info->seq = pfcp_trans->sequence;
			break;

		}
        default:
            assert(0);
	}
}

void 
session_report_error_response(msg_info_t *msg, uint8_t cause_value, int iface)
{
    RTE_SET_USED(iface);
	pfcp_err_rsp_info_t rsp_info = {0};
    pfcp_sess_rpt_rsp_t pfcp_sess_rep_resp = {0};
	uint16_t encoded = 0;

	get_error_session_report_info(msg, &rsp_info); 

	bzero(&pfcp_tx_buf, sizeof(pfcp_tx_buf));


	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_rep_resp.header),
		PFCP_SESSION_REPORT_RESPONSE, HAS_SEID, rsp_info.seq);

	pfcp_sess_rep_resp.header.seid_seqno.has_seid.seid = rsp_info.dp_seid;

	set_cause(&(pfcp_sess_rep_resp.cause), cause_value);

	encoded = encode_pfcp_sess_rpt_rsp_t(&pfcp_sess_rep_resp,(uint8_t *)pfcp_tx_buf);
	pfcp_header_t *pfcp_hdr = (pfcp_header_t *) pfcp_tx_buf;
	pfcp_hdr->message_len = htons(encoded - 4);
	pfcp_send(my_sock.sock_fd_pfcp, pfcp_tx_buf, encoded, &rsp_info.peer_addr);
    increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSREPORTRSP_REJ, rsp_info.peer_addr.sin_addr.s_addr);
    return;
}
