// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include "pfcp_cp_util.h"
#include "pfcp_enum.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "pfcp_cp_association.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_messages_decoder.h"
#include "sm_structs_api.h"
#include "ue.h"
#include "cp_main.h"
#include "pfcp.h"
#include "ipc_api.h"
#include "spgw_config_struct.h"
#include "cp_config_apis.h"
#include "gtpv2_session.h"
#include "gtp_messages.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "cp_config_defs.h"
#include "ip_pool.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gtpv2_internal.h"
#include "gx_interface.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "util.h"
#include "cp_io_poll.h"
#include "pfcp_cp_interface.h"
#include "cp_log.h"

#ifdef PDR_DEBUG
static void print_pdr(pdn_connection_t *);
#endif

/* Header Size of set_upd_forwarding_param ie */
#define UPD_PARAM_HEADER_SIZE 4

/* len of flags*/
#define FLAG_LEN 2

uint32_t 
fill_pfcp_sess_del_req( pfcp_sess_del_req_t *pfcp_sess_del_req)
{
	uint32_t seq = 1;

	memset(pfcp_sess_del_req, 0, sizeof(pfcp_sess_del_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_DELETION_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_del_req->header),
		PFCP_SESSION_DELETION_REQUEST, HAS_SEID, seq);

    return seq;
}

void
fill_pfcp_sess_set_del_req( pfcp_sess_set_del_req_t *pfcp_sess_set_del_req)
{

	uint32_t seq = 1;
	char sgwc_addr[INET_ADDRSTRLEN] = {0};
	char pgwc_addr[INET_ADDRSTRLEN] = {0};
	char mme_addr[INET_ADDRSTRLEN]  = {0};
	char sgwu_addr[INET_ADDRSTRLEN] = {0};
	char pgwu_addr[INET_ADDRSTRLEN] = {0};
	uint32_t node_value = 0;

	/*Added hardcoded value to remove compile error.Right now,we are using
	function. Will remove hard value  */
	const char* pAddr = "192.168.0.10";
	const char* twan_addr = "192.16.0.1";
	const char* epdg_addr = "192.16.0.2";
	unsigned long sgwc_value = 0;

	memset(pfcp_sess_set_del_req, 0, sizeof(pfcp_sess_set_del_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_SET_DELETION_REQUEST, seq);
	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_set_del_req->header),
			PFCP_SESSION_SET_DELETION_REQUEST, HAS_SEID, seq);

	node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_sess_set_del_req->node_id), node_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->sgw_c_fqcsid), sgwc_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->pgw_c_fqcsid), pgwc_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwu_addr, INET_ADDRSTRLEN);
	unsigned long sgwu_value = inet_addr(sgwu_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->sgw_u_fqcsid), sgwu_value);

	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwu_addr, INET_ADDRSTRLEN);
	unsigned long pgwu_value = inet_addr(pgwu_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->pgw_u_fqcsid), pgwu_value);

	// set of twan fqcsid
	//TODO : IP addres for twan is hardcoded
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->twan_fqcsid), twan_value);

	// set of epdg fqcsid
	//TODO : IP addres for epdgg is hardcoded
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->epdg_fqcsid), epdg_value);

	inet_ntop(AF_INET, &(cp_config->s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_set_del_req->mme_fqcsid), mme_value);

}


int
fill_create_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule, eps_bearer_t *bearer)
{

	uint8_t sdf_filter_count = 0;
	pfcp_create_pdr_ie_t *pdr = NULL;
	pfcp_create_far_ie_t *far = NULL;
	pfcp_create_qer_ie_t *qer = NULL;
    uint8_t urr_idx = 0;
    uint32_t ue_ip_flags = 0; // TODO : analyze why we need to send ue_ip_address in pdi in pfcp modify session

	for(int i=0; i<bearer->pdr_count; i++)
	{
		pdr = &(pfcp_sess_mod_req->create_pdr[i]);
		far = &(pfcp_sess_mod_req->create_far[i]);
		qer = &(pfcp_sess_mod_req->create_qer[i]);

		pdr->qer_id_count = 1;

        /* URR_SUPPORT : Each PDR can have only 1 URR as of now */
        pdr->urr_id_count = 0;
        if(cp_config->urr_enable == 1) {
            pdr->urr_id_count = 1;
        }

		creating_pdr(pdr, i, ue_ip_flags);

		pdr->pdr_id.rule_id = dyn_rule->pdr[i]->rule_id;
		pdr->precedence.prcdnc_val = dyn_rule->pdr[i]->prcdnc_val;

		pdr->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;

		pdr->qer_id[0].qer_id_value = dyn_rule->pdr[i]->qer.qer_id;

		pdr->urr_id[0].urr_id_value = dyn_rule->pdr[i]->urr.urr_id;

		pdr->pdi.ue_ip_address.ipv4_address =
			htonl(dyn_rule->pdr[i]->pdi.ue_addr.ipv4_address);
		pdr->pdi.local_fteid.teid =
			dyn_rule->pdr[i]->pdi.local_fteid.teid;
		pdr->pdi.local_fteid.ipv4_address =
				htonl(dyn_rule->pdr[i]->pdi.local_fteid.ipv4_address);
		pdr->pdi.src_intfc.interface_value =
				dyn_rule->pdr[i]->pdi.src_intfc.interface_value;
		strncpy((char *)pdr->pdi.ntwk_inst.ntwk_inst,
				(char *)&dyn_rule->pdr[i]->pdi.ntwk_inst.ntwk_inst, 32);

		pdr->pdi.src_intfc.interface_value =
			dyn_rule->pdr[i]->pdi.src_intfc.interface_value;

		if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) {
			uint32_t size_teid = 0;
			size_teid = pdr->pdi.local_fteid.header.len + sizeof(pfcp_ie_header_t);
			pdr->pdi.header.len = pdr->pdi.header.len - size_teid;
			pdr->header.len = pdr->header.len - size_teid;
			pdr->pdi.local_fteid.header.len = 0;
		} else if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) {
			uint32_t size_ie = pdr->pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t) + pdr->pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);

			pdr->pdi.header.len -= size_ie;
			pdr->header.len -= size_ie;

			pdr->pdi.ue_ip_address.header.len = 0;
			pdr->pdi.ntwk_inst.header.len = 0;
		}
