// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "cp_init.h"
#include "pfcp.h"
#include "pfcp_cp_util.h"
#include "pfcp_enum.h"
#include "csid_struct.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_messages_encoder.h"
#include "cp_log.h"
#include "csid_api.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "gx_interface.h"
#include "ipc_api.h"
#include "cp_io_poll.h"

extern uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];

static uint16_t sequence = 0;

int8_t
fill_pgw_restart_notification(gtpv2c_header_t *gtpv2c_tx,
		uint32_t s11_sgw, uint32_t s5s8_pgw)
{
	/* Encode the PGW Restart Notification request*/
	pgw_rstrt_notif_t pgw_rstrt_notif = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *)&pgw_rstrt_notif.header,
			GTP_PGW_RESTART_NOTIFICATION, 0, ++sequence);

	/* Fill the SGW S11 IP Address */
	set_ie_header(&pgw_rstrt_notif.sgw_s11s4_ip_addr_ctl_plane.header,
			GTP_IE_IP_ADDRESS, IE_INSTANCE_ONE,
			sizeof(uint32_t));

	pgw_rstrt_notif.sgw_s11s4_ip_addr_ctl_plane.ipv4_ipv6_addr = s11_sgw;

	/* Fill the PGW S5/S8 IP Address */
	set_ie_header(&pgw_rstrt_notif.pgw_s5s8_ip_addr_ctl_plane_or_pmip.header,
			GTP_IE_IP_ADDRESS, IE_INSTANCE_ZERO,
			sizeof(uint32_t));

	if (cp_config->cp_type == SAEGWC) {
		pgw_rstrt_notif.pgw_s5s8_ip_addr_ctl_plane_or_pmip.ipv4_ipv6_addr = s11_sgw;
	} else {
		pgw_rstrt_notif.pgw_s5s8_ip_addr_ctl_plane_or_pmip.ipv4_ipv6_addr = s5s8_pgw;
	}

	/* Set Cause value */
	set_ie_header(&pgw_rstrt_notif.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
			sizeof(struct cause_ie_hdr_t));
	pgw_rstrt_notif.cause.cause_value = GTPV2C_CAUSE_PGW_NOT_RESPONDING;


	uint16_t msg_len = 0;
	msg_len = encode_pgw_rstrt_notif(&pgw_rstrt_notif, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	return 0;
}

static int8_t
fill_gtpc_del_set_pdn_conn_req(gtpv2c_header_t *gtpv2c_tx, fqcsid_t *local_csids,
		uint8_t iface)
{
	del_pdn_conn_set_req_t del_pdn_conn_req = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *)&del_pdn_conn_req.header,
			GTP_DELETE_PDN_CONNECTION_SET_REQ, 0, ++sequence);

	if ((iface == S11_SGW_PORT_ID) ||
			(iface == SX_PORT_ID) ||
			(iface == S5S8_SGWC_PORT_ID)) {
			/* Set the SGW FQ-CSID */
			if (cp_config->cp_type != PGWC) {
				if (local_csids->num_csid) {
					set_gtpc_fqcsid_t(&del_pdn_conn_req.sgw_fqcsid, IE_INSTANCE_ONE,
							local_csids);
				}
			} else {
				/* Set the PGW FQ-CSID */
				if (local_csids->num_csid) {
					set_gtpc_fqcsid_t(&del_pdn_conn_req.pgw_fqcsid, IE_INSTANCE_TWO,
							local_csids);
				}
			}
	} else if (iface == S5S8_PGWC_PORT_ID) {
		/* Set the PGW FQ-CSID */
		if (local_csids->num_csid) {
			set_gtpc_fqcsid_t(&del_pdn_conn_req.pgw_fqcsid, IE_INSTANCE_TWO,
					local_csids);
		}
	} else {
		LOG_MSG(LOG_ERROR, "Select Invalid iface type..");
		return -1;
	}

	/* Encode the del pdn conn set request*/
	uint16_t msg_len = 0;
	msg_len = encode_del_pdn_conn_set_req(&del_pdn_conn_req, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

	return 0;
}

uint32_t s5s8_node_addr = 0;

