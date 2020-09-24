// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "gtp_ies.h"
#include "gtpv2_error_rsp.h"
#include "cp_config.h"
#include "ipc_api.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages_encoder.h"
#include "gtp_messages_encoder.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "clogger.h"
#include "gw_adapter.h"
#include "sm_structs_api.h"
#include "gx_error_rsp.h"
#include "pfcp.h"
#include "cp_transactions.h"
#include "spgw_cpp_wrapper.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

void
clean_up_while_error_pfcp_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

void get_error_csrsp_info(msg_info_t *msg, err_rsp_info *rsp_info) 
{

	switch(msg->msg_type) {
		case GTP_CREATE_SESSION_REQ: {
			rsp_info->sender_teid = msg->gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key;
			rsp_info->seq = msg->gtpc_msg.csr.header.teid.has_teid.seq;
			rsp_info->ebi_index = msg->gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid =  msg->gtpc_msg.csr.header.teid.has_teid.teid;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.header.len)
				rsp_info->offending = GTP_IE_CREATE_SESS_REQUEST_BEARER_CTXT_TO_BE_CREATED;

			if (!msg->gtpc_msg.csr.sender_fteid_ctl_plane.header.len)
				rsp_info->offending = GTP_IE_FULLY_QUAL_TUNN_ENDPT_IDNT;

			if (!msg->gtpc_msg.csr.imsi.header.len)
				rsp_info->offending = GTP_IE_IMSI;

			if (!msg->gtpc_msg.csr.apn_ambr.header.len)
				rsp_info->offending = GTP_IE_AGG_MAX_BIT_RATE;

			if (!msg->gtpc_msg.csr.pdn_type.header.len)
					rsp_info->offending = GTP_IE_PDN_TYPE;

			if (!msg->gtpc_msg.csr.bearer_contexts_to_be_created.bearer_lvl_qos.header.len)
				rsp_info->offending = GTP_IE_BEARER_QLTY_OF_SVC;

			if (!msg->gtpc_msg.csr.rat_type.header.len)
				rsp_info->offending = GTP_IE_RAT_TYPE;

			if (!msg->gtpc_msg.csr.apn.header.len)
				rsp_info->offending = GTP_IE_ACC_PT_NAME;

			break;
		}

		case PFCP_ASSOCIATION_SETUP_RESPONSE: {
            proc_context_t *csreq_proc = (proc_context_t *)msg->proc_context;
            ue_context_t  *ue = (ue_context_t *)csreq_proc->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t *)csreq_proc->pdn_context;
            transData_t *gtpc_trans = csreq_proc->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 

			rsp_info->sender_teid = ue->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence; 
			rsp_info->ebi_index = pdn->default_bearer_id;
			rsp_info->teid = ue->s11_sgw_gtpc_teid;
			break;
		}

		case PFCP_SESSION_ESTABLISHMENT_RESPONSE: {
            proc_context_t *csreq_proc = (proc_context_t *)msg->proc_context;
            pdn_connection_t *pdn = (pdn_connection_t *)csreq_proc->pdn_context;
	        ue_context_t *context = pdn->context;
            transData_t *gtpc_trans = csreq_proc->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence; 
		    rsp_info->ebi_index = pdn->default_bearer_id;
			break;
		}

		case GTP_CREATE_SESSION_RSP:{
#ifdef FUTURE_NEED
			if (get_ue_context_while_error(msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid, &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			if(msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi)
				rsp_info->ebi_index = msg->gtpc_msg.cs_rsp.bearer_contexts_created.eps_bearer_id.ebi_ebi;
			rsp_info->teid = msg->gtpc_msg.cs_rsp.header.teid.has_teid.teid;
#endif
			break;
		}
		case GTP_MODIFY_BEARER_RSP: {
#ifdef FUTURE_NEED
			rsp_info->seq = msg->gtpc_msg.mb_rsp.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi;

			if (get_ue_context_while_error(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0){
							clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);

			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
#endif
			break;
		}

		case GX_CCA_MSG: {
#ifdef FUTURE_NEED
			if(parse_gx_cca_msg(&msg->gx_msg.cca, &pdn) < 0) {
				return;
			}
			if(pdn != NULL && pdn->context != NULL ) {
				context = pdn->context;
				rsp_info->ebi_index = pdn->default_bearer_id;
				rsp_info->sender_teid = context->s11_mme_gtpc_teid;
				rsp_info->seq = context->sequence;
				rsp_info->teid = context->s11_sgw_gtpc_teid;
			}
#endif
			break;
		}
        default: {
            proc_context_t *csreq_proc = (proc_context_t *)msg->proc_context;
            ue_context_t  *ue = (ue_context_t *)csreq_proc->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t *)csreq_proc->pdn_context;
            transData_t *gtpc_trans = csreq_proc->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 
			rsp_info->sender_teid = ue->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence; 
			rsp_info->ebi_index = pdn->default_bearer_id;
			rsp_info->teid = ue->s11_sgw_gtpc_teid;
			break;
		}
	}
}
/* Requirement : msg should have upf_context set before calling this API.
 * If there are multiple pending CSRsp then caller should do looping part  
*/
void cs_error_response(msg_info_t *msg, uint8_t cause_value, int iface) 
{
    int ret = 0;
    uint16_t payload_length;

    create_sess_rsp_t cs_resp = {0};
    err_rsp_info rsp_info = {0};

    get_error_csrsp_info(msg, &rsp_info); 

    assert(msg->peer_addr.sin_port != 0);

    // Sending CCR-T in case of failure
    /* we should check if subscriber has gx session..this does not look good */
    if ((cp_config->gx_enabled) &&  (cp_config->cp_type != SGWC) && (msg->msg_type != GX_CCAI_FAILED)){
        send_ccr_t_req(msg, rsp_info.ebi_index, rsp_info.teid);
        struct sockaddr_in saddr_in;
        saddr_in.sin_family = AF_INET;
        inet_aton("127.0.0.1", &(saddr_in.sin_addr));
        increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T,saddr_in.sin_addr.s_addr);
    }
    bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));

    gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;

    set_gtpv2c_teid_header(&cs_resp.header,
            GTP_CREATE_SESSION_RSP,
            rsp_info.sender_teid,
            rsp_info.seq);

    if(cause_value == GTPV2C_CAUSE_MANDATORY_IE_MISSING ){
        set_ie_header(&cs_resp.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
                sizeof(struct cause_ie));
        cs_resp.cause.offend_ie_type = rsp_info.offending;
        cs_resp.cause.offend_ie_len = 0;
    }
    else{
        set_ie_header(&cs_resp.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
                sizeof(struct cause_ie_hdr_t));
    }
    cs_resp.cause.cause_value = cause_value;
    cs_resp.cause.pce = 0;
    cs_resp.cause.bce = 0;
    cs_resp.cause.spareinstance = 0;
    if(cp_config->cp_type != SGWC || cp_config->cp_type !=SAEGWC )
        cs_resp.cause.cs = 1;
    else
        cs_resp.cause.cs = 0;

    set_ie_header(&cs_resp.bearer_contexts_created.header, GTP_IE_BEARER_CONTEXT,
            IE_INSTANCE_ZERO, 0);


    set_ebi(&cs_resp.bearer_contexts_created.eps_bearer_id, IE_INSTANCE_ZERO,
            rsp_info.ebi_index);

    cs_resp.bearer_contexts_created.header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

    set_cause_error_value(&cs_resp.bearer_contexts_created.cause, IE_INSTANCE_ZERO, cause_value);

    cs_resp.bearer_contexts_created.header.len += sizeof(struct cause_ie_hdr_t) + IE_HEADER_SIZE;

    uint16_t msg_len = 0;
    msg_len = encode_create_sess_rsp(&cs_resp,(uint8_t *)gtpv2c_tx);
    gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

    payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