#if 1
        pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].fd = 1;
		memcpy(&(pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
			&(dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc);

		pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc =
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc;

		pdr->pdi.sdf_filter_count++;
#endif
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++) {
			if(dyn_rule->flow_desc[itr].sdf_flow_description == NULL) {
                continue;
            }
            if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
                    ((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
                     (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

                sdf_pkt_filter_gx_mod(
                        pdr, dyn_rule, sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
                sdf_filter_count++;
            }

            if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
                    ((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
                     (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
                sdf_pkt_filter_gx_mod(
                        pdr, dyn_rule, sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
                sdf_filter_count++;
            }
        }

		pdr->pdi.sdf_filter_count = sdf_filter_count;

		creating_far(far);
		far->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;
		set_destination_interface(&(far->frwdng_parms.dst_intfc));
		pfcp_set_ie_header(&(far->frwdng_parms.header),
				PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

		far->frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

		uint16_t len = 0;
		len += sizeof(pfcp_dst_intfc_ie_t);
		len += UPD_PARAM_HEADER_SIZE;

		far->header.len += len;
		far->frwdng_parms.dst_intfc.interface_value = dyn_rule->pdr[i]->far.dst_intfc.interface_value;

		far->apply_action.forw = PRESENT;

		creating_qer(qer);
		qer->qer_id.qer_id_value  = dyn_rule->pdr[i]->qer.qer_id;

		qer->maximum_bitrate.ul_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.ul_mbr;
		qer->maximum_bitrate.dl_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.dl_mbr;
		qer->guaranteed_bitrate.ul_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.ul_gbr;
		qer->guaranteed_bitrate.dl_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.dl_gbr;
		qer->gate_status.ul_gate  = dyn_rule->pdr[i]->qer.gate_status.ul_gate;
		qer->gate_status.dl_gate  = dyn_rule->pdr[i]->qer.gate_status.dl_gate;
		// URR support
        for(uint8_t idx = 0; idx < 1; idx++)
        {
            creating_urr(&pfcp_sess_mod_req->create_urr[urr_idx]);
            pfcp_sess_mod_req->create_urr[urr_idx].urr_id.urr_id_value  = bearer->urr_id[urr_idx].urr_id;
            pfcp_sess_mod_req->create_urr[urr_idx].meas_mthd.volum = 1;
            pfcp_sess_mod_req->create_urr[urr_idx].rptng_triggers.volth = 1;
            pfcp_sess_mod_req->create_urr[urr_idx].rptng_triggers.volqu = 1;
            pfcp_sess_mod_req->create_urr[urr_idx].vol_thresh.tovol = 1;
            pfcp_sess_mod_req->create_urr[urr_idx].vol_thresh.total_volume = 1000000;
            pfcp_sess_mod_req->create_urr[urr_idx].volume_quota.tovol = 1;
            pfcp_sess_mod_req->create_urr[urr_idx].volume_quota.total_volume = 1000000000; // 1GB 
            urr_idx++;
        }

		pfcp_sess_mod_req->create_pdr_count++;
		pfcp_sess_mod_req->create_far_count++;
		pfcp_sess_mod_req->create_qer_count++;
        pfcp_sess_mod_req->create_urr_count += bearer->urr_count;
	}
	return 0;
}

int
fill_update_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, dynamic_rule_t *dyn_rule)
{

	uint8_t sdf_filter_count = 0;
	pfcp_update_pdr_ie_t *pdr = NULL;
	pfcp_update_far_ie_t *far = NULL;
	pfcp_update_qer_ie_t *qer = NULL;

	for(int i=0; i< 2; i++) // FIXME : dedicated bearer
	{
		pdr = &(pfcp_sess_mod_req->update_pdr[pfcp_sess_mod_req->update_pdr_count+i]);
		far = &(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count+i]);
		qer = &(pfcp_sess_mod_req->update_qer[pfcp_sess_mod_req->update_qer_count+i]);

		updating_pdr(pdr, i);

		pdr->pdr_id.rule_id = dyn_rule->pdr[i]->rule_id;
		pdr->precedence.prcdnc_val = dyn_rule->pdr[i]->prcdnc_val;
		pdr->qer_id.qer_id_value = dyn_rule->pdr[i]->qer_id[0].qer_id;
		pdr->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;


		pdr->pdi.ue_ip_address.ipv4_address =
			htonl(dyn_rule->pdr[i]->pdi.ue_addr.ipv4_address);
		pdr->pdi.local_fteid.teid =
			dyn_rule->pdr[i]->pdi.local_fteid.teid;
		pdr->pdi.local_fteid.ipv4_address =
				htonl(dyn_rule->pdr[i]->pdi.local_fteid.ipv4_address);
		pdr->pdi.src_intfc.interface_value =
				dyn_rule->pdr[i]->pdi.src_intfc.interface_value;
		strncpy((char *)pdr->pdi.ntwk_inst.ntwk_inst,
				(char *)&dyn_rule->pdr[i]->pdi.ntwk_inst.ntwk_inst, 32);

		pdr->pdi.src_intfc.interface_value =
			dyn_rule->pdr[i]->pdi.src_intfc.interface_value;

		if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) {
			uint32_t size_teid = 0;
			size_teid = pdr->pdi.local_fteid.header.len + sizeof(pfcp_ie_header_t);
			pdr->pdi.header.len = pdr->pdi.header.len - size_teid;
			pdr->header.len = pdr->header.len - size_teid;
			pdr->pdi.local_fteid.header.len = 0;
		} else if (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) {
			uint32_t size_ie = pdr->pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t) + pdr->pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);

			pdr->pdi.header.len -= size_ie;
			pdr->header.len -= size_ie;

			pdr->pdi.ue_ip_address.header.len = 0;
			pdr->pdi.ntwk_inst.header.len = 0;
		}
#if 0
		memcpy(&(pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
			&(dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].flow_desc),
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc);

		pdr->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc =
				dyn_rule->pdr[i]->pdi.sdf_filter[pdr->pdi.sdf_filter_count].len_of_flow_desc;

		pdr->pdi.sdf_filter_count++;
#endif
		for(int itr = 0; itr < dyn_rule->num_flw_desc; itr++) {

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {

				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					/* TODO: Revisit following line, change funtion signature and remove type casting */
					sdf_pkt_filter_gx_mod((pfcp_create_pdr_ie_t *)pdr, dyn_rule,
							 sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				LOG_MSG(LOG_ERROR, "Empty SDF rules");
			}

			if(dyn_rule->flow_desc[itr].sdf_flow_description != NULL) {
				if((pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (dyn_rule->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					/* TODO: Revisit following line, change funtion signature and remove type casting */
					sdf_pkt_filter_gx_mod((pfcp_create_pdr_ie_t *) pdr, dyn_rule,
							sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				LOG_MSG(LOG_ERROR, "Empty SDF rules");
			}
		}
		pdr->pdi.sdf_filter_count = sdf_filter_count;

		updating_far(far);
		far->far_id.far_id_value = dyn_rule->pdr[i]->far.far_id_value;
		set_destination_interface(&(far->upd_frwdng_parms.dst_intfc));
		pfcp_set_ie_header(&(far->upd_frwdng_parms.header),
				PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

		far->upd_frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

		uint16_t len = 0;
		len += sizeof(pfcp_dst_intfc_ie_t);
		len += UPD_PARAM_HEADER_SIZE;

		far->header.len += len;
		far->upd_frwdng_parms.dst_intfc.interface_value = dyn_rule->pdr[i]->far.dst_intfc.interface_value;

		far->apply_action.forw = PRESENT;

		updating_qer(qer);
		qer->qer_id.qer_id_value  = dyn_rule->pdr[i]->qer.qer_id;

		qer->maximum_bitrate.ul_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.ul_mbr;
		qer->maximum_bitrate.dl_mbr  = dyn_rule->pdr[i]->qer.max_bitrate.dl_mbr;
		qer->guaranteed_bitrate.ul_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.ul_gbr;
		qer->guaranteed_bitrate.dl_gbr  = dyn_rule->pdr[i]->qer.guaranteed_bitrate.dl_gbr;
		qer->gate_status.ul_gate  = dyn_rule->pdr[i]->qer.gate_status.ul_gate;
		qer->gate_status.dl_gate  = dyn_rule->pdr[i]->qer.gate_status.dl_gate;

		pfcp_sess_mod_req->update_pdr_count++;
		pfcp_sess_mod_req->update_far_count++;
		pfcp_sess_mod_req->update_qer_count++;
	}
	return 0;
}

int
fill_remove_pfcp_info(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer)
{
	pfcp_update_far_ie_t *far = NULL;

	for(int i=0; i<bearer->pdr_count; i++)
	{
		far = &(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]);

		updating_far(far);
		far->far_id.far_id_value = bearer->pdrs[i]->far.far_id_value;
		far->apply_action.drop = PRESENT;

		pfcp_sess_mod_req->update_far_count++;
	}
	return 0;
}

void sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
    eps_bearer_t* bearer,
    int pdr_counter,
    int sdf_filter_count,
    int dynamic_filter_cnt,
    int flow_cnt,
    uint8_t direction)
{
    int len = 0;
    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
    sdf_pkt_filter_to_string(&(bearer->dynamic_rules[dynamic_filter_cnt]->flow_desc[flow_cnt]),
        (char*)(pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
        strlen((char*)(&pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

    len += FLAG_LEN;
    len += sizeof(uint16_t);
    len += pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

    pfcp_set_ie_header(
        &(pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

    /*VG updated the header len of pdi as sdf rules has been added*/
    pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
    pfcp_sess_mod_req->update_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}

int fill_upd_bearer_sdf_rule(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
								eps_bearer_t* bearer,	int pdr_counter){
    int ret = 0;
    int sdf_filter_count = 0;
    /*VG convert pkt_filter_strucutre to char string*/
    for(int index = 0; index < bearer->num_dynamic_filters; index++) {

        pfcp_sess_mod_req->update_pdr[pdr_counter].precedence.prcdnc_val = bearer->dynamic_rules[index]->precedence;
        // itr is for flow information counter
        // sdf_filter_count is for SDF information counter
        for(int itr = 0; itr < bearer->dynamic_rules[index]->num_flw_desc; itr++) {

            if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {

                if((pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
                    ((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
                    (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

                    sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req, bearer, pdr_counter,
                    			sdf_filter_count, index, itr, TFT_DIRECTION_UPLINK_ONLY);
                    sdf_filter_count++;
                }

            } else {
                LOG_MSG(LOG_ERROR, "Empty SDF rules");
            }

            if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {
                if((pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
                    ((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
                    (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
                    sdf_pkt_filter_upd_bearer(pfcp_sess_mod_req, bearer, pdr_counter,
                    		sdf_filter_count, index, itr, TFT_DIRECTION_DOWNLINK_ONLY);
                    sdf_filter_count++;
                }
            } else {
                LOG_MSG(LOG_ERROR, "Empty SDF rules");
            }
        }

		pfcp_sess_mod_req->update_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;

    }
    return ret;
}

void
fill_update_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req, eps_bearer_t *bearer){

	int size1 = 0;
    uint32_t ue_ip_flags = 0; // TODO : take these flags from config/pdn and also analyze the impact on update pdr

	for(int i = pfcp_sess_mod_req->update_pdr_count;
			i < pfcp_sess_mod_req->update_pdr_count + NUMBER_OF_PDR_PER_BEARER;
			i++){

		size1 = 0;
		size1 += set_pdr_id(&(pfcp_sess_mod_req->update_pdr[i].pdr_id));
		size1 += set_precedence(&(pfcp_sess_mod_req->update_pdr[i].precedence));
		size1 += set_pdi(&(pfcp_sess_mod_req->update_pdr[i].pdi), ue_ip_flags);

		int itr = i - pfcp_sess_mod_req->update_pdr_count;

		pfcp_set_ie_header(&(pfcp_sess_mod_req->update_pdr[i].header), PFCP_IE_UPDATE_PDR, size1);

		pfcp_sess_mod_req->update_pdr[i].pdr_id.rule_id = bearer->pdrs[itr]->rule_id;

		pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.teid =
			bearer->pdrs[itr]->pdi.local_fteid.teid;

		if((cp_config->cp_type == SGWC) ||
				(bearer->pdrs[itr]->pdi.src_intfc.interface_value ==
				SOURCE_INTERFACE_VALUE_ACCESS)) {
			/*No need to send ue ip and network instance for pgwc access interface or
			 * for any sgwc interface */
			uint32_t size_ie = 0;
			size_ie = pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t);
			size_ie = size_ie + pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->update_pdr[i].pdi.header.len =
				pfcp_sess_mod_req->update_pdr[i].pdi.header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].header.len =
				pfcp_sess_mod_req->update_pdr[i].header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.header.len = 0;
			pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.header.len = 0;
		}else{
			pfcp_sess_mod_req->update_pdr[i].pdi.ue_ip_address.ipv4_address =
				bearer->pdrs[itr]->pdi.ue_addr.ipv4_address;
			strncpy((char *)pfcp_sess_mod_req->update_pdr[i].pdi.ntwk_inst.ntwk_inst,
				(char *)&bearer->pdrs[itr]->pdi.ntwk_inst.ntwk_inst, 32);
		}

		if (
				((PGWC == cp_config->cp_type) || (SAEGWC == cp_config->cp_type)) &&
				(SOURCE_INTERFACE_VALUE_CORE ==
				bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {

			uint32_t size_ie = 0;

			size_ie = pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->update_pdr[i].pdi.header.len =
				pfcp_sess_mod_req->update_pdr[i].pdi.header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].header.len =
				pfcp_sess_mod_req->update_pdr[i].header.len - size_ie;
			pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.header.len = 0;

		} else {
			pfcp_sess_mod_req->update_pdr[i].pdi.local_fteid.ipv4_address =
				bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
		}

		pfcp_sess_mod_req->update_pdr[i].pdi.src_intfc.interface_value =
			bearer->pdrs[itr]->pdi.src_intfc.interface_value;

		fill_upd_bearer_sdf_rule(pfcp_sess_mod_req, bearer, i);
	}

	pfcp_sess_mod_req->update_pdr_count += NUMBER_OF_PDR_PER_BEARER;
	return;
}

/* REVIEW: Context will remove after merging */
uint32_t 
fill_pfcp_sess_mod_req( pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		gtpv2c_header_t *header, eps_bearer_t *bearer,
		pdn_connection_t *pdn, pfcp_update_far_ie_t update_far[], uint8_t x2_handover_flag)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;

	if( header != NULL)
		LOG_MSG(LOG_DEBUG, "TEID[%d]", header->teid.has_teid.teid);

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
					           HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	/*SP: This depends on condition in pcrf data(pcrf will send bar_rule_id if it needs to be delated). Need to handle after pcrf integration*/
	/* removing_bar(&(pfcp_sess_mod_req->remove_bar)); */

	//set create PDR

	/************************************************
	 *  cp_type  count     FTEID_1          FTEID_2 *
	 *************************************************
	 In case MBR received from MME:-
	 SGWC         1      enodeB               -
	 PGWC         -        -                  -
	 SAEGWC       1      enodeB               -
	 *************************************************
	 In case of CSResp received from PGWC to SGWC :-
	 SGWC <----CSResp--- PGWC
	 |
	 pfcp_sess_mod_req
	 |
	 v
	 SGWU
	 In above scenario:
	 count = 1 ,     FTEID_1 = s5s8 PGWU
	 ************************************************/
	/*SP: create pdr IE is not needed in session modification request , hence removing*/
	/*
	pfcp_sess_mod_req->create_pdr_count = 1;

	for( int i = 0; i < pfcp_sess_mod_req->create_pdr_count ; i++)
		creating_pdr(&(pfcp_sess_mod_req->create_pdr[i]));
	*/

	if (pfcp_sess_mod_req->create_pdr_count) {
		fill_pdr_far_qer_using_bearer(pfcp_sess_mod_req, bearer);
	}

	/*SP: This depends on condition  if the CP function requests the UP function to create a new BAR
	  Need to add condition to check if CP needs creation of BAR*/
	for( int i = 0; i < pfcp_sess_mod_req->create_pdr_count ; i++) {
		if((pfcp_sess_mod_req->create_pdr[i].header.len) &&
				(pfcp_sess_mod_req->create_pdr[i].far_id.header.len)){
			for( int j = 0; j < pfcp_sess_mod_req->create_far_count ; j++){
				if(pfcp_sess_mod_req->create_far[i].bar_id.header.len){
					/* TODO: Pass bar_id from pfcp_session_mod_req->create_far[i].bar_id.bar_id_value
					   to set bar_id*/
					creating_bar(&(pfcp_sess_mod_req->create_bar));
				}
			}
		}
	}

	/*SP: Adding FAR IE*/
    LOG_MSG(LOG_DEBUG,"pfcp sess mod req %d, bearer pdr count %d ", pfcp_sess_mod_req->update_far_count, bearer->pdr_count);
	uint8_t itr1 = 0; 
	for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
		if (SOURCE_INTERFACE_VALUE_ACCESS == bearer->pdrs[itr]->pdi.src_intfc.interface_value) {
            continue;
        }
		pdr_ctxt = bearer->pdrs[itr];
		updating_far(&(pfcp_sess_mod_req->update_far[itr1]));
        LOG_MSG(LOG_DEBUG,"updating FAR %d in modification message",itr1);
		pfcp_sess_mod_req->update_far[itr1].far_id.far_id_value = pdr_ctxt->far.far_id_value;
		pfcp_sess_mod_req->update_far[itr1].apply_action.forw = PRESENT;
        /* below block should be logically part of updating far */
		uint16_t len = 0;
		len += set_upd_forwarding_param(&(pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms));
		/* Currently take as hardcoded value */
		len += UPD_PARAM_HEADER_SIZE;
		pfcp_sess_mod_req->update_far[itr1].header.len += len;

		pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.outer_hdr_creation.teid =
			update_far[itr].upd_frwdng_parms.outer_hdr_creation.teid;
		pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.outer_hdr_creation.ipv4_address =
			(update_far[itr].upd_frwdng_parms.outer_hdr_creation.ipv4_address);
		pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.dst_intfc.interface_value =
			update_far[itr].upd_frwdng_parms.dst_intfc.interface_value;

		if(x2_handover_flag) {
			set_pfcpsmreqflags(&(pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.pfcpsmreq_flags));
			pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.pfcpsmreq_flags.sndem = 1;
			pfcp_sess_mod_req->update_far[itr1].header.len += sizeof(struct  pfcp_pfcpsmreq_flags_ie_t);
			pfcp_sess_mod_req->update_far[itr1].upd_frwdng_parms.header.len += sizeof(struct  pfcp_pfcpsmreq_flags_ie_t);
		}
        itr1++;
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->create_pdr_count){
				for(int itr = 0; itr < pfcp_sess_mod_req->create_pdr_count; itr++) {
					pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.teid =
						bearer->pdrs[itr]->pdi.local_fteid.teid ;
					/* TODO: Revisit this for change in yang */
					pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.ipv4_address =
						htonl(bearer->pdrs[itr]->pdi.ue_addr.ipv4_address);
					pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.ipv4_address =
						bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
					pfcp_sess_mod_req->create_pdr[itr].pdi.src_intfc.interface_value =
						bearer->pdrs[itr]->pdi.src_intfc.interface_value;
				}
			}
			break;

		case PGWC :
			break;

		default :
			LOG_MSG(LOG_DEBUG,"default pfcp sess mod req");
			break;
	}

	// set of update QER
	/*SP: No QER is not generated previously, No update needed*/
	/*
	pfcp_sess_mod_req->update_qer_count = bearer->qer_count;

	for(int i=0; i < pfcp_sess_mod_req->update_qer_count; i++ ){
		updating_qer(&(pfcp_sess_mod_req->update_qer[i]));
		pfcp_sess_mod_req->update_qer[i] == bearer->qer_id.qer.id;
	}
	*/

	// set of update BAR
	/*SP: If previously created BAR needs to be modified, this IE should be included*/
	/*
	 updating_bar(&(pfcp_sess_mod_req->update_bar));
	*/

	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	      excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}

	/*SP: This IE is included if node supports Partial failure handling support
	      excluding this IE since we dont have this support  */
	/*
	char sgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	unsigned long sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_mod_req->sgw_c_fqcsid), sgwc_value);

	char mme_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config.s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_mod_req->mme_fqcsid), mme_value);

	char pgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_mod_req->pgw_c_fqcsid), pgwc_value);

	//TODO : IP addres for epdgg is hardcoded
	const char* epdg_addr = "0.0.0.0";
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_mod_req->epdg_fqcsid), epdg_value);

	//TODO : IP addres for twan is hardcoded
	const char* twan_addr = "0.0.0.0";
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_mod_req->twan_fqcsid), twan_value);
	*/

	 /*SP: Not in use*/
	 /*
		set_up_inactivity_timer(&(pfcp_sess_mod_req->user_plane_inact_timer));
	 */

	/*SP: This IE is included if QAURR flag is set (this flag is in PFCPSMReq-Flags IE) or Query URR IE is present,
	  Adding check to exclud  this IE if any of these condition is not satisfied*/
	if(pfcp_sess_mod_req->pfcpsmreq_flags.qaurr ||
			pfcp_sess_mod_req->query_urr_count){
		set_query_urr_refernce(&(pfcp_sess_mod_req->query_urr_ref));
	}

	upf_ctx = (upf_context_t *) upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
    //if UPF context is available then dont set trace info.
    // its weird that we are still sending modify message out still no upf context.
    // message would go nowhere. But its easy to handle the failure
    if(upf_ctx != NULL) {
        if (upf_ctx->up_supp_features & UP_TRACE)
            set_trace_info(&(pfcp_sess_mod_req->trc_info));
    }

    return seq;
}

void
sdf_pkt_filter_to_string(flow_desc_t *flow,
		char *sdf_str , uint8_t direction)
{
        sdf_pkt_fltr_t *sdf_flow = &flow->sdf_flw_desc; 
        strcpy(sdf_str, flow->sdf_flow_description);
        LOG_MSG(LOG_NEVER, "sdf flow = %p direction = %d ", sdf_flow, direction);
#if 0
	char local_ip[INET_ADDRSTRLEN];
	char remote_ip[INET_ADDRSTRLEN];

	snprintf(local_ip, sizeof(local_ip), "%s",
			inet_ntoa(sdf_flow->local_ip_addr));
	snprintf(remote_ip, sizeof(remote_ip), "%s",
			inet_ntoa(sdf_flow->remote_ip_addr));

	if (direction == TFT_DIRECTION_DOWNLINK_ONLY) {
		snprintf(sdf_str, MAX_LEN, "%s/%"PRIu8" %s/%"PRIu8" %"
				PRIu16" : %"PRIu16" %"PRIu16" : %"PRIu16
				" 0x%"PRIx8"/0x%"PRIx8"",
				local_ip, sdf_flow->local_ip_mask, remote_ip,
				sdf_flow->remote_ip_mask,
				(sdf_flow->local_port_low),
				(sdf_flow->local_port_high),
				(sdf_flow->remote_port_low),
				(sdf_flow->remote_port_high),
				sdf_flow->proto_id, sdf_flow->proto_mask);
	} else if (direction == TFT_DIRECTION_UPLINK_ONLY) {
		snprintf(sdf_str, MAX_LEN, "%s/%"PRIu8" %s/%"PRIu8" %"
				PRIu16" : %"PRIu16" %"PRIu16" : %"PRIu16
				" 0x%"PRIx8"/0x%"PRIx8"",
				local_ip, sdf_flow->local_ip_mask, remote_ip,
				sdf_flow->remote_ip_mask,
				(sdf_flow->local_port_low),
				(sdf_flow->local_port_high),
				(sdf_flow->remote_port_low),
				(sdf_flow->remote_port_high),
				sdf_flow->proto_id, sdf_flow->proto_mask);
	}
#endif
}

void
fill_pdr_far_qer_using_bearer(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		eps_bearer_t *bearer)
{
	pfcp_sess_mod_req->create_pdr_count = bearer->pdr_count;
	uint32_t ue_ip_flags = 0; // TODO : analyze why we need to send ue ip address in pdi for pfcp session modify

	for(int i = 0; i < pfcp_sess_mod_req->create_pdr_count; i++) {
		pfcp_sess_mod_req->create_pdr[i].qer_id_count = 1;
		//pfcp_sess_mod_req->create_pdr[i].qer_id_count = bearer->qer_count;
		creating_pdr(&(pfcp_sess_mod_req->create_pdr[i]), bearer->pdrs[i]->pdi.src_intfc.interface_value, ue_ip_flags);
		pfcp_sess_mod_req->create_far_count++;
		creating_far(&(pfcp_sess_mod_req->create_far[i]));
	}

	for(int itr = 0; itr < pfcp_sess_mod_req->create_pdr_count ; itr++) {
		pfcp_sess_mod_req->create_pdr[itr].pdr_id.rule_id  =
			bearer->pdrs[itr]->rule_id;
		pfcp_sess_mod_req->create_pdr[itr].far_id.far_id_value =
			bearer->pdrs[itr]->far.far_id_value;
		pfcp_sess_mod_req->create_pdr[itr].precedence.prcdnc_val =
			bearer->pdrs[itr]->prcdnc_val;

		pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.teid =
			bearer->pdrs[itr]->pdi.local_fteid.teid;

		if((cp_config->cp_type == SGWC) ||
				(bearer->pdrs[itr]->pdi.src_intfc.interface_value ==
				SOURCE_INTERFACE_VALUE_ACCESS)) {
			/*No need to send ue ip and network instance for pgwc access interface or
			 * for any sgwc interface */
			uint32_t size_ie = 0;
			size_ie = pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.header.len +
				sizeof(pfcp_ie_header_t);
			size_ie = size_ie + pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->create_pdr[itr].pdi.header.len =
				pfcp_sess_mod_req->create_pdr[itr].pdi.header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].header.len =
				pfcp_sess_mod_req->create_pdr[itr].header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.header.len = 0;
			pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.header.len = 0;
		}else{
			pfcp_sess_mod_req->create_pdr[itr].pdi.ue_ip_address.ipv4_address =
				bearer->pdrs[itr]->pdi.ue_addr.ipv4_address;
			strncpy((char *)pfcp_sess_mod_req->create_pdr[itr].pdi.ntwk_inst.ntwk_inst,
				(char *)&bearer->pdrs[itr]->pdi.ntwk_inst.ntwk_inst, 32);
		}

		if ( ((PGWC == cp_config->cp_type) || (SAEGWC == cp_config->cp_type)) &&
				(SOURCE_INTERFACE_VALUE_CORE ==
				bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {

			uint32_t size_ie = 0;

			size_ie = pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.header.len +
				sizeof(pfcp_ie_header_t);
			pfcp_sess_mod_req->create_pdr[itr].pdi.header.len =
				pfcp_sess_mod_req->create_pdr[itr].pdi.header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].header.len =
				pfcp_sess_mod_req->create_pdr[itr].header.len - size_ie;
			pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.header.len = 0;

		} else {
			pfcp_sess_mod_req->create_pdr[itr].pdi.local_fteid.ipv4_address =
				bearer->pdrs[itr]->pdi.local_fteid.ipv4_address;
		}

		pfcp_sess_mod_req->create_pdr[itr].pdi.src_intfc.interface_value =
			bearer->pdrs[itr]->pdi.src_intfc.interface_value;

		pfcp_sess_mod_req->create_far[itr].far_id.far_id_value =
			bearer->pdrs[itr]->far.far_id_value;

        if(cp_config->gx_enabled) {
            if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
                pfcp_sess_mod_req->create_pdr[itr].qer_id_count =
                    bearer->pdrs[itr]->qer_id_count;
                for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_pdr[itr].qer_id_count; itr1++) {
                    pfcp_sess_mod_req->create_pdr[itr].qer_id[itr1].qer_id_value =
                        bearer->pdrs[itr]->qer_id[itr1].qer_id;
                }
            }
        }

		if ((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) {
			pfcp_sess_mod_req->create_far[itr].apply_action.forw = PRESENT;
			if (pfcp_sess_mod_req->create_far[itr].apply_action.forw == PRESENT) {
				uint16_t len = 0;

				if (
						(SAEGWC == cp_config->cp_type) ||
						(SOURCE_INTERFACE_VALUE_ACCESS ==
						 bearer->pdrs[itr]->pdi.src_intfc.interface_value)) {
					set_destination_interface(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc));
					pfcp_set_ie_header(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms.header),
							PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

					pfcp_sess_mod_req->create_far[itr].frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

					len += sizeof(pfcp_dst_intfc_ie_t);
					len += UPD_PARAM_HEADER_SIZE;

					pfcp_sess_mod_req->create_far[itr].header.len += len;

					pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc.interface_value =
						bearer->pdrs[itr]->far.dst_intfc.interface_value;
				} else {
					pfcp_sess_mod_req->create_far[itr].apply_action.forw = NO_FORW_ACTION;
				}
			}
		} else
		if ((SGWC == cp_config->cp_type) &&
			(DESTINATION_INTERFACE_VALUE_CORE ==
			 bearer->pdrs[itr]->far.dst_intfc.interface_value) &&
			(bearer->s5s8_pgw_gtpu_teid != 0) &&
			(bearer->s5s8_pgw_gtpu_ipv4.s_addr != 0))
		{
			uint16_t len = 0;
			len += set_forwarding_param(&(pfcp_sess_mod_req->create_far[itr].frwdng_parms));
			/* Currently take as hardcoded value */
			len += UPD_PARAM_HEADER_SIZE;
			pfcp_sess_mod_req->create_far[itr].header.len += len;

			pfcp_sess_mod_req->create_far[itr].apply_action.forw = PRESENT;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.outer_hdr_creation.ipv4_address =
					bearer->pdrs[itr]->far.outer_hdr_creation.ipv4_address;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.outer_hdr_creation.teid =
					bearer->pdrs[itr]->far.outer_hdr_creation.teid;
			pfcp_sess_mod_req->create_far[itr].frwdng_parms.dst_intfc.interface_value =
					bearer->pdrs[itr]->far.dst_intfc.interface_value;
		}

	} /*for loop*/

    if(cp_config->gx_enabled) {
        if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC){
            pfcp_sess_mod_req->create_qer_count = bearer->qer_count;
            qer_t *qer_context = NULL;
            for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_qer_count ; itr1++) {
                creating_qer(&(pfcp_sess_mod_req->create_qer[itr1]));
                pfcp_sess_mod_req->create_qer[itr1].qer_id.qer_id_value  =
                    bearer->qer_id[itr1].qer_id;
                qer_context = (qer_t*)get_qer_entry(pfcp_sess_mod_req->create_qer[itr1].qer_id.qer_id_value);
                /* Assign the value from the PDR */
                if(qer_context){
                    pfcp_sess_mod_req->create_qer[itr1].maximum_bitrate.ul_mbr  =
                        qer_context->max_bitrate.ul_mbr;
                    pfcp_sess_mod_req->create_qer[itr1].maximum_bitrate.dl_mbr  =
                        qer_context->max_bitrate.dl_mbr;
                    pfcp_sess_mod_req->create_qer[itr1].guaranteed_bitrate.ul_gbr  =
                        qer_context->guaranteed_bitrate.ul_gbr;
                    pfcp_sess_mod_req->create_qer[itr1].guaranteed_bitrate.dl_gbr  =
                        qer_context->guaranteed_bitrate.dl_gbr;
                }
            }

            for(int itr1 = 0; itr1 < pfcp_sess_mod_req->create_pdr_count ; itr1++) {
                fill_sdf_rules_modification(pfcp_sess_mod_req, bearer, itr1);
            }
        }
    }

}

void fill_gate_status(pfcp_sess_estab_req_t *pfcp_sess_est_req,
	int qer_counter,
	enum flow_status f_status)
{
	switch(f_status)
	{
		case FL_ENABLED_UPLINK:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_OPEN;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_CLOSED;
			break;

		case FL_ENABLED_DOWNLINK:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_CLOSED;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_OPEN;
			break;

		case FL_ENABLED:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_OPEN;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_OPEN;
			break;

		case FL_DISABLED:
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.ul_gate  = UL_GATE_CLOSED;
			pfcp_sess_est_req->create_qer[qer_counter].gate_status.dl_gate  = UL_GATE_CLOSED;
			break;
		case FL_REMOVED:
			/*TODO*/
			break;
	}
}

void sdf_pkt_filter_add(pfcp_sess_estab_req_t* pfcp_sess_est_req,
		dynamic_rule_t *dynamic_rules,
		int pdr_counter,
		int sdf_filter_count,
		int flow_cnt,
		uint8_t direction)
{
	int len = 0;
	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
	sdf_pkt_filter_to_string(&(dynamic_rules->flow_desc[flow_cnt]),
			(char*)(pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pfcp_sess_est_req->create_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pfcp_sess_est_req->create_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}

void sdf_pkt_filter_mod(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
		eps_bearer_t* bearer,
		int pdr_counter,
		int sdf_filter_count,
		int dynamic_filter_cnt,
		int flow_cnt,
		uint8_t direction)
{
	int len = 0;
	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].fd = 1;
	sdf_pkt_filter_to_string(&(bearer->dynamic_rules[dynamic_filter_cnt]->flow_desc[flow_cnt]),
			(char*)(pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pfcp_sess_mod_req->create_pdr[pdr_counter].header.len += (len + sizeof(pfcp_ie_header_t));
}
void sdf_pkt_filter_gx_mod(pfcp_create_pdr_ie_t *pdr, dynamic_rule_t *dyn_rule, int sdf_filter_count, int flow_cnt, uint8_t direction)
{
	int len = 0;
	pdr->pdi.sdf_filter[sdf_filter_count].fd = 1;
    LOG_MSG(LOG_DEBUG,"dyn_rule->flow_desc[flow_cnt].sdf_flow_description = %s ", dyn_rule->flow_desc[flow_cnt].sdf_flow_description);
	sdf_pkt_filter_to_string(&(dyn_rule->flow_desc[flow_cnt]),
			(char*)(pdr->pdi.sdf_filter[sdf_filter_count].flow_desc), direction);

	pdr->pdi.sdf_filter[sdf_filter_count].len_of_flow_desc =
		strlen((char*)(&pdr->pdi.sdf_filter[sdf_filter_count].flow_desc));

	len += FLAG_LEN;
	len += sizeof(uint16_t);
	len += pdr->pdi.sdf_filter[sdf_filter_count].len_of_flow_desc;

	pfcp_set_ie_header(
			&(pdr->pdi.sdf_filter[sdf_filter_count].header), PFCP_IE_SDF_FILTER, len);

	/*VG updated the header len of pdi as sdf rules has been added*/
	pdr->pdi.header.len += (len + sizeof(pfcp_ie_header_t));
	pdr->header.len += (len + sizeof(pfcp_ie_header_t));
}
int fill_sdf_rules_modification(pfcp_sess_mod_req_t* pfcp_sess_mod_req,
	eps_bearer_t* bearer,
	int pdr_counter)
{
	int ret = 0;
	int sdf_filter_count = 0;
	/*VG convert pkt_filter_strucutre to char string*/
	for(int index = 0; index < bearer->num_dynamic_filters; index++) {

		pfcp_sess_mod_req->create_pdr[pdr_counter].precedence.prcdnc_val = bearer->dynamic_rules[index]->precedence;
		// itr is for flow information counter
		// sdf_filter_count is for SDF information counter
		for(int itr = 0; itr < bearer->dynamic_rules[index]->num_flw_desc; itr++) {

			if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {

				if((pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
						((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
						 (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

					sdf_pkt_filter_mod(
							pfcp_sess_mod_req, bearer, pdr_counter, sdf_filter_count, index, itr, TFT_DIRECTION_UPLINK_ONLY);
					sdf_filter_count++;
				}

			} else {
				LOG_MSG(LOG_ERROR, "Empty SDF rules");
			}

			if(bearer->dynamic_rules[index]->flow_desc[itr].sdf_flow_description != NULL) {
				if((pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
						((bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
						 (bearer->dynamic_rules[index]->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
					sdf_pkt_filter_mod(
							pfcp_sess_mod_req, bearer, pdr_counter, sdf_filter_count, index, itr, TFT_DIRECTION_DOWNLINK_ONLY);
					sdf_filter_count++;
				}
			} else {
				LOG_MSG(LOG_ERROR, "Empty SDF rules");
			}
		}

		pfcp_sess_mod_req->create_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;

	}
	return ret;
}

int fill_sdf_rules(pfcp_sess_estab_req_t* pfcp_sess_est_req,
	dynamic_rule_t *dynamic_rules,
	int pdr_counter)
{
    int ret = 0;
    int sdf_filter_count = 0;
    /*VG convert pkt_filter_strucutre to char string*/
    //pfcp_sess_est_req->create_pdr[pdr_counter].precedence.prcdnc_val = dynamic_rules->precedence;
    // itr is for flow information counter
    // sdf_filter_count is for SDF information counter
    for(int itr = 0; itr < dynamic_rules->num_flw_desc; itr++) {
        if(dynamic_rules->flow_desc[itr].sdf_flow_description != NULL) {
            if((pfcp_sess_est_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS) &&
                    ((dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_UPLINK_ONLY) ||
                     (dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {

                sdf_pkt_filter_add(
                        pfcp_sess_est_req, dynamic_rules, pdr_counter, sdf_filter_count, itr, TFT_DIRECTION_UPLINK_ONLY);
                sdf_filter_count++;
            }
        } else {
            LOG_MSG(LOG_ERROR, "Empty SDF rules");
        }

        if(dynamic_rules->flow_desc[itr].sdf_flow_description != NULL) {
            if((pfcp_sess_est_req->create_pdr[pdr_counter].pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE) &&
                    ((dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_DOWNLINK_ONLY) ||
                     (dynamic_rules->flow_desc[itr].sdf_flw_desc.direction == TFT_DIRECTION_BIDIRECTIONAL))) {
                sdf_pkt_filter_add(
                        pfcp_sess_est_req, dynamic_rules, pdr_counter, sdf_filter_count, itr, TFT_DIRECTION_DOWNLINK_ONLY);
                sdf_filter_count++;
            } 
        } else {
            LOG_MSG(LOG_ERROR, "Empty SDF rules");
        }
    }

    pfcp_sess_est_req->create_pdr[pdr_counter].pdi.sdf_filter_count = sdf_filter_count;
    return ret;
}

//Create qer and add it to global table 
int
fill_qer_entry(pdn_connection_t *pdn, eps_bearer_t *bearer, uint32_t qer_id, bool apnAmbr, dynamic_rule_t *dyn_rule)
{
	int ret = -1;
	qer_t *qer_ctxt = NULL;
	qer_ctxt = (qer_t*)calloc(1, sizeof(qer_t));
	if (qer_ctxt == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate qer context ");
		return ret;
	}
	qer_ctxt->qer_id = qer_id;
	qer_ctxt->qci = bearer->qos.qci;
    if(apnAmbr == true) {
        qer_ctxt->max_bitrate.ul_mbr = pdn->apn_ambr.ambr_uplink;
	    qer_ctxt->max_bitrate.dl_mbr = pdn->apn_ambr.ambr_downlink;
    } else {
	    qer_ctxt->max_bitrate.ul_mbr = dyn_rule->qos.ul_mbr;
	    qer_ctxt->max_bitrate.dl_mbr = dyn_rule->qos.dl_mbr;
	    qer_ctxt->guaranteed_bitrate.ul_gbr = dyn_rule->qos.ul_gbr;
	    qer_ctxt->guaranteed_bitrate.dl_gbr = dyn_rule->qos.dl_gbr;
    }

	ret = add_qer_entry(qer_ctxt->qer_id,qer_ctxt);
	if(ret != 0) {
		LOG_MSG(LOG_ERROR, "Adding qer entry Error: %d ", ret);
		return ret;
	}

	return ret;
}


pdr_t *
fill_pdr_entry(ue_context_t *context, pdn_connection_t *pdn,
		eps_bearer_t *bearer, uint8_t iface, uint8_t itr, uint32_t qer_id, uint32_t urr_id)
{
	char mnc[4] = {0};
	char mcc[4] = {0};
	char nwinst[32] = {0};
	pdr_t *pdr_ctxt = NULL;
	int ret;

	if (context->serving_nw.mnc_digit_3 == 15) {
		sprintf(mnc, "0%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2);
	} else {
		sprintf(mnc, "%u%u%u", context->serving_nw.mnc_digit_1,
				context->serving_nw.mnc_digit_2,
				context->serving_nw.mnc_digit_3);
	}

	sprintf(mcc, "%u%u%u", context->serving_nw.mcc_digit_1,
			context->serving_nw.mcc_digit_2,
			context->serving_nw.mcc_digit_3);

	sprintf(nwinst, "mnc%s.mcc%s", mnc, mcc);

	pdr_ctxt = (pdr_t *)calloc(1, sizeof(pdr_t));
	if (pdr_ctxt == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate pdr");
		return NULL;
	}
	memset(pdr_ctxt,0,sizeof(pdr_t));

	pdr_ctxt->rule_id =  generate_pdr_id();
	pdr_ctxt->prcdnc_val =  1; // FIXME : why this is 1 here ?
	pdr_ctxt->far.far_id_value = generate_far_id();
	pdr_ctxt->session_id = pdn->seid; // FIXME - why this ?
	/*to be filled in fill_sdf_rule*/
	pdr_ctxt->pdi.sdf_filter_cnt = 0;
	pdr_ctxt->pdi.src_intfc.interface_value = iface;
	strncpy((char * )pdr_ctxt->pdi.ntwk_inst.ntwk_inst, (char *)nwinst, 32);

	/* TODO: NS Add this changes after DP related changes of VS
	 * if(cp_config->cp_type != SGWC){
	 * pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;
	 * }
	 */

    pdr_ctxt->qer_id_count = 0;
	if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC) {
		/* TODO Hardcode 1 set because one PDR contain only 1 QER entry
		 * Revist again in case of multiple rule support
		 */
        LOG_MSG(LOG_DEBUG,"Bearer QER count = %d ", bearer->qer_count);
        pdr_ctxt->qer_id[pdr_ctxt->qer_id_count].qer_id = qer_id; 
        LOG_MSG(LOG_DEBUG,"PDR %d adding QER %x ", pdr_ctxt->rule_id, bearer->qer_id[pdr_ctxt->qer_id_count].qer_id);
		pdr_ctxt->qer_id_count++;
        for(int count=0; count < bearer->qer_count; count++)
        {
            if(isCommonQER(bearer->qer_id[count].qer_id)) {
                LOG_MSG(LOG_DEBUG,"PDR %d adding common QER %x ", pdr_ctxt->rule_id, bearer->qer_id[count].qer_id);
                pdr_ctxt->qer_id[pdr_ctxt->qer_id_count].qer_id = bearer->qer_id[count].qer_id;
		        pdr_ctxt->qer_id_count++;
            }
        }
        LOG_MSG(LOG_DEBUG,"PDR %d and QER added count = %d ", pdr_ctxt->rule_id, pdr_ctxt->qer_id_count);
	}

	pdr_ctxt->urr_id_count = 0;
	if ((cp_config->urr_enable == 1) && (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC)) {
		/* TODO Hardcode 1 set because one PDR contain only 1 QER entry
		 * Revist again in case of multiple rule support
		 */
        pdr_ctxt->urr_id[pdr_ctxt->urr_id_count].urr_id = urr_id;
		pdr_ctxt->urr_id_count++;
        for(int count=0; count < bearer->urr_count; count++)
        {
            if(isCommonURR(bearer->urr_id[count].urr_id)) {
                pdr_ctxt->urr_id[pdr_ctxt->urr_id_count].urr_id = bearer->urr_id[count].urr_id;
			    pdr_ctxt->urr_id_count++;
            }
        }
	}

	pdr_ctxt->pdi.ue_addr.ipv4_address = pdn->ipv4.s_addr;

	if (iface == SOURCE_INTERFACE_VALUE_ACCESS) {
		pdr_ctxt->pdi.local_fteid.teid = bearer->s1u_sgw_gtpu_teid;
		pdr_ctxt->pdi.local_fteid.ipv4_address = 0;

		if ((SGWC == cp_config->cp_type) &&
				(bearer->s5s8_pgw_gtpu_ipv4.s_addr != 0) &&
				(bearer->s5s8_pgw_gtpu_teid != 0)) {
			pdr_ctxt->far.actions.forw = 2;
			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
			pdr_ctxt->far.outer_hdr_creation.ipv4_address =
				bearer->s5s8_pgw_gtpu_ipv4.s_addr;
			pdr_ctxt->far.outer_hdr_creation.teid =
				bearer->s5s8_pgw_gtpu_teid;
		} else {
			pdr_ctxt->far.actions.forw = 0;
		}

		if ((cp_config->cp_type == PGWC) ||
				(SAEGWC == cp_config->cp_type)) {
			pdr_ctxt->far.dst_intfc.interface_value =
				DESTINATION_INTERFACE_VALUE_CORE;
		}
	} else {
		if(cp_config->cp_type == SGWC){
			pdr_ctxt->pdi.local_fteid.teid = (bearer->s5s8_sgw_gtpu_teid);
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
		}else{
			pdr_ctxt->pdi.local_fteid.teid = 0;
            //CONFUSION - where are we filling ipv4_address ?
			pdr_ctxt->pdi.local_fteid.ipv4_address = 0;
			pdr_ctxt->far.actions.forw = 0;
			if(cp_config->cp_type == PGWC){
				pdr_ctxt->far.outer_hdr_creation.ipv4_address =
					bearer->s5s8_sgw_gtpu_ipv4.s_addr;
				pdr_ctxt->far.outer_hdr_creation.teid =
					bearer->s5s8_sgw_gtpu_teid;
				pdr_ctxt->far.dst_intfc.interface_value =
					DESTINATION_INTERFACE_VALUE_ACCESS;
			}
		}
	}

    LOG_MSG(LOG_DEBUG,"PDR at bearer index %d - PDR Id = %d , pdr_ctx = %p, QER count = %d , URR count = %d ", itr, pdr_ctxt->rule_id, pdr_ctxt, pdr_ctxt->qer_id_count, pdr_ctxt->urr_id_count);
	bearer->pdrs[itr] = pdr_ctxt;
	ret = add_pdr_entry(bearer->pdrs[itr]->rule_id, bearer->pdrs[itr]);
	if ( ret != 0) {
		LOG_MSG(LOG_ERROR, "Adding pdr entry Error: %d ", ret);
		return NULL;
	}
	return pdr_ctxt;
}



/**
 * Get PDR entry from PDR hash table.
 * update entry
 */
int
update_pdr_teid(eps_bearer_t *bearer, uint32_t teid, uint32_t ip, uint8_t iface)
{
	int ret = -1;

    // FIXME : application filtering
    LOG_MSG(LOG_DEBUG, "update_pdr_teid bearer->pdr_count %d ", bearer->pdr_count);
	for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
		if(bearer->pdrs[itr]->pdi.src_intfc.interface_value == iface){
			bearer->pdrs[itr]->pdi.local_fteid.teid = teid;
			bearer->pdrs[itr]->pdi.local_fteid.ipv4_address = htonl(ip);
			LOG_MSG(LOG_DEBUG, "Updated pdr entry successfully for bearer (%d) and PDR_ID:%u",
					bearer->eps_bearer_id, bearer->pdrs[itr]->rule_id);
		}
	}
	return ret;
}

void
fill_pfcp_sess_est_req( pfcp_sess_estab_req_t *pfcp_sess_est_req,
		pdn_connection_t *pdn, uint32_t seq)
{
	/*TODO :generate seid value and store this in array
	  to send response from cp/dp , first check seid is there in array or not if yes then
	  fill that seid in response and if not then seid =0 */

	int ret = 0;
	eps_bearer_t *bearer = NULL;
	upf_context_t *upf_ctx = NULL;

    // FIXME PERFORMANCE : why we are doing lookup ? Just use it from pdn  
	upf_ctx = (upf_context_t *) upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
    if(upf_ctx == NULL) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
        //FIXME : return error 
		return;
	}
	assert(upf_ctx != NULL);

	memset(pfcp_sess_est_req, 0, sizeof(pfcp_sess_estab_req_t));

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_est_req->header), PFCP_SESSION_ESTABLISHMENT_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_est_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	char pAddr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);
	set_node_id(&(pfcp_sess_est_req->node_id), node_value);

	set_fseid(&(pfcp_sess_est_req->cp_fseid), pdn->seid, node_value);

	set_pdn_type(&(pfcp_sess_est_req->pdn_type), &(pdn->pdn_type));

	if ((cp_config->cp_type == PGWC) || (cp_config->cp_type == SAEGWC))
	{
        // 2 PDRs per rule 
		/*
		 * For pgw create pdr, far and qer while handling pfcp messages
		 */
        pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);

        while (pcc_rule != NULL) {
            TAILQ_REMOVE(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
            LOG_MSG(LOG_DEBUG, "Rule %s dynamic_rules->num_flw_desc -  %d ", pcc_rule->dyn_rule->rule_name, pcc_rule->dyn_rule->num_flw_desc);

			bearer = NULL;
            //FIXME : why this does not work..
			if(compare_default_bearer_qos(&pdn->policy.default_bearer_qos, &pcc_rule->dyn_rule->qos) == 0)
			{
				LOG_MSG(LOG_DEBUG, "Found matching default bearer ");
				 // This means this Dynamic rule going to install in default bearer
				bearer = get_default_bearer(pdn);
			}
			else
			{
				uint8_t bearer_id = 0;
				bearer = get_bearer(pdn, &pcc_rule->dyn_rule->qos);
				if(bearer == NULL) {
					// create dedicated bearer
				    LOG_MSG(LOG_DEBUG, "Need to create dedicated bearer ");
					bearer = (eps_bearer_t *)calloc(1, sizeof(eps_bearer_t));
					if(bearer == NULL) {
						LOG_MSG(LOG_ERROR, "Failure to allocate bearer ");
						return;
						/* return GTPV2C_CAUSE_SYSTEM_FAILURE; */
					}
					bzero(bearer,  sizeof(eps_bearer_t));
					bearer->pdn = pdn;
					bearer_id = get_new_bearer_id(pdn);
					pdn->eps_bearers[bearer_id] = bearer;
					pdn->context->eps_bearers[bearer_id] = bearer;
					pdn->num_bearer++;
					set_s5s8_pgw_gtpu_teid_using_pdn(bearer, pdn);
					memcpy(&(bearer->qos), &(pcc_rule->dyn_rule->qos), sizeof(bearer_qos_ie));
					fill_dedicated_bearer_info(bearer, pdn->context, pdn);
				} else {
				    LOG_MSG(LOG_DEBUG, "Use existing bearer %d to install pcc rules", bearer->eps_bearer_id);
                }
			}

			bearer->dynamic_rules[bearer->num_dynamic_filters] = pcc_rule->dyn_rule;
            // Adding APN AMBR QER in the bearer only if filters are more than 1 
            // FIXME : if Gx is enabled but only 1 rule then do we need common QER ?
            if(cp_config->gx_enabled) {
                eps_bearer_t *bearer = get_default_bearer(pdn);
                if (bearer->ambr_qer_flag == false) {
                    uint32_t qer_id = generate_qer_id(SOURCE_INTERFACE_VALUE_UNKNOWN);
                    bearer->qer_id[bearer->qer_count].qer_id = qer_id; 
                    LOG_MSG(LOG_DEBUG,"Adding default APN AMBR QER. Current QER Count %d, New QER ID = %d  ", bearer->qer_count, bearer->qer_id[bearer->qer_count].qer_id);
                    fill_qer_entry(pdn, bearer, qer_id, true, NULL);
                    bearer->qer_count++;
                    bearer->ambr_qer_flag = true;
                }
            }

            /* URR_SUPPORT : generate URR ids and keep it in the default bearer */
			uint32_t urr_id = generate_urr_id(SOURCE_INTERFACE_VALUE_ACCESS);
			bearer->urr_id[bearer->urr_count].urr_id = urr_id;

			uint32_t qer_id  = generate_qer_id(SOURCE_INTERFACE_VALUE_ACCESS);
			bearer->qer_id[bearer->qer_count].qer_id = qer_id; 
			fill_qer_entry(pdn, bearer, qer_id, false, pcc_rule->dyn_rule);


            // Fill 1st PDR entry and corresponding rules  
            pdr_t *temp_pdr = fill_pdr_entry(pdn->context, pdn, bearer, SOURCE_INTERFACE_VALUE_ACCESS, bearer->pdr_count, qer_id, urr_id);
            temp_pdr->prcdnc_val = pcc_rule->dyn_rule->precedence; 
			pcc_rule->dyn_rule->pdr[0] = temp_pdr; 
            temp_pdr->dynamic_rule = pcc_rule->dyn_rule; 



			// assuming no of qer and pdr is same
            bearer->urr_count++;
            bearer->qer_count++;
            bearer->pdr_count++;

            LOG_MSG(LOG_DEBUG,"First URR count = %d QER count = %d PDR count= %d",bearer->urr_count, bearer->qer_count, bearer->pdr_count);

            // Fill 2nd PDR entry and corresponding rules  

			urr_id = generate_urr_id(SOURCE_INTERFACE_VALUE_CORE);
			bearer->urr_id[bearer->urr_count].urr_id = urr_id; 

			qer_id = generate_qer_id(SOURCE_INTERFACE_VALUE_CORE);
			bearer->qer_id[bearer->qer_count].qer_id = qer_id; 
			fill_qer_entry(pdn, bearer, qer_id, false, pcc_rule->dyn_rule);

			temp_pdr = fill_pdr_entry(pdn->context, pdn, bearer, SOURCE_INTERFACE_VALUE_CORE, bearer->pdr_count, qer_id, urr_id);
            temp_pdr->prcdnc_val = pcc_rule->dyn_rule->precedence; 
			pcc_rule->dyn_rule->pdr[1] = temp_pdr; 
            temp_pdr->dynamic_rule = pcc_rule->dyn_rule; 

            bearer->urr_count++;
            bearer->qer_count++;
            bearer->pdr_count++;

			bearer->num_dynamic_filters++;

            LOG_MSG(LOG_DEBUG,"Second URR count = %d QER count = %d PDR count = %d",bearer->urr_count, bearer->qer_count, bearer->pdr_count);

			//Adding rule and bearer id to a hash
			bearer_id_t *id;
			id = (bearer_id_t *)malloc(sizeof(bearer_id_t));
			memset(id, 0 , sizeof(bearer_id_t));
			rule_name_key_t key = {0};
			id->bearer_id = bearer->eps_bearer_id;
			strncpy(key.rule_name, pcc_rule->dyn_rule->rule_name, strlen(pcc_rule->dyn_rule->rule_name));
			sprintf(key.rule_name, "%s%d", key.rule_name, pdn->call_id);
            // FIXME : do we need to check rule presence 
			if (add_rule_name_entry(key.rule_name, bearer->eps_bearer_id) != 0) {
				LOG_MSG(LOG_ERROR,"Failed to add rule name %s ", key.rule_name);
				return;
			}

		    pfcp_sess_est_req->create_pdr_count += 2; // 1 for downlink, 1 for uplink
            free(pcc_rule); // pcc rule was just container, actual rule is not moved to bearer.
            pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
		}

        // FIXME : we should put it in above loops 
		if (cp_config->cp_type == SAEGWC) {
			update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		} else if (cp_config->cp_type == PGWC) {
			update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		}
	} else {
		bearer = get_default_bearer(pdn);
		pfcp_sess_est_req->create_pdr_count = bearer->pdr_count;
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
		update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, upf_ctx->s5s8_sgwu_ip, SOURCE_INTERFACE_VALUE_CORE);
	}

	{
        eps_bearer_t *bearer;
		uint8_t pdr_idx =0;
		for(uint8_t i = 0; i < MAX_BEARERS; i++)
		{
			bearer = pdn->eps_bearers[i];
			if(bearer == NULL)
                continue;

            for(uint8_t idx = 0; idx < bearer->pdr_count; idx++)
			{
                uint32_t ue_ip_flags = 0;
                pdr_t *pdr = bearer->pdrs[idx];
				assert(pdr != NULL);
                struct dynamic_rule *dyn_rule = pdr->dynamic_rule;

                // FIXME : move this code down to the place where we are filling IDs
                pfcp_sess_est_req->create_pdr[pdr_idx].urr_id_count = 0;
                if(cp_config->urr_enable == 1) {
                    pfcp_sess_est_req->create_pdr[pdr_idx].urr_id_count = 1;
                }

                pfcp_sess_est_req->create_pdr[pdr_idx].qer_id_count = pdr->qer_id_count;

                if(IF_PDN_ADDR_ALLOC_CONTROL(pdn)) {
                  ue_ip_flags = PDN_ADDR_ALLOC_CONTROL;
                } else {
                  ue_ip_flags = PDN_ADDR_ALLOC_UPF;
                }
                creating_pdr(&(pfcp_sess_est_req->create_pdr[pdr_idx]), pdr->pdi.src_intfc.interface_value, ue_ip_flags);

                pfcp_sess_est_req->create_far_count++;
                creating_far(&(pfcp_sess_est_req->create_far[pdr_idx]));

				pfcp_sess_est_req->create_pdr[pdr_idx].pdr_id.rule_id  = pdr->rule_id;
				pfcp_sess_est_req->create_pdr[pdr_idx].precedence.prcdnc_val = pdr->prcdnc_val;

                // FIXME : add comment why we are doing this 
				if(((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) &&
						(pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_CORE)) {
					uint32_t size_teid = 0;
					size_teid = pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.header.len + sizeof(pfcp_ie_header_t);

					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.header.len = 0;

					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len =
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len - size_teid;

					pfcp_sess_est_req->create_pdr[pdr_idx].header.len =
					pfcp_sess_est_req->create_pdr[pdr_idx].header.len - size_teid;


				} else {
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.teid = pdr->pdi.local_fteid.teid;
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.local_fteid.ipv4_address = pdr->pdi.local_fteid.ipv4_address;
				}

				if((cp_config->cp_type == SGWC) || (pdr->pdi.src_intfc.interface_value == SOURCE_INTERFACE_VALUE_ACCESS)) {
					/*No need to send ue ip and network instance for pgwc access interface or
					 * for any sgwc interface */
					uint32_t size_ie = 0;
					size_ie = pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.header.len +
						sizeof(pfcp_ie_header_t);
					size_ie = size_ie + pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.header.len +
						sizeof(pfcp_ie_header_t);
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len =
						pfcp_sess_est_req->create_pdr[pdr_idx].pdi.header.len - size_ie;
					pfcp_sess_est_req->create_pdr[pdr_idx].header.len =
						pfcp_sess_est_req->create_pdr[pdr_idx].header.len - size_ie;
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.header.len = 0;
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.header.len = 0;
				} else {
					pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ue_ip_address.ipv4_address =
						pdr->pdi.ue_addr.ipv4_address;
					strncpy((char *)pfcp_sess_est_req->create_pdr[pdr_idx].pdi.ntwk_inst.ntwk_inst,
							(char *)&pdr->pdi.ntwk_inst.ntwk_inst, 32);
				}

				pfcp_sess_est_req->create_pdr[pdr_idx].pdi.src_intfc.interface_value =
					pdr->pdi.src_intfc.interface_value;

                //ADD FAR id in PDR, and FAR should be created with farid 
                // FIXME : move this code where we are referring to FAR
				pfcp_sess_est_req->create_far[pdr_idx].far_id.far_id_value =
					pdr->far.far_id_value;
				pfcp_sess_est_req->create_pdr[pdr_idx].far_id.far_id_value =
					 pdr->far.far_id_value;

		        fill_sdf_rules(pfcp_sess_est_req, dyn_rule, pdr_idx);

                // filling qer id in PDR in the pdrpfcp_sess_est_req
				if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC) {
                    LOG_MSG(LOG_DEBUG, "Number of QERs on PDR %d are %d ", pdr->rule_id, pdr->qer_id_count);
					pfcp_sess_est_req->create_pdr[pdr_idx].qer_id_count = pdr->qer_id_count;
					for(int itr1 = 0; itr1 < pdr->qer_id_count; itr1++) {
						pfcp_sess_est_req->create_pdr[pdr_idx].qer_id[itr1].qer_id_value = pdr->qer_id[itr1].qer_id;
                        LOG_MSG(LOG_DEBUG,"\tAdding QER %d in PDR-ID %d ", pdr->qer_id[itr1].qer_id, pdr->rule_id);
					}
		            uint8_t qer_idx = pfcp_sess_est_req->create_qer_count;
                    for(uint8_t qer_itr = 0; qer_itr < pdr->qer_id_count; qer_itr++) {
                        qer_t *qer_context = (qer_t*)get_qer_entry(pdr->qer_id[qer_itr].qer_id);
                        //check if this QER is already added in Session Est 
                        bool found = false;
                        for(int id=0; id < qer_idx; id++) {
                            if(pfcp_sess_est_req->create_qer[id].qer_id.qer_id_value == qer_context->qer_id) {
                                found = true;
                            }
                        }

                        if (found == true) {
                            continue;
                        }

                        creating_qer(&(pfcp_sess_est_req->create_qer[qer_idx]));
			            fill_gate_status(pfcp_sess_est_req, qer_idx, (enum flow_status)dyn_rule->flow_status);
                        pfcp_sess_est_req->create_qer[qer_idx].qer_id.qer_id_value  = pdr->qer_id[qer_itr].qer_id;
                        /* Assign the value from the PDR */
                        if(qer_context) {
                            //pfcp_sess_est_req->create_pdr[qer_idx].qer_id[0].qer_id_value =
                            //	pfcp_sess_est_req->create_qer[qer_idx].qer_id.qer_id_value;
                            pfcp_sess_est_req->create_qer[qer_idx].maximum_bitrate.ul_mbr  =
                                qer_context->max_bitrate.ul_mbr;
                            pfcp_sess_est_req->create_qer[qer_idx].maximum_bitrate.dl_mbr  =
                                qer_context->max_bitrate.dl_mbr;
                            pfcp_sess_est_req->create_qer[qer_idx].guaranteed_bitrate.ul_gbr  =
                                qer_context->guaranteed_bitrate.ul_gbr;
                            pfcp_sess_est_req->create_qer[qer_idx].guaranteed_bitrate.dl_gbr  =
                                qer_context->guaranteed_bitrate.dl_gbr;
                            pfcp_sess_est_req->create_qer[qer_idx].qos_flow_ident.qfi_value = 
                                qer_context->qci;
                        }
                        pfcp_sess_est_req->create_qer_count++;
                        qer_idx++;
                    }
				}

                // URR_SUPPORT : filling urr id in PDR in the pdrpfcp_sess_est_req
                if (cp_config->cp_type == PGWC || cp_config->cp_type == SAEGWC) {
                    if(cp_config->urr_enable == 1) {
                        LOG_MSG(LOG_DEBUG,"Number of URRs on PDR (%d)  are %d ", pdr->rule_id, pdr->urr_id_count);
                        pfcp_sess_est_req->create_pdr[pdr_idx].urr_id_count = pdr->urr_id_count;
                        for(int itr1 = 0; itr1 < pdr->urr_id_count; itr1++) {
                            pfcp_sess_est_req->create_pdr[pdr_idx].urr_id[itr1].urr_id_value = pdr->urr_id[itr1].urr_id;
                            LOG_MSG(LOG_DEBUG,"\tAdding URR %d in PDR-ID %d ", pdr->urr_id[itr1].urr_id, pdr->rule_id);
                        }

				        /* URR_SUPPORT : now fill the URRs in the PFCP Session EST Message. */
		                uint8_t urr_idx = pfcp_sess_est_req->create_urr_count;
				        for(uint8_t urr_itr = 0; urr_itr < pdr->urr_id_count; urr_itr++)
				        {
				        		creating_urr(&pfcp_sess_est_req->create_urr[urr_idx]);
				        		pfcp_sess_est_req->create_urr[urr_idx].urr_id.urr_id_value  = pdr->urr_id[urr_itr].urr_id;
				        		pfcp_sess_est_req->create_urr[urr_idx].meas_mthd.volum = 1;
				        		pfcp_sess_est_req->create_urr[urr_idx].rptng_triggers.volth = 1;
				        		pfcp_sess_est_req->create_urr[urr_idx].rptng_triggers.volqu = 1;
				        		pfcp_sess_est_req->create_urr[urr_idx].vol_thresh.tovol = 1;
				        		pfcp_sess_est_req->create_urr[urr_idx].vol_thresh.total_volume = 1000000;
				        		pfcp_sess_est_req->create_urr[urr_idx].volume_quota.tovol = 1;
				        		pfcp_sess_est_req->create_urr[urr_idx].volume_quota.total_volume = 1000000000; // 1GB 
				                pfcp_sess_est_req->create_urr_count++;
				        		urr_idx++;
				        }
				        LOG_MSG(LOG_DEBUG, "pfcp_sess_est_req->create_urr_count %d ", pfcp_sess_est_req->create_urr_count);
                    }
                }

                // Creating FAR @msg level which is referenced in PDR 
				if ((cp_config->cp_type == PGWC) || (SAEGWC == cp_config->cp_type)) {
					uint16_t len = 0;

					if (SOURCE_INTERFACE_VALUE_ACCESS == pdr->pdi.src_intfc.interface_value) {
					    pfcp_sess_est_req->create_far[pdr_idx].apply_action.forw = PRESENT;
						set_destination_interface(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc));
						pfcp_set_ie_header(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.header),
								PFCP_IE_FRWDNG_PARMS, sizeof(pfcp_dst_intfc_ie_t));

						pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.header.len = sizeof(pfcp_dst_intfc_ie_t);

						len += sizeof(pfcp_dst_intfc_ie_t);
						len += UPD_PARAM_HEADER_SIZE;

						pfcp_sess_est_req->create_far[pdr_idx].header.len += len;
						pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
							pdr->far.dst_intfc.interface_value;
					} else {
                        // destination access - need to drop initial packets. Forward when eNB teids available
					    pfcp_sess_est_req->create_far[pdr_idx].apply_action.drop = PRESENT;
						len += set_forwarding_param(&(pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms));
						/* Currently take as hardcoded value */
						len += UPD_PARAM_HEADER_SIZE;
						pfcp_sess_est_req->create_far[pdr_idx].header.len += len;

						pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.ipv4_address =
							pdr->far.outer_hdr_creation.ipv4_address;
						pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.outer_hdr_creation.teid =
							pdr->far.outer_hdr_creation.teid;
						pfcp_sess_est_req->create_far[pdr_idx].frwdng_parms.dst_intfc.interface_value =
							pdr->far.dst_intfc.interface_value;
					}
				}
				pdr_idx++;
			}
		}
	}


#if 0
	creating_bar(&(pfcp_sess_est_req->create_bar));

	char sgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), sgwc_addr, INET_ADDRSTRLEN);
	unsigned long sgwc_value = inet_addr(sgwc_addr);
	set_fq_csid( &(pfcp_sess_est_req->sgw_c_fqcsid), sgwc_value);

	char mme_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config.s11_mme_ip), mme_addr, INET_ADDRSTRLEN);
	unsigned long mme_value = inet_addr(mme_addr);
	set_fq_csid( &(pfcp_sess_est_req->mme_fqcsid), mme_value);

	char pgwc_addr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pgwc_addr, INET_ADDRSTRLEN);
	unsigned long pgwc_value = inet_addr(pgwc_addr);
	set_fq_csid( &(pfcp_sess_est_req->pgw_c_fqcsid), pgwc_value);

	//TODO : IP addres for epdgg is hardcoded
	const char* epdg_addr = "0.0.0.0";
	uint32_t epdg_value = inet_addr(epdg_addr);
	set_fq_csid( &(pfcp_sess_est_req->epdg_fqcsid), epdg_value);

	//TODO : IP addres for twan is hardcoded
	const char* twan_addr = "0.0.0.0";
	uint32_t twan_value = inet_addr(twan_addr);
	set_fq_csid( &(pfcp_sess_est_req->twan_fqcsid), twan_value);

	set_up_inactivity_timer(&(pfcp_sess_est_req->user_plane_inact_timer));

	set_user_id(&(pfcp_sess_est_req->user_id));
#endif

	if (upf_ctx->up_supp_features & UP_TRACE)
		set_trace_info(&(pfcp_sess_est_req->trc_info));

}


int
fill_dedicated_bearer_info(eps_bearer_t *bearer,
		ue_context_t *context, pdn_connection_t *pdn)
{
	upf_context_t *upf_ctx = context->upf_context;

    /* BUG - how can we put default bearer teid for dedicated bearer ?*/
	bearer->s5s8_sgw_gtpu_ipv4.s_addr = context->eps_bearers[pdn->default_bearer_id - 5]->s5s8_sgw_gtpu_ipv4.s_addr;

    if(cp_config->gx_enabled) {
        if (cp_config->cp_type != SGWC){
            bearer->qer_count = NUMBER_OF_QER_PER_BEARER; 
            for(uint8_t itr=0; itr < bearer->qer_count; itr++){
                // FIXME : dedicated bearer - we need to pass on correct inerface Id to generate qer id. 
                uint32_t qer_id = generate_qer_id(SOURCE_INTERFACE_VALUE_CORE); 
                bearer->qer_id[itr].qer_id = qer_id; 
                LOG_MSG(LOG_DEBUG, "fill_dedicated_bearer_info - qer_id = %d ", bearer->qer_id[itr].qer_id);
                /* GXCONFUSION - Where are we filling qos details in the bearer ?*/
                fill_qer_entry(pdn, bearer, qer_id, false, NULL); // TODO - pass correct rule pointer
            }

			/* URR_SUPPORT : generate urr ids for dedicated bearer */
            bearer->urr_count = NUMBER_OF_URR_PER_BEARER;
            for(uint8_t itr=0; itr < bearer->urr_count; itr++) {
                // FIXME : dedicated bearer we need to pass on correct inerface Id to generate urr id. 
			    bearer->urr_id[itr].urr_id = generate_urr_id(SOURCE_INTERFACE_VALUE_CORE); 
            }
            LOG_MSG(LOG_DEBUG, "Bearer has %d URRs ", bearer->urr_count);
        }
    }

	/*SP: As per discussion Per bearer two pdrs and fars will be there*/
	/************************************************
	 *  cp_type  count      FTEID_1        FTEID_2 *
	 *************************************************
	 SGWC         2      s1u  SGWU      s5s8 SGWU
	 PGWC         2      s5s8 PGWU          NA
	 SAEGWC       2      s1u SAEGWU         NA
	 ************************************************/

    if (cp_config->cp_type == SGWC) {
        bearer->pdr_count = NUMBER_OF_PDR_PER_BEARER;
        for(uint8_t itr=0; itr < bearer->pdr_count; itr++){
            switch(itr) {
                case SOURCE_INTERFACE_VALUE_ACCESS:
                    // FIXME : dedicated bearer. Send correct URR ID, QER ID 
                    fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_ACCESS, itr, 0, 0); 
                    break;
                case SOURCE_INTERFACE_VALUE_CORE:
                    // FIXME : dedicated bearer. Send correct URR ID, QER ID 
                    fill_pdr_entry(context, pdn, bearer, SOURCE_INTERFACE_VALUE_CORE, itr, 0, 0); 
                    break;
                default:
                    break;
            }
        }
    }

    /* update address/teid in PDR-PDI */
	if (cp_config->cp_type == SGWC) {
		bearer->s5s8_sgw_gtpu_ipv4.s_addr = upf_ctx->s5s8_sgwu_ip;
		bearer->s1u_sgw_gtpu_ipv4.s_addr = upf_ctx->s1u_ip;

		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);

		set_s5s8_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_sgw_gtpu_teid, upf_ctx->s5s8_sgwu_ip, SOURCE_INTERFACE_VALUE_CORE);
	} else if (cp_config->cp_type == SAEGWC) {
		bearer->s1u_sgw_gtpu_ipv4.s_addr = upf_ctx->s1u_ip;
		set_s1u_sgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s1u_sgw_gtpu_teid, upf_ctx->s1u_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	} else if (cp_config->cp_type == PGWC) {
		bearer->s5s8_pgw_gtpu_ipv4.s_addr = upf_ctx->s5s8_pgwu_ip;
		set_s5s8_pgw_gtpu_teid(bearer, context);
		update_pdr_teid(bearer, bearer->s5s8_pgw_gtpu_teid, upf_ctx->s5s8_pgwu_ip, SOURCE_INTERFACE_VALUE_ACCESS);
	}

	return 0;
}

#ifdef FUTURE_NEED
// some of the required code is rleady moved under service request and rab release procedure 
uint8_t
process_pfcp_sess_mod_resp(uint64_t sess_id, gtpv2c_header_t *gtpv2c_tx)
{
	int ret = 0;
	uint8_t ebi_index = 0;
	eps_bearer_t *bearer  = NULL;
	ue_context_t *context = NULL;
	pdn_connection_t *pdn = NULL;
	uint32_t teid = UE_SESS_ID(sess_id);

	/* Retrive the session information based on session id. */
	context = (ue_context_t *)get_sess_entry_seid(sess_id);
	if (context == NULL){
		LOG_MSG(LOG_ERROR, "NO Session Entry Found for sess ID:%lu", sess_id);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	/* Retrieve the UE context */
	context = (ue_context_t *)get_ue_context(teid);
	if (context == NULL) {
		LOG_MSG(LOG_ERROR, "Failed to update UE State for teid: %u", teid);
        assert(0);
	}

	ebi_index = UE_BEAR_ID(sess_id) - 5;
	bearer = context->eps_bearers[ebi_index];
	/* Update the UE state */
	pdn = GET_PDN(context, ebi_index);
	pdn->state = PFCP_SESS_MOD_RESP_RCVD_STATE;

	if (!bearer) {
		LOG_MSG(LOG_ERROR,
				"Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet");
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if (resp->msg_type == GTP_MODIFY_BEARER_REQ) 
    {
		/* Fill the modify bearer response */
		set_modify_bearer_response(gtpv2c_tx,
				context->sequence, context, bearer);

		/* Update the UE state */
		pdn->state = CONNECTED_STATE;

		/* Update the next hop IP address */
		if (PGWC != cp_config->cp_type) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		}
		return 0;

	} else if (resp->msg_type == GTP_CREATE_SESSION_RSP) {
		/* Fill the Create session response */
		set_create_session_response(
				gtpv2c_tx, context->sequence, context, bearer->pdn, bearer);

	} else if (resp->msg_type == GX_RAR_MSG) {
		uint8_t ebi = 0;
		get_bearer_info_install_rules(pdn, &ebi);
		bearer = context->eps_bearers[ebi];
		if (!bearer) {
			LOG_MSG(LOG_ERROR,
				"Retrive modify bearer context but EBI is non-existent- "
				"Bitmap Inconsistency - Dropping packet");
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
		}

		/* TODO: NC Need to remove hard coded pti value */
		set_create_bearer_request(gtpv2c_tx, context->sequence, context,
				bearer, pdn->default_bearer_id, 0, NULL, 0);

		resp->state = CREATE_BER_REQ_SNT_STATE;
		pdn->state = CREATE_BER_REQ_SNT_STATE;

		if (SAEGWC == cp_config->cp_type) {
			s11_mme_sockaddr.sin_addr.s_addr =
				htonl(context->s11_mme_gtpc_ipv4.s_addr);
		} else {
		    my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(bearer->pdn->s5s8_sgw_gtpc_ipv4.s_addr);
		}

		return 0;
	} else if (resp->msg_type == GTP_CREATE_BEARER_REQ) {
		bearer = context->eps_bearers[resp->eps_bearer_id - 5];
		set_create_bearer_request(
				gtpv2c_tx, context->sequence, context, bearer,
				pdn->default_bearer_id, 0, resp->eps_bearer_lvl_tft, resp->tft_header_len);

		resp->state = CREATE_BER_REQ_SNT_STATE;
		pdn->state = CREATE_BER_REQ_SNT_STATE;

		s11_mme_sockaddr.sin_addr.s_addr =
					htonl(context->s11_mme_gtpc_ipv4.s_addr);

		return 0;

	} else if (resp->msg_type == GTP_CREATE_BEARER_RSP) {
		if ((SAEGWC == cp_config->cp_type) || (PGWC == cp_config->cp_type)) {
			gen_reauth_response(context, resp->eps_bearer_id - 5);
			struct sockaddr_in saddr_in;
			saddr_in.sin_family = AF_INET;
			inet_aton("127.0.0.1", &(saddr_in.sin_addr));
            increment_gx_peer_stats(MSG_TX_DIAMETER_GX_RAA, saddr_in.sin_addr.s_addr);

			resp->msg_type = GX_RAA_MSG;
			resp->state = CONNECTED_STATE;
			pdn->state = CONNECTED_STATE;

			return 0;
		} else {
			bearer = context->eps_bearers[resp->eps_bearer_id - 5];
			set_create_bearer_response(
				gtpv2c_tx, context->sequence, context, bearer, resp->eps_bearer_id, 0);

			resp->state = CONNECTED_STATE;
			pdn->state = CONNECTED_STATE;

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				htonl(context->pdns[0]->s5s8_pgw_gtpc_ipv4.s_addr);

			return 0;
		}
	} else if(resp->msg_type == GTP_DELETE_SESSION_REQ){
		if (cp_config->cp_type == SGWC) {
			uint8_t encoded_msg[512];

			/* Indication flags not required in DSR for PGWC */
			resp->gtpc_msg.dsr.indctn_flgs.header.len = 0;
			encode_del_sess_req(
					(del_sess_req_t *)&(resp->gtpc_msg.dsr),
					encoded_msg);

			gtpv2c_header_t *header;
			header =(gtpv2c_header_t*) encoded_msg;

			ret =
				gen_sgwc_s5s8_delete_session_request((gtpv2c_header_t *)encoded_msg,
						gtpv2c_tx, htonl(bearer->pdn->s5s8_pgw_gtpc_teid),
						header->teid.has_teid.seq,
						resp->eps_bearer_id);

			my_sock.s5s8_recv_sockaddr.sin_addr.s_addr =
				resp->s5s8_pgw_gtpc_ipv4;

			/* Update the session state */
			resp->state = DS_REQ_SNT_STATE;

			/* Update the UE state */
			ret = update_ue_state(context->s11_sgw_gtpc_teid,
					DS_REQ_SNT_STATE, resp->eps_bearer_id - 5);
			if (ret < 0) {
				LOG_MSG(LOG_ERROR, "Failed to update UE State.");
			}

			LOG_MSG(LOG_DEBUG, "SGWC:s5s8_recv_sockaddr.sin_addr.s_addr :%s",
					inet_ntoa(*((struct in_addr *)&my_sock.s5s8_recv_sockaddr.sin_addr.s_addr)));

			return ret;
		}
	} else {
		/* Fill the release bearer response */
		set_release_access_bearer_response(gtpv2c_tx,
				context->sequence, context->s11_mme_gtpc_teid);

		/* Update the session state */
		resp->state = IDEL_STATE;

		/* Update the UE state */
		pdn->state = IDEL_STATE;

		s11_mme_sockaddr.sin_addr.s_addr =
						htonl(context->s11_mme_gtpc_ipv4.s_addr);

		LOG_MSG(LOG_DEBUG, "s11_mme_sockaddr.sin_addr.s_addr :%s",
				inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));

		return 0;
	}

	/* Update the session state */
	resp->state = CONNECTED_STATE;

	/* Update the UE state */
	pdn->state = CONNECTED_STATE;

	s11_mme_sockaddr.sin_addr.s_addr =
					htonl(context->s11_mme_gtpc_ipv4.s_addr);

	LOG_MSG(LOG_DEBUG, "s11_mme_sockaddr.sin_addr.s_addr :%s",
				inet_ntoa(*((struct in_addr *)&s11_mme_sockaddr.sin_addr.s_addr)));

	return 0;
}
#endif

void
fill_pfcp_sess_mod_req_pgw_init_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer = NULL;

	upf_ctx = (upf_context_t *) upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
	if(upf_ctx == NULL) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->update_far_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
		bearer = NULL;
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case PGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->update_far_count){
				for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count; itr1++) {
					pfcp_sess_mod_req->update_far[itr1].apply_action.drop = PRESENT;
				}
			}
			break;

		default :
			LOG_MSG(LOG_DEBUG,"default pfcp sess mod req");
			break;
	}

	#if 0
	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}
	#endif
}


void
fill_pfcp_sess_mod_req_pgw_init_remove_pdr(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	int ret = 0;
	uint32_t seq = 0;
	eps_bearer_t *bearer;
	pdr_t *pdr_ctxt = NULL;
	upf_context_t *upf_ctx = NULL;

	upf_ctx = (upf_context_t *) upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
	if(upf_ctx == NULL) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->remove_pdr_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count ; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					removing_pdr(&(pfcp_sess_mod_req->remove_pdr[pfcp_sess_mod_req->remove_pdr_count]));
					pfcp_sess_mod_req->remove_pdr[pfcp_sess_mod_req->remove_pdr_count].pdr_id.rule_id = pdr_ctxt->rule_id;
					pfcp_sess_mod_req->remove_pdr_count++;
				}
			}
		}

		bearer = NULL;
	}
}