static int8_t
fill_ccr_t_request(pdn_connection_t *pdn, uint8_t ebi_index)
{
	uint16_t msglen = 0;
	char *buffer = NULL;
	gx_msg ccr_request = {0};
	gx_context_t *gx_context = NULL;

	/* Retrive Gx_context based on Sess ID. */
	ue_context_t *temp_ue_context = (ue_context_t *)get_gx_context((uint8_t *)pdn->gx_sess_id);
	if (temp_ue_context == NULL) {
		LOG_MSG(LOG_ERROR, "NO ENTRY FOUND IN Gx HASH [%s]", pdn->gx_sess_id);
		return -1;
	}
    gx_context = (gx_context_t *)temp_ue_context->gx_context;

	/* VS: Set the Msg header type for CCR-T */
	ccr_request.msg_type = GX_CCR_MSG ;

	/* VS: Set Credit Control Request type */
	ccr_request.data.ccr.presence.cc_request_type = PRESENT;
	ccr_request.data.ccr.cc_request_type = TERMINATION_REQUEST ;

	/* VG: Set Credit Control Bearer opertaion type */
	ccr_request.data.ccr.presence.bearer_operation = PRESENT;
	ccr_request.data.ccr.bearer_operation = TERMINATION ;

	/* VS: Fill the Credit Crontrol Request to send PCRF */
	if(fill_ccr_request(&ccr_request.data.ccr, pdn->context, ebi_index, pdn->gx_sess_id) != 0) {
		LOG_MSG(LOG_ERROR, "Failed CCR request filling process");
		return -1;
	}
	/* Update UE State */
	pdn->state = CCR_SNT_STATE;

	/* VS: Set the Gx State for events */
	gx_context->state = CCR_SNT_STATE;
	gx_context->proc = pdn->proc;

	/* VS: Calculate the max size of CCR msg to allocate the buffer */
	msglen = gx_ccr_calc_length(&ccr_request.data.ccr);

	buffer = (char *) calloc (1, msglen + sizeof(ccr_request.msg_type));
	if (buffer == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate CCR Buffer memory");
		return -1;
	}

	memcpy(buffer, &ccr_request.msg_type, sizeof(ccr_request.msg_type));

	if (gx_ccr_pack(&(ccr_request.data.ccr),
				(unsigned char *)(buffer + sizeof(ccr_request.msg_type)+sizeof(ccr_request.seq_num)), msglen) == 0) {
		LOG_MSG(LOG_ERROR, "ERROR: Packing CCR Buffer... ");
		return -1;
	}

	/* VS: Write or Send CCR -T msg to Gx_App */
	gx_send(my_sock.gx_app_sock, buffer, msglen + sizeof(ccr_request.msg_type) + sizeof(ccr_request.seq_num));
	LOG_MSG(LOG_DEBUG, "Send CCR-T to PCRF ");

	struct sockaddr_in saddr_in;
	saddr_in.sin_family = AF_INET;
	inet_aton("127.0.0.1", &(saddr_in.sin_addr));
    increment_gx_peer_stats(MSG_TX_DIAMETER_GX_CCR_T, saddr_in.sin_addr.s_addr);

	return 0;
}

static int8_t
flush_pdr_entries(eps_bearer_t *bearer)
{
	for (uint8_t itr = 0; itr < bearer->pdr_count; itr++) {
		if (del_pdr_entry((bearer->pdrs[itr])->rule_id)) {
			return -1;
		}
	}

	for (uint8_t itr1 = 0; itr1 < bearer->qer_count; itr1++) {
		if (del_qer_entry(bearer->qer_id[itr1].qer_id)) {
			return -1;
		}
	}
	return 0;
}

