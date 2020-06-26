// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_common.h"
#include <rte_errno.h>
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "cp_config.h"
#include "pfcp_timer.h"
#include "gtpv2c_error_rsp.h"
#include "clogger.h"
#include "rte_hash_crc.h"
#include "pfcp_cp_set_ie.h" // ajay - upf context should be part of differnt file 
#include "upf_struct.h"
#include "cp_interface.h"
#include "sm_structs_api.h"
#include "gx_error_rsp.h"

#define DIAMETER_PCC_RULE_EVENT (5142)

extern udp_sock_t my_sock;

bool
pfcp_add_timer_entry(peerData_t *conn_data, uint32_t timeout_ms,
		gstimercallback cb)
{

	if (!init_timer(&conn_data->pt, timeout_ms, cb, (void *)conn_data))
	{
		clLog(clSystemLog, eCLSeverityCritical,"%s:%s:%u =>%s - initialization of %s failed erro no %d\n",
				__FILE__, __func__, __LINE__,
				getPrintableTime(), conn_data->name, errno);
		return false;
	}
	return true;
}

void
pfcp_peer_timer_callback(gstimerinfo_t *ti, const void *data_t )
{
        int ret = 0;
        int64_t seid = 0;
        msg_info msg;
        ue_context_t *context = NULL;
        pdn_connection_t *pdn = NULL;
        struct resp_info *resp = NULL;

        RTE_SET_USED(ti);

#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
        peerData_t *data =  (peerData_t *) data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */


        if (data->itr_cnt >= data->itr - 1) {
                ret = get_ue_context(data->teid, &context);
                if ( ret < 0) {
                        clLog(clSystemLog, eCLSeverityCritical, "%s:Entry not found for teid:%x...\n", __func__, data->teid);
                        return;
                }
                if(context != NULL && context->eps_bearers[data->ebi_index] != NULL
                                && context->eps_bearers[data->ebi_index]->pdn != NULL ) {
                        pdn = context->eps_bearers[data->ebi_index]->pdn;
                        seid = pdn->seid;
                        if (get_sess_entry(seid, &resp) != 0){
                                /* Assuming that session is not established or resp entry is not created */
                                upf_context_t *upf_context = NULL;
                                context_key *key = NULL;

                                ret = rte_hash_lookup_data(upf_context_by_ip_hash,
                                                (const void*) &(context->eps_bearers[data->ebi_index]->pdn->upf_ipv4.s_addr),
                                                (void **) &(upf_context));

                                if (upf_context != NULL &&  ret >= 0) {
                                        /*msg.msg_type = GTP_CREATE_SESSION_REQ;
                                          msg.gtpc_msg.csr = upf_context->csr;
                                          cs_error_response(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING,
                                          cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);*/
                                        key = LIST_FIRST(&upf_context->pendingCSRs);
                                        while(key != NULL) {
                                                LIST_REMOVE(key, csrentries);
                                                msg.msg_type = GTP_CREATE_SESSION_REQ;
                                                msg.gtpc_msg.csr.sender_fteid_ctl_plane.teid_gre_key = key->sender_teid;
                                                msg.gtpc_msg.csr.header.teid.has_teid.seq = key->sequence;
                                                msg.gtpc_msg.csr.bearer_contexts_to_be_created.eps_bearer_id.ebi_ebi =
                                                        key->ebi_index + 5;
                                                msg.gtpc_msg.csr.header.teid.has_teid.teid = key->teid;
                                                cs_error_response(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING,
                                                                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
                                                
                                                rte_free(key);
                                                key = LIST_FIRST(&upf_context->pendingCSRs);
                                        }
                                }
                        } else {
                                if (GTP_MODIFY_BEARER_REQ == resp->msg_type) {
                                        msg.gtpc_msg.mbr = resp->gtpc_msg.mbr;
                                        msg.msg_type = resp->msg_type;
                                        mbr_error_response(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING,
                                                        cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
                                } else if (GTP_CREATE_SESSION_REQ == resp->msg_type) {
                                        msg.gtpc_msg.csr = resp->gtpc_msg.csr;
                                        msg.msg_type = resp->msg_type;
                                        cs_error_response(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING,
                                                        cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
                                } else if (GTP_DELETE_SESSION_REQ == resp->msg_type) {
                                        msg.gtpc_msg.dsr = resp->gtpc_msg.dsr;
                                        msg.msg_type = resp->msg_type;
                                        ds_error_response(&msg, GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING,
                                                        cp_config->cp_type != PGWC ? S11_IFACE :S5S8_IFACE);
                                } else if ((cp_config->cp_type == PGWC ||  cp_config->cp_type ==  SAEGWC )
                                                && (resp->msg_type == GX_RAR_MSG || resp->state == CREATE_BER_REQ_SNT_STATE)) {
                                    if(cp_config->gx_enabled) {
                                        gen_reauth_error_response(pdn, DIAMETER_PCC_RULE_EVENT);
                                    }
                                        pdn->state = IDEL_STATE;
                                } else {
                                        /* Need to handle for other request */
                                }
                        }
                }
                if(data->pt.ti_id != 0) {
                        stoptimer(&data->pt.ti_id);
                        deinittimer(&data->pt.ti_id);
                        /* free peer data when timer is de int */
                        rte_free(data);
                }
                return;
        }

        /* timer retry handler */
        switch(data->portId) {
                case PFCP_IFACE:
                        pfcp_timer_retry_send(my_sock.sock_fd_pfcp, data);
                        break;
                default:
                        break;
        }
        data->itr_cnt++;
        return;
}

peerData_t *
pfcp_fill_timer_entry_data(enum source_interface iface, struct sockaddr_in *peer_addr,
		uint8_t *buf, uint16_t buf_len, uint8_t itr, uint32_t teid,  uint8_t ebi_index)
{
	peerData_t *timer_entry = NULL;

	timer_entry = rte_zmalloc_socket(NULL, sizeof(peerData_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if(timer_entry == NULL )
	{
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate timer entry :"
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__, __LINE__);
		return NULL;
	}
	memset(timer_entry, 0, sizeof(peerData_t));

	timer_entry->portId = (uint8_t)iface;
	timer_entry->dstIP = peer_addr->sin_addr.s_addr;
	timer_entry->dstPort = peer_addr->sin_port;
	timer_entry->itr = itr;
	timer_entry->teid = teid;
	timer_entry->ebi_index = ebi_index;
	timer_entry->buf_len = buf_len;
	memcpy(&timer_entry->buf,(uint8_t *)buf, buf_len);

	return(timer_entry);
}

void
delete_pfcp_if_timer_entry(uint32_t teid, uint8_t ebi_index)
{
        int ret = 0;
        peerData_t *data = NULL;
        ue_context_t *context = NULL;

        ret = get_ue_context(teid, &context);
        if ( ret < 0) {
                clLog(clSystemLog, eCLSeverityCritical, "%s:Entry not found for teid:%x...\n", __func__, teid);
                return;
        }
        if(context != NULL && context->eps_bearers[ebi_index] != NULL
                        && context->eps_bearers[ebi_index]->pdn != NULL ) {
                data = context->eps_bearers[ebi_index]->pdn->timer_entry;
                if(data->pt.ti_id != 0) {
                        stoptimer(&data->pt.ti_id);
                        deinittimer(&data->pt.ti_id);
                        /* free peer data when timer is de int */
                        rte_free(data);
                }
        }
        return;
}


void
pfcp_delete_timer_entry(uint32_t teid)
{
        int ret = 0;
        peerData_t *data = NULL;
        eps_bearer_t *bearer = NULL;

        ret = get_bearer_by_teid(teid, &bearer);
        if ( ret < 0) {
                clLog(clSystemLog, eCLSeverityCritical, "%s:Entry not found for teid:%x...\n", __func__, teid);
                return;
        }

        if(bearer != NULL && bearer->pdn != NULL &&
                        bearer->pdn->timer_entry != NULL ) {
                data = bearer->pdn->timer_entry;
                if(data->pt.ti_id != 0) {
                        stoptimer(&data->pt.ti_id);
                        deinittimer(&data->pt.ti_id);
                        /* free peer data when timer is de int */
                        rte_free(data);
                }
        }
        return;
}

void
add_pfcp_if_timer_entry(uint32_t teid, struct sockaddr_in *peer_addr,
        uint8_t *buf, uint16_t buf_len, uint8_t ebi_index)
{
        int ret = 0;
        peerData_t *timer_entry = NULL;
        ue_context_t *context = NULL;

        ret = get_ue_context(teid, &context);
        if ( ret < 0) {
                clLog(clSystemLog, eCLSeverityCritical, "%s:Entry not found for teid:%x...\n", __func__, teid);
                return;
        }
        /* fill and add timer entry */
        timer_entry = pfcp_fill_timer_entry_data(PFCP_IFACE, peer_addr,
                        buf, buf_len, cp_config->request_tries, teid, ebi_index);

        if(!(pfcp_add_timer_entry(timer_entry, cp_config->request_timeout, pfcp_peer_timer_callback))) {
                clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Faild to add timer entry...\n",
                                __FILE__, __func__, __LINE__);
        }
        if(context != NULL && context->eps_bearers[ebi_index] != NULL
                        &&  context->eps_bearers[ebi_index]->pdn != NULL ) {
                context->eps_bearers[ebi_index]->pdn->timer_entry = timer_entry;

                if (starttimer(&timer_entry->pt) < 0) {
                        clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Periodic Timer failed to start...\n",
                                        __FILE__, __func__, __LINE__);
                }
        }
}