void
fill_pfcp_sess_mod_req_pgw_del_cmd_update_far(pfcp_sess_mod_req_t *pfcp_sess_mod_req,
		pdn_connection_t *pdn, eps_bearer_t *bearers[], uint8_t bearer_cntr)
{
	uint32_t seq = 0;
	upf_context_t *upf_ctx = NULL;
	pdr_t *pdr_ctxt = NULL;
	int ret = 0;
	eps_bearer_t *bearer = NULL;

	upf_ctx = (upf_context_t *) upf_context_entry_lookup(pdn->upf_ipv4.s_addr);
    if(upf_ctx == NULL) {
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return;
	}

	memset(pfcp_sess_mod_req, 0, sizeof(pfcp_sess_mod_req_t));

	seq = get_pfcp_sequence_number(PFCP_SESSION_MODIFICATION_REQUEST, seq);

	set_pfcp_seid_header((pfcp_header_t *) &(pfcp_sess_mod_req->header), PFCP_SESSION_MODIFICATION_REQUEST,
			HAS_SEID, seq);

	pfcp_sess_mod_req->header.seid_seqno.has_seid.seid = pdn->dp_seid;

	//TODO modify this hard code to generic
	char pAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cp_config->pfcp_ip), pAddr, INET_ADDRSTRLEN);
	unsigned long node_value = inet_addr(pAddr);

	set_fseid(&(pfcp_sess_mod_req->cp_fseid), pdn->seid, node_value);

	pfcp_sess_mod_req->update_far_count = 0;
	for (uint8_t index = 0; index < bearer_cntr; index++){
		bearer = bearers[index];
		if(bearer != NULL) {
			for(uint8_t itr = 0; itr < bearer->pdr_count; itr++) {
				pdr_ctxt = bearer->pdrs[itr];
				if(pdr_ctxt){
					updating_far(&(pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count]));
					pfcp_sess_mod_req->update_far[pfcp_sess_mod_req->update_far_count].far_id.far_id_value = pdr_ctxt->far.far_id_value;
					pfcp_sess_mod_req->update_far_count++;
				}
			}
		}
		bearer = NULL;
	}

	switch (cp_config->cp_type)
	{
		case SGWC :
		case PGWC :
		case SAEGWC :
			if(pfcp_sess_mod_req->update_far_count){
				for(uint8_t itr1 = 0; itr1 < pfcp_sess_mod_req->update_far_count; itr1++) {
					pfcp_sess_mod_req->update_far[itr1].apply_action.drop = PRESENT;
				}
			}
			break;

		default :
			LOG_MSG(LOG_DEBUG,"default pfcp sess mod req");
			break;
	}

	set_pfcpsmreqflags(&(pfcp_sess_mod_req->pfcpsmreq_flags));
	pfcp_sess_mod_req->pfcpsmreq_flags.drobu = PRESENT;

	/*SP: This IE is included if one of DROBU and QAURR flag is set,
	  excluding this IE since we are not setting  any of this flag  */
	if(!pfcp_sess_mod_req->pfcpsmreq_flags.qaurr &&
			!pfcp_sess_mod_req->pfcpsmreq_flags.drobu){
		pfcp_sess_mod_req->pfcpsmreq_flags.header.len = 0;
	}

}

#ifdef PDR_DEBUG
void print_pdr(pdn_connection_t *pdn)
{
        eps_bearer_t *bearer;
        for(uint8_t i = 0; i < MAX_BEARERS; i++)
        {
                bearer = pdn->eps_bearers[i];
                if(bearer == NULL)
                        continue; 
                LOG_MSG(LOG_DEBUG, "Bearer pdr count %d, bearer qer count = %d ", bearer->pdr_count, bearer->qer_count);
                for(uint8_t idx = 0; idx < bearer->pdr_count; idx++)
                {
                        LOG_MSG(LOG_DEBUG, "\t\tIndex %d PDR_PTR = %p, PDR %d QER Count = %d ", idx, bearer->pdrs[idx], bearer->pdrs[idx]->rule_id, bearer->pdrs[idx]->qer_id_count);
	                    pdr_t *pdr_ctxt = bearer->pdrs[idx];
                        for (uint8_t i=0; i<pdr_ctxt->qer_id_count; i++) 
                        {
                            LOG_MSG(LOG_DEBUG, "\t\t\tIndex = %d PDR %d QER ID %d ", i, pdr_ctxt->rule_id, pdr_ctxt->qer_id[i].qer_id);
                        }
                }
        }
}
#endif