static int8_t
del_sess_by_csid_entry(uint32_t teid, uint8_t iface)
{
	int i = 0;
	ue_context_t *context = NULL;

	context = (ue_context_t *)get_ue_context(teid);

	if (context == NULL)
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;


	for (uint8_t ebi_index = 0; ebi_index < MAX_BEARERS; ebi_index++) {

		eps_bearer_t *bearer = context->eps_bearers[ebi_index];
		if (bearer == NULL)
			continue;

		pdn_connection_t *pdn = bearer->pdn;
		if (pdn == NULL)
			continue;

		if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == PGWC))
			s5s8_node_addr = pdn->s5s8_pgw_gtpc_ipv4.s_addr;

		if ((cp_config->gx_enabled) && (cp_config->cp_type != SGWC)) {
			/* TODO: Need to remove in real enviorment*/
			if (iface != S11_SGW_PORT_ID) {
				fill_ccr_t_request(pdn, ebi_index);
			}
		}

		for (i = 0; i < MAX_BEARERS; ++i) {
			if (pdn->eps_bearers[i] == NULL)
				continue;

			if (context->eps_bearers[i] == pdn->eps_bearers[i]) {
				if (flush_pdr_entries(pdn->eps_bearers[i])) {
					/* TODO: Error Handling */
					return -1;
				}
				free(context->eps_bearers[i]);
			}
		}

		free(pdn);
		pdn = NULL;
	}

	/* Delete UE context entry from IMSI Hash */
	if (ue_context_delete_entry_imsiKey(context->imsi) < 0) {
		LOG_MSG(LOG_ERROR, "Error on ue_context_by_imsi_hash del");
	}
	free(context);
	context = NULL;

    decrement(NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);


	return 0;
}

static int8_t
cleanup_sess_by_csid_entry(fqcsid_t *csids, uint8_t iface)
{
	/* Get the session ID by csid */
	for (uint8_t itr1 = 0; itr1 < csids->num_csid; itr1++) {
		sess_csid *tmp = NULL;
		tmp = get_sess_csid_entry(csids->local_csid[itr1]);
		if (tmp == NULL) {
			LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
			return -1;
		}

		for (uint8_t itr2 = 0; itr2 < tmp->seid_cnt; itr2++) {
			uint32_t teid_key = UE_SESS_ID(tmp->cp_seid[itr2]);
			if (del_sess_by_csid_entry(teid_key, iface)) {
				/* TODO Handle Error */
			}

			/* Delete UE context entry from UE Hash */
			if (ue_context_delete_entry_teidKey(teid_key) < 0) {
				LOG_MSG(LOG_ERROR, "Error on ue_context_by_fteid_hash del");
			}
		}
	}
	return 0;
}