#ifdef TEMP
    // error handling should not be part of this function . 
    // caller should do it after sending CSRsp 
    ret = clean_up_while_error(rsp_info.ebi_index,
            rsp_info.teid,&msg->gtpc_msg.csr.imsi.imsi_number_digits,
            msg->gtpc_msg.csr.imsi.header.len, rsp_info.seq);
#endif

    
    if(iface == S11_IFACE){
        gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
                (struct sockaddr *) &msg->peer_addr, sizeof(struct sockaddr_in));
        increment_mme_peer_stats(MSG_TX_GTPV2_S11_CSRSP,msg->peer_addr.sin_addr.s_addr);
    }else{
        gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
                (struct sockaddr *)(&my_sock.s5s8_recv_sockaddr),
		        sizeof(struct sockaddr_in));

        increment_mme_peer_stats(MSG_TX_GTPV2_S5S8_CSRSP,msg->peer_addr.sin_addr.s_addr);
    }

    /* Cleanup transaction and cross references  */
    transData_t *gtpc_trans = (transData_t *)delete_gtp_transaction(msg->peer_addr.sin_addr.s_addr,         
                                        msg->peer_addr.sin_port,
                                        rsp_info.seq);

    assert(gtpc_trans != NULL);
    gtpc_trans->cb_data = NULL;
    proc_context_t *trans_proc = gtpc_trans->proc_context; 
    proc_context_t *msg_proc = msg->proc_context; 
    assert(trans_proc == msg_proc);
    msg_proc->gtpc_trans = NULL;
    free(gtpc_trans);
    RTE_SET_USED(ret);
}

