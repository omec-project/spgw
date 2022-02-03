// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "cp_log.h"
#include "gtpv2_ie.h"
#include "ue.h"
#include "sm_struct.h"
#include "proc_struct.h"
#include "cp_peer.h"
#include "gtp_messages_decoder.h"
#include "spgw_cpp_wrapper.h"
#include "cp_io_poll.h"
#include "util.h"
#include "sm_structs_api.h"
#include "cp_transactions.h"
#include "gtpv2_interface.h"
#include "proc.h"

// saegw - PDN_GW_INIT_BEARER_DEACTIVATION  DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_resp_handler  
// saegw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT => process_delete_bearer_response_handler 
// pgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_resp_handler
// pgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT ==> process_delete_bearer_response_handler
// sgw - PDN_GW_INIT_BEARER_DEACTIVATION DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT : process_delete_bearer_resp_handler 
// sgw - MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC DELETE_BER_REQ_SNT_STATE DELETE_BER_RESP_RCVD_EVNT - process_delete_bearer_response_handler 
int 
handle_delete_bearer_response_msg(msg_info_t **msg_p, gtpv2c_header_t *gtpv2c_rx)
{
    int ret = 0;
    msg_info_t *msg = *msg_p;
    struct sockaddr_in *peer_addr = &msg->peer_addr;
    del_bearer_rsp_t *dbrsp = &msg->rx_msg.db_rsp;

    /* Reset periodic timers */
    process_response(peer_addr->sin_addr.s_addr);

    ret = decode_del_bearer_rsp((uint8_t *) gtpv2c_rx, dbrsp);
    if (!ret) {
        LOG_MSG(LOG_DEBUG0, "DBrsp decode failure ");
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DBRSP_DROP, peer_addr->sin_addr.s_addr);
        return ret;
    }

    ue_context_t *context = (ue_context_t *)get_ue_context(dbrsp->header.teid.has_teid.teid);
    if (context == NULL) {
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DBRSP_DROP, peer_addr->sin_addr.s_addr);
        LOG_MSG(LOG_DEBUG0, "Rcvd DBRsp, Context not found for TEID %u ", dbrsp->header.teid.has_teid.teid);
        return -1;
    }
 

    uint32_t seq_num = dbrsp->header.teid.has_teid.seq;
    uint32_t local_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.s11_sockaddr.sin_port;

    transData_t *gtpc_trans = (transData_t *)delete_gtp_transaction(local_addr, port_num, seq_num);
    if(gtpc_trans == NULL) {
        LOG_MSG(LOG_DEBUG3, "Unsolicitated DBRsp response seq = %d ", seq_num);
        LOG_MSG(LOG_DEBUG3, "Unsolicitated DBRsp response seq = %d ", ntohl(seq_num));
        increment_mme_peer_stats(MSG_RX_GTPV2_S11_DBRSP_DROP, peer_addr->sin_addr.s_addr);
        return -1;
    }
	stop_transaction_timer(gtpc_trans);

    proc_context_t *proc_context = (proc_context_t *)gtpc_trans->proc_context;

    if(context == NULL) {
        context = (ue_context_t*)proc_context->ue_context;
    } else {
        assert(proc_context->ue_context == context);
    }

    msg->proc_context = gtpc_trans->proc_context;
    msg->event = RCVD_GTP_DEL_BEARER_RSP;
    msg->proc  = proc_context->state;
    SET_PROC_MSG(proc_context, msg);
    // Note : important to note that we are holding on this msg now 
    *msg_p = NULL;

    LOG_MSG(LOG_DEBUG, "Callback called for "
            "Msg_Type:%s[%u], Teid:%u, "
            "Procedure:%s, Event:%s",
            gtp_type_str(msg->msg_type), msg->msg_type,
            dbrsp->header.teid.has_teid.teid,
            get_proc_string(msg->proc),
            get_event_string(msg->event));

    proc_context->handler(proc_context, msg);

    //process_delete_bearer_response_handler(msg, NULL);
    return 0;
}