/* Cleanup Session information by local csid*/
int8_t
del_peer_node_sess(uint32_t node_addr, uint8_t iface)
{
	LOG_MSG(LOG_DEBUG, "START");
	int ret = 0;
	uint16_t payload_length = 0;
	fqcsid_t csids = {0};
	fqcsid_t *peer_csids = NULL;
	bzero(&tx_buf, sizeof(tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)tx_buf;
	memset(gtpv2c_tx, 0, sizeof(gtpv2c_header_t));

	/* Get peer CSID associated with node */
	peer_csids = get_peer_addr_csids_entry(ntohl(node_addr),
			MOD);

	if (peer_csids == NULL) {
		/* Delete UPF hash entry */
		if (iface == SX_PORT_ID) {
			if (upf_context_delete_entry(&node_addr) < 0) {
				LOG_MSG(LOG_ERROR, " Error on deleting upf context del");
			}
		}
		LOG_MSG(LOG_DEBUG, " Entry not found ");
		return -1;
	}

	/* Get the mapped local CSID */
	csids.num_csid = peer_csids->num_csid;
	for (int8_t itr = 0; itr < peer_csids->num_csid; itr++) {
		csid_t *tmp = NULL;
		tmp = get_peer_csid_entry(&peer_csids->local_csid[itr],
				iface);
		if (tmp == NULL) {
			LOG_MSG(LOG_DEBUG, " Entry not found ");
			return -1;
		}
		csids.local_csid[itr] = tmp->local_csid;
		csids.node_addr = tmp->node_addr;
	}

	if (!csids.num_csid) {
		LOG_MSG(LOG_DEBUG, "Csids are empty ");
		return 0;
	}

	/* Cleanup Internal data structures */
	ret = cleanup_sess_by_csid_entry(&csids, iface);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		//return -1;
	}

	if (cp_config->cp_type != PGWC) {
		if (iface != S11_SGW_PORT_ID) {
				fill_pgw_restart_notification(gtpv2c_tx, ntohl(cp_config->s11_ip.s_addr),
						s5s8_node_addr);
				/* Send the Delete PDN Request to peer node */
				payload_length = 0;
				payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
					+ sizeof(gtpv2c_tx->gtpc);

				/* TODO: NEED TO HANDLE THIS */
				/* Send the PGW Restart notification */
				if (cp_config->cp_type == SAEGWC) {
					gtpv2c_send(my_sock.sock_fd_s11, tx_buf, payload_length,
							(struct sockaddr *) &s11_mme_sockaddr,
							sizeof(struct sockaddr_in));

					LOG_MSG(LOG_DEBUG, "Send PGW Restart notification to MME ");

				}

				if ((cp_config->cp_type == SGWC) && (iface == S5S8_SGWC_PORT_ID)) {
					gtpv2c_send(my_sock.sock_fd_s11, tx_buf, payload_length,
							(struct sockaddr *) &s11_mme_sockaddr,
							sizeof(struct sockaddr_in));

					LOG_MSG(LOG_DEBUG, "Send PGW Restart notification to MME ");
				}

				memset(gtpv2c_tx, 0, sizeof(gtpv2c_header_t));
		}
	}


	/* Fill the Delete PDN Request */
	fill_gtpc_del_set_pdn_conn_req(gtpv2c_tx, &csids, iface);

	/* Send the Delete PDN Request to peer node */
	payload_length = 0;
	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
		+ sizeof(gtpv2c_tx->gtpc);

	//fqcsid_t *csid = NULL;
	if (cp_config->cp_type == PGWC) {
		if (iface != S5S8_PGWC_PORT_ID) {
			/* Get peer CSID associated with node */
			//csid = get_peer_addr_csids_entry(s5s8_recv_sockaddr.sin_addr.s_addr,
			//		MOD);
				gtpv2c_send(my_sock.sock_fd_s5s8, tx_buf, payload_length,
						(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
                         sizeof(struct sockaddr_in));
		}
	} else {
		if (iface != S11_SGW_PORT_ID) {
			/* Get peer CSID associated with node */
			//csid = get_peer_addr_csids_entry(s11_mme_sockaddr.sin_addr.s_addr,
			//		MOD);
				gtpv2c_send(my_sock.sock_fd_s11, tx_buf, payload_length,
						(struct sockaddr *) &s11_mme_sockaddr,
						sizeof(struct sockaddr_in));
		}
		if (cp_config->cp_type == SGWC) {
			/* Get peer CSID associated with node */
			//csid = get_peer_addr_csids_entry(s5s8_recv_sockaddr.sin_addr.s_addr,
			//		MOD);
			if (iface != S5S8_SGWC_PORT_ID) {
				gtpv2c_send(my_sock.sock_fd_s5s8, tx_buf, payload_length,
						(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
                        sizeof(struct sockaddr_in));
			}
		}
	}

	/* TODO: Temp work around */
	if (iface == S5S8_SGWC_PORT_ID) {
		peer_csids->node_addr = ntohl(peer_csids->node_addr);
	}

	/* Cleanup Internal data structures */
	ret = del_csid_entry_hash(peer_csids, &csids, iface);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}

	/* Delete UPF hash entry */
	if (iface == SX_PORT_ID) {
		if (upf_context_delete_entry(&node_addr) < 0) {
			LOG_MSG(LOG_ERROR,
					" Error on delete upf context ");
		}
	}
	LOG_MSG(LOG_DEBUG, "END");
	return 0;
}

int8_t
process_del_pdn_conn_set_req_t(del_pdn_conn_set_req_t *del_pdn_req,
		gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	uint8_t iface = 0;
	fqcsid_t csids = {0};
	fqcsid_t peer_csids = {0};

	/* MME FQ-CSID */
	if (del_pdn_req->mme_fqcsid.header.len) {
		csids.num_csid = del_pdn_req->mme_fqcsid.number_of_csids;
		peer_csids.num_csid = csids.num_csid;
		for (uint8_t itr = 0; itr < csids.num_csid; itr++) {
			/* Get linked local csid */
			csid_t *tmp = NULL;
			tmp = get_peer_csid_entry(&del_pdn_req->mme_fqcsid.pdn_csid[itr],
					S11_SGW_PORT_ID);
			if (tmp == NULL) {
				LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
				return -1;
			}
			/* TODO: Hanlde Multiple CSID with single MME CSID */
			csids.local_csid[itr] = tmp->local_csid;
			csids.node_addr = tmp->node_addr;
			peer_csids.local_csid[itr] = del_pdn_req->mme_fqcsid.pdn_csid[itr];
			peer_csids.node_addr = del_pdn_req->mme_fqcsid.node_address;
		}

		/* Send the delete PDN set request to PGW */
		if (cp_config->cp_type == SGWC ) {
			/* TODO: UPDATE THE NODE ADDRESS */
			csids.node_addr = cp_config->s5s8_ip.s_addr;

			fill_gtpc_del_set_pdn_conn_req(gtpv2c_tx, &csids,
					S5S8_SGWC_PORT_ID);
		}
		/* Set the interface */
		iface = S11_SGW_PORT_ID;
	}

	/* SGW FQ-CSID */
	if (del_pdn_req->sgw_fqcsid.header.len) {
		csids.num_csid = del_pdn_req->sgw_fqcsid.number_of_csids;
		peer_csids.num_csid = csids.num_csid;
		for (uint8_t itr = 0; itr < csids.num_csid; itr++) {
			/* Get linked local csid */
			csid_t *tmp = NULL;
			tmp = get_peer_csid_entry(&del_pdn_req->sgw_fqcsid.pdn_csid[itr],
					S5S8_PGWC_PORT_ID);
			if (tmp == NULL) {
				LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
				return -1;
			}
			/* TODO: Hanlde Multiple CSID with single MME CSID */
			csids.local_csid[itr] = tmp->local_csid;
			csids.node_addr = tmp->node_addr;
			peer_csids.local_csid[itr] = del_pdn_req->sgw_fqcsid.pdn_csid[itr];
			peer_csids.node_addr = del_pdn_req->sgw_fqcsid.node_address;
		}
		/* Set the interface */
		iface = S5S8_PGWC_PORT_ID;
	}

	/* PGW FQ-CSID */
	if (del_pdn_req->pgw_fqcsid.header.len) {
		csids.num_csid = del_pdn_req->pgw_fqcsid.number_of_csids;
		peer_csids.num_csid = csids.num_csid;
		for (uint8_t itr = 0; itr < csids.num_csid; itr++) {
			/* Get linked local csid */
			csid_t *tmp = NULL;
			tmp = get_peer_csid_entry(&del_pdn_req->pgw_fqcsid.pdn_csid[itr],
					S5S8_SGWC_PORT_ID);
			if (tmp == NULL) {
				LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
				return -1;
			}
			/* TODO: Hanlde Multiple CSID with single MME CSID */
			csids.local_csid[itr] = tmp->local_csid;
			csids.node_addr = tmp->node_addr;
			peer_csids.local_csid[itr] = del_pdn_req->pgw_fqcsid.pdn_csid[itr];
			peer_csids.node_addr = del_pdn_req->pgw_fqcsid.node_address;
		}

		/* Set the interface */
		iface = S5S8_SGWC_PORT_ID;
	}

	if (csids.num_csid == 0) {
		LOG_MSG(LOG_ERROR, "Not received any CSID from peer node ");
		return -1;
	}

	/* PGW FQ-CSID */
	if (del_pdn_req->pgw_fqcsid.header.len) {
		/* Send the delete PDN set request to MME */
		if (cp_config->cp_type == SGWC ) {
			csids.node_addr = ntohl(cp_config->s11_ip.s_addr);

			fill_gtpc_del_set_pdn_conn_req(gtpv2c_tx, &csids,
					S11_SGW_PORT_ID);
		}
		if (cp_config->cp_type == SGWC) {
			bzero(&s11_tx_buf, sizeof(s11_tx_buf));
			gtpv2c_header_t *gtpv2c_tx_t = (gtpv2c_header_t *)s11_tx_buf;
			//fqcsid_t *csid_t = NULL;
			//csid_t = get_peer_addr_csids_entry(s11_mme_sockaddr.sin_addr.s_addr,
			//		MOD);
			//if (csid_t != NULL) {
				/* Fill the PGW restart notification request */
				fill_pgw_restart_notification(gtpv2c_tx_t, ntohl(cp_config->s11_ip.s_addr),
						peer_csids.node_addr);
				/* Send the Delete PDN Request to peer node */
				int payload_length = 0;
				payload_length = ntohs(gtpv2c_tx_t->gtpc.message_len)
					+ sizeof(gtpv2c_tx_t->gtpc);

				/* TODO: NEED TO HANDLE THIS */
				/* Send the PGW Restart notification */
				gtpv2c_send(my_sock.sock_fd_s11, s11_tx_buf, payload_length,
						(struct sockaddr *) &s11_mme_sockaddr,
						sizeof(struct sockaddr_in));

				LOG_MSG(LOG_DEBUG, "Send PGW Restart notification to MME ");
			//}
			}
	}

	/* MME FQ-CSID */
	if (del_pdn_req->mme_fqcsid.header.len) {
		/* Send the delete PDN set request to PGW */
		if (cp_config->cp_type == SGWC ) {
			csids.node_addr = ntohl(cp_config->s5s8_ip.s_addr);

			fill_gtpc_del_set_pdn_conn_req(gtpv2c_tx, &csids,
					S5S8_SGWC_PORT_ID);
		}
	}

	/* Send the PFCP deletion session set request to PGW */
	/* TODO: UPDATE THE NODE ADDRESS */
	csids.node_addr = cp_config->pfcp_ip.s_addr;

	pfcp_sess_set_del_req_t del_set_req_t = {0};

	/* TODO: Need To think on it iface*/
	if (cp_config->cp_type == PGWC) {
		fill_pfcp_sess_set_del_req_t(&del_set_req_t, &csids, S5S8_PGWC_PORT_ID);
	} else {
		fill_pfcp_sess_set_del_req_t(&del_set_req_t, &csids, S11_SGW_PORT_ID);
	}

	/* Send the Delete set Request to peer node */
	uint8_t pfcp_msg[1024]={0};
	int encoded = encode_pfcp_sess_set_del_req_t(&del_set_req_t, pfcp_msg);

	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

    struct sockaddr_in upf_pfcp_sockaddr;
    assert(0); // Need handling 
	pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &upf_pfcp_sockaddr);

	/* Cleanup Internal data structures */
	ret = del_csid_entry_hash(&peer_csids, &csids, iface);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}
	return 0;
}