void get_error_mbrsp_info(msg_info_t *msg, err_rsp_info *rsp_info) 
{
	switch(msg->msg_type) {
		case GTP_MODIFY_BEARER_REQ:{
            proc_context_t *proc_context = (proc_context_t *)msg->proc_context;
            if(proc_context == NULL)
            {
			    rsp_info->seq = msg->gtpc_msg.mbr.header.teid.has_teid.seq;
			    rsp_info->ebi_index = msg->gtpc_msg.mbr.bearer_contexts_to_be_modified.eps_bearer_id.ebi_ebi;
                return;
            }
            ue_context_t  *ue_record = (ue_context_t *)proc_context->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
            transData_t *trans_rec = proc_context->gtpc_trans;
			rsp_info->seq = trans_rec->sequence; 
			rsp_info->ebi_index = pdn->default_bearer_id; 
			rsp_info->sender_teid = ue_record->s11_mme_gtpc_teid;
			break;
		}

		case GTP_MODIFY_BEARER_RSP: {
#ifdef FUTURE_NEED
	        ue_context_t *context = NULL;
			rsp_info->seq = msg->gtpc_msg.mb_rsp.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid;
			rsp_info->ebi_index = msg->gtpc_msg.mb_rsp.bearer_contexts_modified.eps_bearer_id.ebi_ebi;

			if (get_ue_context_while_error(msg->gtpc_msg.mb_rsp.header.teid.has_teid.teid, &context) != 0){
							clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);

			}

			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
#endif
			break;
		}
		case PFCP_SESSION_MODIFICATION_RESPONSE: {
            proc_context_t *proc_context = (proc_context_t *)msg->proc_context;
            ue_context_t  *ue_context = (ue_context_t *)proc_context->ue_context; 
            pdn_connection_t *pdn = (pdn_connection_t *)proc_context->pdn_context;
            transData_t *gtpc_trans = proc_context->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 
 
			rsp_info->sender_teid = ue_context->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence; 
		    rsp_info->ebi_index = pdn->default_bearer_id;
			break;
		}
		case GX_CCA_MSG: {
#ifdef FUTURE_NEED
			if(parse_gx_cca_msg(&msg->gx_msg.cca, &pdn) < 0) {
				return;
			}
			if(pdn != NULL && pdn->context != NULL ) {
				context = pdn->context;
				rsp_info->ebi_index = pdn->default_bearer_id;
				rsp_info->sender_teid = context->s11_mme_gtpc_teid;
				rsp_info->seq = context->sequence;
				rsp_info->teid = context->s11_sgw_gtpc_teid;
			}
#endif
            assert(0);
			break;
		}
        default:
            assert(0);

	}
}

void mbr_error_response(msg_info_t *msg, uint8_t cause_value, int iface)
{
    uint16_t payload_length;

	err_rsp_info rsp_info = {0};

	get_error_mbrsp_info(msg, &rsp_info); // mbrsp 

    assert(msg->peer_addr.sin_port != 0);

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;

	mod_bearer_rsp_t mb_resp = {0};
	set_gtpv2c_teid_header(&mb_resp.header,
			GTP_MODIFY_BEARER_RSP,
			rsp_info.sender_teid,
			rsp_info.seq);

	set_cause_error_value(&mb_resp.cause, IE_INSTANCE_ZERO, cause_value);

	set_ie_header(&mb_resp.bearer_contexts_modified.header,
			GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO, 0);

	set_cause_error_value(&mb_resp.bearer_contexts_modified.cause,
			IE_INSTANCE_ZERO,cause_value);


	mb_resp.bearer_contexts_modified.header.len += sizeof(struct cause_ie_hdr_t) +
													IE_HEADER_SIZE;

	set_ebi(&mb_resp.bearer_contexts_modified.eps_bearer_id, IE_INSTANCE_ZERO,
			rsp_info.ebi_index);

	 mb_resp.bearer_contexts_modified.header.len += sizeof(uint8_t)+ IE_HEADER_SIZE;

	ue_context_t *context = NULL;

	if (get_ue_context(rsp_info.teid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
	}

	uint16_t msg_len = 0;
	msg_len = encode_mod_bearer_rsp(&mb_resp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

	if(iface == S11_IFACE){
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &msg->peer_addr, sizeof(struct sockaddr_in));

        increment_mme_peer_stats(MSG_TX_GTPV2_S11_MBRSP, msg->peer_addr.sin_addr.s_addr);
	}else{
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *)(&my_sock.s5s8_recv_sockaddr), 
                sizeof(struct sockaddr_in));

        increment_mme_peer_stats(MSG_TX_GTPV2_S5S8_MBRSP, msg->peer_addr.sin_addr.s_addr);
	}
}