#ifdef FUTURE_NEED
int
process_delete_bearer_resp_handler(void *data, void *unused_param)
{
	msg_info_t *msg = (msg_info_t *)data;

	if (msg->rx_msg.db_rsp.lbi.header.len != 0) {
		/* Delete Default Bearer. Send PFCP Session Deletion Request */
		process_pfcp_sess_del_request_delete_bearer_rsp(&msg->rx_msg.db_rsp);
	} else {
		/* Delete Dedicated Bearer. Send PFCP Session Modification Request */
		process_delete_bearer_resp(&msg->rx_msg.db_rsp , 0);
	}

    LOG_MSG(LOG_NEVER, "unused_param = %p, data = %p ", unused_param, data);

	return 0;
}

void 
process_delete_bearer_resp_pfcp_timeout(void *data)
{
    LOG_MSG(LOG_NEVER, "data = %p ", data);
    return;
}

int
process_delete_bearer_resp(del_bearer_rsp_t *db_rsp, uint8_t is_del_bearer_cmd)
{
	int ret;
	uint8_t ebi_index = 5;
	uint8_t bearer_cntr = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint8_t default_bearer_id = 0;
	eps_bearer_t *bearers[MAX_BEARERS];
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	context = (ue_context_t *)get_ue_context(db_rsp->header.teid.has_teid.teid);
	if (context == NULL) {
        LOG_MSG(LOG_NEVER, "is_del_bearer_cmd = %d", is_del_bearer_cmd);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	s11_mme_sockaddr.sin_addr.s_addr =
		context->s11_mme_gtpc_ipv4.s_addr;

	if (db_rsp->lbi.header.len) {
		default_bearer_id = db_rsp->lbi.ebi_ebi;
		pdn = context->pdns[default_bearer_id];
		bearers[default_bearer_id - 5] = context->eps_bearers[default_bearer_id - 5];
		bearer_cntr = 1;
	} else {
		for (uint8_t iCnt = 0; iCnt < db_rsp->bearer_count; ++iCnt) {
			ebi_index = db_rsp->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
			bearers[iCnt] = context->eps_bearers[ebi_index - 5];
		}
		pdn = context->eps_bearers[ebi_index - 5]->pdn;
		bearer_cntr = db_rsp->bearer_count;

	}

	fill_pfcp_sess_mod_req_pgw_init_remove_pdr(&pfcp_sess_mod_req, pdn, bearers, bearer_cntr);

	uint8_t pfcp_msg[512]={0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &context->upf_context->upf_sockaddr);

    increment_userplane_stats(MSG_RX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(context->upf_context));
    transData_t *trans_entry;
	trans_entry = start_response_wait_timer(context, pfcp_msg, encoded, process_delete_bearer_resp_pfcp_timeout);
    pdn->trans_entry = trans_entry;

#ifdef FUTURE_NEED
	context->sequence = db_rsp->header.teid.has_teid.seq;
#endif
	pdn->state = PFCP_SESS_MOD_REQ_SNT_STATE;

#if 0
	resp = get_sess_entry_seid(pdn->seid);
	if (resp == NULL) {
		LOG_MSG(LOG_ERROR,
			"Failed to add response in entry in SM_HASH");
		return -1;
	}

	if (db_rsp->lbi.header.len != 0) {
		resp->linked_eps_bearer_id = db_rsp->lbi.ebi_ebi;
		resp->bearer_count = 0;
	} else {
		resp->bearer_count = db_rsp->bearer_count;
		for (uint8_t iCnt = 0; iCnt < db_rsp->bearer_count; ++iCnt) {
			resp->eps_bearer_ids[iCnt] = db_rsp->bearer_contexts[iCnt].eps_bearer_id.ebi_ebi;
		}
	}
	if(is_del_bearer_cmd == 0){
		resp->msg_type = GTP_DELETE_BEARER_RSP;
		resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
		resp->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		pdn->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
	}else{

		resp->msg_type = GTP_DELETE_BEARER_RSP;
		resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;
		resp->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
		pdn->proc = MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC;
	}
#endif

	return 0;
}
#endif