int8_t
fill_gtpc_del_set_pdn_conn_rsp(gtpv2c_header_t *gtpv2c_tx, uint8_t seq_t,
		uint8_t casue_value)
{
	del_pdn_conn_set_rsp_t del_pdn_conn_rsp = {0};

	set_gtpv2c_teid_header((gtpv2c_header_t *)&del_pdn_conn_rsp.header,
			GTP_DELETE_PDN_CONNECTION_SET_RSP, 0, seq_t);

	/* Set Cause value */
	set_ie_header(&del_pdn_conn_rsp.cause.header, GTP_IE_CAUSE, IE_INSTANCE_ZERO,
			sizeof(struct cause_ie_hdr_t));
	del_pdn_conn_rsp.cause.cause_value = casue_value;

	uint16_t msg_len = 0;
	msg_len =  encode_del_pdn_conn_set_rsp(&del_pdn_conn_rsp, (uint8_t *)gtpv2c_tx);
	gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);
	return 0;
}

int8_t
process_del_pdn_conn_set_rsp_t(del_pdn_conn_set_rsp_t *del_pdn_rsp)
{
	if (del_pdn_rsp->cause.cause_value != GTPV2C_CAUSE_REQUEST_ACCEPTED) {
		LOG_MSG(LOG_ERROR, "Error: %s ", strerror(errno));
		return -1;
	}
	return 0;
}