void get_error_dsrsp_info(proc_context_t *detach_proc, msg_info_t *msg, err_rsp_info *rsp_info) 
{
	ue_context_t *context = NULL;

	switch(msg->msg_type) {
		case GTP_DELETE_SESSION_REQ: {
            // immediate rejection of dsreq ( validation of msg, ....)
			rsp_info->seq = msg->gtpc_msg.dsr.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.dsr.header.teid.has_teid.teid;

			if(get_ue_context(msg->gtpc_msg.dsr.header.teid.has_teid.teid,
							&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_DELETION_RESPONSE: {
            // pfcp timeout OR user plane rejection 
            ue_context_t *ue_context = detach_proc->ue_context;
            transData_t *gtpc_trans = detach_proc->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 
			rsp_info->sender_teid = ue_context->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence;
			break;

		}

		case GTP_DELETE_SESSION_RSP: {
            // in case of SGW mode -- DSRsp from pgw  
			if(get_ue_context_while_error(msg->gtpc_msg.ds_rsp.header.teid.has_teid.teid, &context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
            transData_t *gtpc_trans = detach_proc->gtpc_trans;
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence;
			break;
		}
        default:
            assert(0);
	}
}

void 
ds_error_response(proc_context_t *ds_proc, msg_info_t *msg, uint8_t cause_value, int iface)
{
	del_sess_rsp_t ds_resp = {0};
	uint16_t msg_len = 0;
    uint16_t payload_length;
	err_rsp_info rsp_info = {0};

	get_error_dsrsp_info(ds_proc, msg, &rsp_info); // dsrsp

    assert(msg->peer_addr.sin_port != 0);

#ifdef FUTURE_NEED
	uint8_t eps_bearer_id = 0;
    /* we should check if subscriber has gx session..this does not look good */
    if ((cp_config->gx_enabled) &&  
            (cp_config->cp_type != SGWC)){
        send_ccr_t_req(msg, eps_bearer_id, rsp_info.teid);
        struct sockaddr_in saddr_in;
		saddr_in.sin_family = AF_INET;
		inet_aton("127.0.0.1", &(saddr_in.sin_addr));
        increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T,saddr_in.sin_addr.s_addr);
	}
#endif

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;


    set_gtpv2c_teid_header(&ds_resp.header,
                           GTP_DELETE_SESSION_RSP,
                           rsp_info.sender_teid,
                           rsp_info.seq);

	set_cause_error_value(&ds_resp.cause, IE_INSTANCE_ZERO, cause_value);

	msg_len = encode_del_sess_rsp(&ds_resp,(uint8_t *)gtpv2c_tx);
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;
	header->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);

	if(iface == S11_IFACE){
		gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
				(struct sockaddr *) &msg->peer_addr, sizeof(struct sockaddr_in));
        increment_mme_peer_stats(MSG_TX_GTPV2_S11_DSRSP, msg->peer_addr.sin_addr.s_addr);
	}else{
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *)(&my_sock.s5s8_recv_sockaddr), 
		        sizeof(struct sockaddr_in));

        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_DSRSP, msg->peer_addr.sin_addr.s_addr);
	}

}

void get_error_rabrsp_info(msg_info_t *msg, err_rsp_info *rsp_info) 
{
	ue_context_t *context = NULL;

	switch(msg->msg_type) {
		case GTP_RELEASE_ACCESS_BEARERS_REQ: {
			rsp_info->seq = msg->gtpc_msg.rab.header.teid.has_teid.seq;
			rsp_info->teid = msg->gtpc_msg.rab.header.teid.has_teid.teid;

			if(get_ue_context(msg->gtpc_msg.rab.header.teid.has_teid.teid,
							&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			break;
		}

		case PFCP_SESSION_MODIFICATION_RESPONSE: {
            proc_context_t *rab_proc = (proc_context_t *)msg->proc_context;
            ue_context_t *ue_context = rab_proc->ue_context;
            transData_t *gtpc_trans = rab_proc->gtpc_trans;
            msg->peer_addr = gtpc_trans->peer_sockaddr; 
			rsp_info->sender_teid = ue_context->s11_mme_gtpc_teid;
			rsp_info->seq = gtpc_trans->sequence;
			break;

		}
        default:
            assert(0);
	}
}

void rab_error_response(msg_info_t *msg, uint8_t cause_value, int iface)
{
    uint16_t payload_length;

    RTE_SET_USED(iface);
	err_rsp_info rsp_info = {0};

	get_error_rabrsp_info(msg, &rsp_info); // rab-rsp

    assert(msg->peer_addr.sin_port != 0);

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;
    rel_acc_bearer_rsp_t rab_rsp = {0};

	set_gtpv2c_teid_header(&rab_rsp.header, GTP_RELEASE_ACCESS_BEARERS_RSP,
	    htonl(rsp_info.sender_teid), rsp_info.seq);

	set_cause_error_value(&rab_rsp.cause, IE_INSTANCE_ZERO, cause_value);

	uint16_t msg_len = 0;
	msg_len = encode_rel_acc_bearer_rsp(&rab_rsp,(uint8_t *)gtpv2c_tx);
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;
	header->gtpc.message_len = htons(msg_len - 4);

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
#ifdef FUTURE_NEED
	ue_context_t *context = NULL;
	uint8_t eps_bearer_id = 0;
	int ret = 0;
	ret = clean_up_while_error(eps_bearer_id, rsp_info.teid, &context->imsi, context->imsi_len, rsp_info.seq);
	if( ret < 0 ) {
		return;
	}
#endif
    gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
            (struct sockaddr *) &msg->peer_addr, sizeof(struct sockaddr_in));
    increment_mme_peer_stats(MSG_TX_GTPV2_S11_RABRSP, msg->peer_addr.sin_addr.s_addr);
}

#ifdef UPDATE_BEARER_SUPPORT
void get_error_ubrsp_info(msg_info_t *msg, err_rsp_info *rsp_info) 
{

	int ret = 0;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;

	switch(msg->msg_type) {
		case PFCP_SESSION_MODIFICATION_RESPONSE: {

			if (get_ue_context(UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid), &context) != 0){
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			rsp_info->sender_teid = context->s11_mme_gtpc_teid;
			rsp_info->seq = context->sequence;
			rsp_info->teid = UE_SESS_ID(msg->pfcp_msg.pfcp_sess_mod_resp.header.seid_seqno.has_seid.seid);
			if(resp)
				rsp_info->ebi_index = resp->eps_bearer_id;
			break;
		}

		case GTP_UPDATE_BEARER_REQ :{
#ifdef FUTURE_NEED
			if(get_ue_context_by_sgw_s5s8_teid(msg->gtpc_msg.ub_req.header.teid.has_teid.teid,
																				&context) != 0) {
				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;

			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_req.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_req.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_req.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
#endif
			break;
		}

		case GTP_UPDATE_BEARER_RSP:{

#ifdef FUTURE_NEED
			if(get_ue_context(msg->gtpc_msg.ub_rsp.header.teid.has_teid.teid, &context)){

				clLog(clSystemLog, eCLSeverityCritical, "[%s]:[%s]:[%d]UE context not found \n", __file__, __func__, __LINE__);
				return;
			}
			pdn_connection_t *pdn_cntxt = NULL;
			rsp_info->seq = msg->gtpc_msg.ub_rsp.header.teid.has_teid.seq;
			rsp_info->teid = context->s11_sgw_gtpc_teid;
			for(uint8_t i =0; i < msg->gtpc_msg.ub_rsp.bearer_context_count;i++){
				rsp_info->bearer_id[rsp_info->bearer_count++] =
							msg->gtpc_msg.ub_rsp.bearer_contexts[i].eps_bearer_id.ebi_ebi;
			}
			pdn_cntxt = context->eps_bearers[rsp_info->ebi_index]->pdn;
			rsp_info->sender_teid = pdn_cntxt->s5s8_pgw_gtpc_teid;
#endif
			break;
		}
        default:
            // unhandled error 
            assert(0);
	}
}

void ubr_error_response(msg_info_t *msg, uint8_t cause_value, int iface)
{

	int ret = 0;
	err_rsp_info rsp_info = {0};
	int ebi_index = 0;
    uint16_t payload_length;

	get_error_ubrsp_info(msg, &rsp_info); // ubrsp 

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	if(iface == S5S8_IFACE){
		upd_bearer_rsp_t ubr_rsp = {0};

		set_gtpv2c_teid_header(&ubr_rsp.header,
									GTP_UPDATE_BEARER_RSP,
									rsp_info.sender_teid,
									rsp_info.seq);
		set_cause_error_value(&ubr_rsp.cause, IE_INSTANCE_ZERO,
													cause_value);
		ubr_rsp.bearer_context_count = rsp_info.bearer_count;
		for(int i = 0; i < rsp_info.bearer_count; i++){

			set_ie_header(&ubr_rsp.bearer_contexts[i].header, GTP_IE_BEARER_CONTEXT,
				IE_INSTANCE_ZERO, 0);

			set_ebi(&ubr_rsp.bearer_contexts[i].eps_bearer_id, IE_INSTANCE_ZERO,
															rsp_info.bearer_id[i]);
			ubr_rsp.bearer_contexts[i].header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

			set_cause_error_value(&ubr_rsp.bearer_contexts[i].cause, IE_INSTANCE_ZERO,
																			cause_value);
			ubr_rsp.bearer_contexts[i].header.len += sizeof(uint16_t) + IE_HEADER_SIZE;
		}

		uint16_t msg_len = 0;
		msg_len = encode_upd_bearer_rsp(&ubr_rsp, (uint8_t *)gtpv2c_tx);
		gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
		payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
			//send S5S8 interface update bearer response.
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
		   	  		(struct sockaddr *) (&my_sock.s5s8_recv_sockaddr),
		            sizeof(struct sockaddr_in));
	}else{
		ebi_index = rsp_info.bearer_id[0] - 5;
		ue_context_t *context = NULL;
		pdn_connection_t *pdn_cntxt = NULL;

		if(msg->msg_type == GTP_UPDATE_BEARER_REQ){
			ret = get_ue_context_by_sgw_s5s8_teid(rsp_info.teid, &context);
		}else{
			ret = get_ue_context(rsp_info.teid, &context);
		}

		if (ret) {
			clLog(sxlogger, eCLSeverityCritical, "%s:%d Error: %d \n", __func__,
				__LINE__, ret);
			return;
		}
        pdn_cntxt = context->eps_bearers[ebi_index]->pdn;
        if(cp_config->gx_enabled) {
            gen_reauth_error_response(pdn_cntxt, DIAMETER_UNABLE_TO_COMPLY);
        }
	}

}
#endif

/* Function to Fill and Send  Version not supported response to peer node */
void send_version_not_supported(struct sockaddr_in *peer_addr, int iface, uint32_t seq)
{
	uint16_t payload_length = 0;
	uint16_t msg_len = 0;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) gtp_tx_buf;
	gtpv2c_header_t *header = (gtpv2c_header_t *) gtpv2c_tx;

	set_gtpv2c_header(header, 0, GTP_VERSION_NOT_SUPPORTED_IND, 0, seq);

	msg_len = encode_gtpv2c_header_t(header, (uint8_t *)gtpv2c_tx);
	header->gtpc.message_len = htons(msg_len - 4);

	payload_length = msg_len;
    if(iface == S11_IFACE){
        payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
            + sizeof(gtpv2c_tx->gtpc);

        gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
                (struct sockaddr *) peer_addr,
                sizeof(struct sockaddr_in));

        increment_mme_peer_stats(MSG_TX_GTPV2_S11_VERSION_NOT_SUPPORTED,peer_addr->sin_addr.s_addr);

        gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
                (struct sockaddr *) peer_addr, sizeof(struct sockaddr_in));

    }else{
        increment_sgw_peer_stats(MSG_TX_GTPV2_S5S8_VERSION_NOT_SUPPORTED, peer_addr->sin_addr.s_addr);
		gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
				(struct sockaddr *)(&my_sock.s5s8_recv_sockaddr), 
                sizeof(struct sockaddr_in));

	}

	return;
}
