// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0
#include <rte_errno.h>
#include "cp_config_defs.h"
#include "pfcp_ies.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_enum.h"
#include "cp_log.h"

#include "cp_main.h"
#include "pfcp.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "cp_config.h"
#include "spgw_cpp_wrapper.h"
#include "cp_config_apis.h"
#include "pfcp_cp_association.h"
#include "ue.h"

#define RI_MAX 8

/* size of user ip resource info ie will be 6 if teid_range is not included otherwise 7 */
#define SIZE_IF_TEIDRI_PRESENT 7
#define SIZE_IF_TEIDRI_NOT_PRESENT 6

extern uint32_t start_time;

pfcp_context_t pfcp_ctxt;
/* extern */
const uint32_t pfcp_base_seq_no = 0x00000000;
static uint32_t pfcp_seq_no_offset;

static uint32_t pfcp_sgwc_seid_offset;

const uint64_t pfcp_sgwc_base_seid = 0xC0FFEE;


void
set_pfcp_header(pfcp_header_t *pfcp, uint8_t type, bool flag )
{
	pfcp->s       = flag;
	pfcp->mp      = 0;
	pfcp->spare   = 0;
	pfcp->version = PFCP_VERSION;
	pfcp->message_type = type;
}

uint32_t
generate_seq_no(void) 
{
	uint32_t id = 0;
	id = pfcp_base_seq_no + (++pfcp_seq_no_offset);
	return id;
}

uint32_t
get_pfcp_sequence_number(uint8_t type, uint32_t seq){
	switch(type){
		case PFCP_HEARTBEAT_REQUEST :
		case PFCP_PFD_MGMT_REQUEST:
		case PFCP_ASSOCIATION_SETUP_REQUEST:
		case PFCP_ASSOCIATION_UPDATE_REQUEST:
		case PFCP_ASSOCIATION_RELEASE_REQUEST:
		case PFCP_NODE_REPORT_REQUEST:
		case PFCP_SESSION_SET_DELETION_REQUEST:
		case PFCP_SESSION_ESTABLISHMENT_REQUEST:
		case PFCP_SESSION_MODIFICATION_REQUEST:
		case PFCP_SESSION_DELETION_REQUEST:
		case PFCP_SESSION_REPORT_REQUEST:
			return generate_seq_no();
		case PFCP_HEARTBEAT_RESPONSE:
		case PFCP_ASSOCIATION_SETUP_RESPONSE:
		case PFCP_ASSOCIATION_UPDATE_RESPONSE:
		case PFCP_ASSOCIATION_RELEASE_RESPONSE:
		case PFCP_NODE_REPORT_RESPONSE:
		case PFCP_SESSION_SET_DELETION_RESPONSE:
		case PFCP_SESSION_ESTABLISHMENT_RESPONSE:
		case PFCP_SESSION_MODIFICATION_RESPONSE:
		case PFCP_SESSION_DELETION_RESPONSE:
		case PFCP_SESSION_REPORT_RESPONSE:
			return seq;
		default:
			LOG_MSG(LOG_DEBUG, "Unknown pfcp msg type. ");
			return 0;
			break;
	}
	return 0;
}

void
set_pfcp_seid_header(pfcp_header_t *pfcp, uint8_t type, bool flag,uint32_t seq)
{
	set_pfcp_header(pfcp, type, flag );

	if(flag == HAS_SEID){

		if (cp_config->cp_type == SGWC){
			pfcp->seid_seqno.has_seid.seid  =
						pfcp_sgwc_base_seid + pfcp_sgwc_seid_offset;
			pfcp_sgwc_seid_offset++;
		}
		pfcp->seid_seqno.has_seid.seq_no = seq;
		pfcp->seid_seqno.has_seid.spare  = 0;
		pfcp->seid_seqno.has_seid.message_prio = 0;

	}else if (flag == NO_SEID){
		pfcp->seid_seqno.no_seid.seq_no = seq;
		pfcp->seid_seqno.no_seid.spare  = 0;
	}
}

void
pfcp_set_ie_header(pfcp_ie_header_t *header, uint8_t type, uint16_t length)
{
	header->type = type;
	header->len = length;
}

int
set_node_id(pfcp_node_id_ie_t *node_id, uint32_t nodeid_value)
{
	int ie_length = sizeof(pfcp_node_id_ie_t) -
			sizeof(node_id->node_id_value);

	/* TODO: Need to remove hard coded value*/
	node_id->node_id_type = NODE_ID_TYPE_TYPE_IPV4ADDRESS;

	if(NODE_ID_TYPE_TYPE_IPV4ADDRESS == node_id->node_id_type) {
		memcpy(node_id->node_id_value, &nodeid_value, IPV4_SIZE);
		ie_length += IPV4_SIZE;
	} else if(NODE_ID_TYPE_TYPE_IPV6ADDRESS == node_id->node_id_type) {
		/* IPv6 Handling */
		ie_length += IPV6_SIZE;
	} else {
		/* FQDN Handling */
	}

	pfcp_set_ie_header(&(node_id->header), PFCP_IE_NODE_ID, ie_length - PFCP_IE_HDR_SIZE);
	ie_length += PFCP_IE_HDR_SIZE;

	return ie_length;
}

void
set_recovery_time_stamp(pfcp_rcvry_time_stmp_ie_t *rec_time_stamp)
{
	pfcp_set_ie_header(&(rec_time_stamp->header),
						PFCP_IE_RCVRY_TIME_STMP,sizeof(uint32_t));

	rec_time_stamp->rcvry_time_stmp_val = start_time; //uptime();

}

void
set_upf_features(pfcp_up_func_feat_ie_t *upf_feat)
{
	pfcp_set_ie_header(&(upf_feat->header), PFCP_IE_UP_FUNC_FEAT,
					sizeof(uint16_t));

	//upf_feat->supported_features = ALL_UPF_FEATURES_SUPPORTED;
	upf_feat->sup_feat = 0;
}

void
set_cpf_features(pfcp_cp_func_feat_ie_t *cpf_feat)
{
	pfcp_set_ie_header(&(cpf_feat->header), PFCP_IE_CP_FUNC_FEAT,
					sizeof(uint8_t));
	//cpf_feat->supported_features = ALL_CPF_FEATURES_SUPPORTED;
	cpf_feat->sup_feat = 0;
}


void
set_bar_id(pfcp_bar_id_ie_t *bar_id)
{
	pfcp_set_ie_header(&(bar_id->header), PFCP_IE_BAR_ID, sizeof(uint8_t));
	bar_id->bar_id_value = 1;
}

void
set_dl_data_notification_delay(pfcp_dnlnk_data_notif_delay_ie_t *dl_data_notification_delay)
{
	pfcp_set_ie_header(&(dl_data_notification_delay->header),
			PFCP_IE_DNLNK_DATA_NOTIF_DELAY, sizeof(uint8_t));

	dl_data_notification_delay->delay_val_in_integer_multiples_of_50_millisecs_or_zero= 123;
}

void
set_sgstd_buff_pkts_cnt( pfcp_suggstd_buf_pckts_cnt_ie_t *sgstd_buff_pkts_cnt)
{
	pfcp_set_ie_header(&(sgstd_buff_pkts_cnt->header),
			PFCP_IE_DL_BUF_SUGGSTD_PCKT_CNT,sizeof(uint8_t));
	sgstd_buff_pkts_cnt->pckt_cnt_val = 121;
}

int
set_pdr_id(pfcp_pdr_id_ie_t *pdr_id)
{
	int size = sizeof(pfcp_pdr_id_ie_t);

	pfcp_set_ie_header(&(pdr_id->header), PFCP_IE_PDR_ID,
			(sizeof(pfcp_pdr_id_ie_t) - sizeof(pfcp_ie_header_t)));
	pdr_id->rule_id = 0;

	return size;
}

int
set_far_id(pfcp_far_id_ie_t *far_id)
{
	int size = sizeof(pfcp_far_id_ie_t);

	pfcp_set_ie_header(&(far_id->header), PFCP_IE_FAR_ID,
			(sizeof(pfcp_far_id_ie_t) - sizeof(pfcp_ie_header_t)));
	far_id->far_id_value = 0;

	return size;

}

int
set_far_id_mbr(pfcp_far_id_ie_t *far_id)
{
    int size = sizeof(pfcp_far_id_ie_t);
	pfcp_set_ie_header(&(far_id->header), PFCP_IE_FAR_ID, sizeof(uint32_t));
	far_id->far_id_value = 0;
    return size;
}

int
set_urr_id(pfcp_urr_id_ie_t *urr_id)
{
    int size = sizeof(pfcp_urr_id_ie_t);
	pfcp_set_ie_header(&(urr_id->header), PFCP_IE_URR_ID, sizeof(uint32_t));
	urr_id->urr_id_value = 0;
    return size;
}

int
set_measurement_method(pfcp_meas_mthd_ie_t *m_method)
{
    int size = sizeof(pfcp_meas_mthd_ie_t);
	pfcp_set_ie_header(&(m_method->header), PFCP_IE_MEAS_MTHD, 1);
	m_method->volum = 0;
    return size;
}

int
set_reporting_triggers(pfcp_rptng_triggers_ie_t *triggers)
{
    int size = sizeof(pfcp_rptng_triggers_ie_t);
	pfcp_set_ie_header(&(triggers->header), PFCP_IE_RPTNG_TRIGGERS, 2);
    triggers->droth = 0; /* report when drop exceede */
    triggers->perio = 0; /* report periodically */
    triggers->volth = 0; /* report when volume threshold reaches */
    return size;
}

int
set_measurement_period(pfcp_meas_period_ie_t *meas_period)
{
    int size = sizeof(pfcp_meas_period_ie_t);
	pfcp_set_ie_header(&(meas_period->header), PFCP_IE_MEAS_PERIOD, sizeof(uint32_t));
    meas_period->meas_period = 0; /* periodic measurement period */ 
    return size;
}

int
set_volume_threshold(pfcp_vol_thresh_ie_t *vol_thresh)
{
    int size = sizeof(pfcp_vol_thresh_ie_t);
	pfcp_set_ie_header(&(vol_thresh->header), PFCP_IE_VOL_THRESH, 9);
#if 0
    vol_thresh->dlvol = 0; 
    vol_thresh->downlink_volume = 0;
    vol_thresh->ulvol = 0; 
    vol_thresh->uplink_volume = 0;
#endif
    vol_thresh->tovol = 0; 
    vol_thresh->total_volume = 0;
    return size-16;
}

int
set_volume_quota(pfcp_volume_quota_ie_t *volume_quota)
{
    int size = sizeof(pfcp_volume_quota_ie_t);
	pfcp_set_ie_header(&(volume_quota->header), PFCP_IE_VOLUME_QUOTA, 9);
#if 0
    volume_quota->dlvol = 0; 
    volume_quota->downlink_volume = 0;
    volume_quota->ulvol = 0; 
    volume_quota->uplink_volume = 0;
#endif
    volume_quota->tovol = 0; 
    volume_quota->total_volume = 0;
    return size-16;
}

int 
set_quota_holding_time(pfcp_quota_hldng_time_ie_t *quota_hldng_time)
{
    int size = sizeof(pfcp_quota_hldng_time_ie_t);
	pfcp_set_ie_header(&(quota_hldng_time->header), PFCP_IE_QUOTA_HLDNG_TIME, sizeof(uint32_t));
    quota_hldng_time->quota_hldng_time_val = 0; // how long this quota is valid ?
    return size;
}

int 
set_downlink_drop_traffic_threshold(pfcp_drpd_dl_traffic_thresh_ie_t *drpd_dl_traffic_thresh)
{
    int size = sizeof(pfcp_drpd_dl_traffic_thresh_ie_t);
	pfcp_set_ie_header(&(drpd_dl_traffic_thresh->header), PFCP_IE_DRPD_DL_TRAFFIC_THRESH, 17);
    drpd_dl_traffic_thresh->dlby = 0; 
    drpd_dl_traffic_thresh->nbr_of_bytes_of_dnlnk_data = 0;
    drpd_dl_traffic_thresh->dlpa = 0;
    drpd_dl_traffic_thresh->dnlnk_pckts = 0;
    return size;
}

int 
set_far_id_quota_action(pfcp_far_id_ie_t *far_id_for_quota_act)
{
    int size = sizeof(pfcp_far_id_ie_t);
	pfcp_set_ie_header(&(far_id_for_quota_act->header), PFCP_IE_FAR_ID, sizeof(uint32_t));
    far_id_for_quota_act->far_id_value = 0;
    return size;
}

int
set_precedence(pfcp_precedence_ie_t *prec)
{
	int size = sizeof(pfcp_precedence_ie_t);

	pfcp_set_ie_header(&(prec->header), PFCP_IE_PRECEDENCE,
			(sizeof(pfcp_precedence_ie_t) - sizeof(pfcp_ie_header_t)));
	prec->prcdnc_val = 0;

	return size;
}
int
set_outer_hdr_removal(pfcp_outer_hdr_removal_ie_t *out_hdr_rem)
{
	int size = sizeof(pfcp_outer_hdr_removal_ie_t);
	pfcp_set_ie_header(&(out_hdr_rem->header), PFCP_IE_OUTER_HDR_REMOVAL,
			 sizeof(uint8_t));

	/* TODO: Revisit this for change in yang */
	out_hdr_rem->outer_hdr_removal_desc = 0;
	/* TODO: Revisit this for change in yang */
	//out_hdr_rem->gtpu_ext_hdr_del = 0;
	//	PFCP_IE_OUTER_HDR_REMOVAL;// OUTER_HEADER_REMOVAL_DESCRIPTION_GTP_U_UDP_IPV4;
	return size;
}
void
set_application_id(pfcp_application_id_ie_t *app_id)
{
	int j =1;
	pfcp_set_ie_header(&(app_id->header), PFCP_IE_APPLICATION_ID,
			sizeof(pfcp_application_id_ie_t) - sizeof(pfcp_ie_header_t));

	/* TODO: Revisit this for change in yang */
	for(int i = 0 ;i < 8 ; i++)
		app_id->app_ident[i] = j++;

}

int
set_source_intf(pfcp_src_intfc_ie_t *src_intf)
{
	int size = sizeof(pfcp_src_intfc_ie_t);

	pfcp_set_ie_header(&(src_intf->header), PFCP_IE_SRC_INTFC,
			(sizeof(pfcp_src_intfc_ie_t) - sizeof(pfcp_ie_header_t)));
	src_intf->src_intfc_spare = 0;
	src_intf->interface_value = SOURCE_INTERFACE_VALUE_ACCESS;

	return size;
}

int
set_pdi(pfcp_pdi_ie_t *pdi, uint32_t ue_ip_flags)
{
	int size = 0;

	size += set_source_intf(&(pdi->src_intfc));
	size += set_fteid(&(pdi->local_fteid));
	size += set_network_instance(&(pdi->ntwk_inst));
	size += set_ue_ip(&(pdi->ue_ip_address), ue_ip_flags); /* flag : chv4 or v4 */
	/*size += set_traffic_endpoint(&(pdi->traffic_endpt_id));
	size += set_application_id(&(pdi->application_id));
	size += set_ethernet_pdu_sess_info(&(pdi->eth_pdu_sess_info));
	size += set_framed_routing(&(pdi->framed_routing)); */

	/* TODO: Revisit this for change in yang */
	pfcp_set_ie_header(&(pdi->header), PFCP_IE_PDI, size);

	return (size + sizeof(pfcp_ie_header_t));
}

void
set_activate_predefined_rules(pfcp_actvt_predef_rules_ie_t *act_predef_rule)
{

	pfcp_set_ie_header(&(act_predef_rule->header),PFCP_IE_ACTVT_PREDEF_RULES,
						sizeof(pfcp_actvt_predef_rules_ie_t) - sizeof(pfcp_ie_header_t));
	memcpy(&(act_predef_rule->predef_rules_nm), "PCC_RULE",8);
}

int
creating_pdr(pfcp_create_pdr_ie_t *create_pdr, int source_iface_value, uint32_t ue_ip_flags)
{
	int size = 0;

	size += set_pdr_id(&(create_pdr->pdr_id));
	size += set_precedence(&(create_pdr->precedence));
	size += set_pdi(&(create_pdr->pdi), ue_ip_flags);
	if (cp_config->cp_type != SGWC && source_iface_value == SOURCE_INTERFACE_VALUE_ACCESS)
		size += set_outer_hdr_removal(&(create_pdr->outer_hdr_removal));

	size += set_far_id(&(create_pdr->far_id));

	/* TODO: Revisit this for change in yang*/
    if (cp_config->cp_type != SGWC){
        for(int i=0; i < create_pdr->qer_id_count; i++ ) {
            size += set_qer_id(&(create_pdr->qer_id[i]));
        }
        /* URR_SUPPORT : count space used by urr_ids in the PDR */
        for(int i=0; i < create_pdr->urr_id_count; i++ ) {
            size += set_urr_id(&(create_pdr->urr_id[i]));
        }	
    }
 

	/* TODO: Revisit this for change in yang
	create_pdr->actvt_predef_rules_count = 1;
	for(int i=0; i < create_pdr->actvt_predef_rules_count; i++ ) {
		set_activate_predefined_rules(&(create_pdr->actvt_predef_rules[i]));
	} */

	pfcp_set_ie_header(&(create_pdr->header), PFCP_IE_CREATE_PDR, size);

	return size;
}

void
creating_far(pfcp_create_far_ie_t *create_far)
{
	uint16_t len = 0;

	len += set_far_id(&(create_far->far_id));
	len += set_apply_action(&(create_far->apply_action));
	/* Currently take as hardcoded value */
	len += 4; /* Header Size of set_apply action ie */

	pfcp_set_ie_header(&(create_far->header), PFCP_IE_CREATE_FAR, len);
}

int
updating_pdr(pfcp_update_pdr_ie_t *create_pdr, int source_iface_value)
{
	int size = 0;
    uint32_t ue_ip_flags = 0; // TODO : set these flags from pdn or config

	size += set_pdr_id(&(create_pdr->pdr_id));
	size += set_precedence(&(create_pdr->precedence));
	size += set_pdi(&(create_pdr->pdi), ue_ip_flags);
	if (cp_config->cp_type != SGWC && source_iface_value == SOURCE_INTERFACE_VALUE_ACCESS)
		size += set_outer_hdr_removal(&(create_pdr->outer_hdr_removal));
	size += set_far_id(&(create_pdr->far_id));
	/*TODO : need to check this durinf UT */
	if ((cp_config->gx_enabled) && (cp_config->cp_type != SGWC)){
		//for(int i=0; i < create_pdr->qer_id_count; i++ ) {
		for(int i=0; i < 1; i++ ) {
			size += set_qer_id(&(create_pdr->qer_id));
		}
	}
	/* TODO: Revisit this for change in yang
	create_pdr->urr_id_count = 1;
	for(int i=0; i < create_pdr->urr_id_count; i++ ) {
		set_urr_id(&(create_pdr->urr_id[i]));
	} */

	/* TODO: Revisit this for change in yang
	create_pdr->actvt_predef_rules_count = 1;
	for(int i=0; i < create_pdr->actvt_predef_rules_count; i++ ) {
		set_activate_predefined_rules(&(create_pdr->actvt_predef_rules[i]));
	} */

	pfcp_set_ie_header(&(create_pdr->header), PFCP_IE_CREATE_PDR, size);

	return size;
}

void
creating_bar(pfcp_create_bar_ie_t *create_bar)
{
	pfcp_set_ie_header(&(create_bar->header), PFCP_IE_CREATE_BAR,
			sizeof(pfcp_create_bar_ie_t) - sizeof(pfcp_ie_header_t));

	set_bar_id(&(create_bar->bar_id));
	set_dl_data_notification_delay(&(create_bar->dnlnk_data_notif_delay));
	set_sgstd_buff_pkts_cnt(&(create_bar->suggstd_buf_pckts_cnt));
}

uint16_t
set_apply_action(pfcp_apply_action_ie_t *apply_action)
{
	pfcp_set_ie_header(&(apply_action->header), PFCP_IE_APPLY_ACTION_ID, sizeof(uint8_t));
	apply_action->apply_act_spare = 0;
	apply_action->apply_act_spare2 = 0;
	apply_action->apply_act_spare3 = 0;
	apply_action->dupl = 0;
	apply_action->nocp = 0;
	apply_action->buff = 0;
	apply_action->forw = 0;
	apply_action->drop = 0;

	return sizeof(uint8_t);
}

uint16_t
set_forwarding_param(pfcp_frwdng_parms_ie_t *frwdng_parms)
{
	uint16_t len = 0;
	/* TODO: Remove hardcod value of size */
	len += set_destination_interface(&(frwdng_parms->dst_intfc));
	len += set_outer_header_creation(&(frwdng_parms->outer_hdr_creation));

	/* Currently take as hardcoded value */
	len += 8; /* Header Size of set_destination_interface and set_outer_header_creation ie */
	pfcp_set_ie_header(&(frwdng_parms->header), PFCP_IE_FRWDNG_PARMS, len);

	return len;
}

uint16_t
set_upd_forwarding_param(pfcp_upd_frwdng_parms_ie_t *upd_frwdng_parms)
{
	uint16_t len = 0;
	/* TODO: Remove hardcod value of size */
	len += set_destination_interface(&(upd_frwdng_parms->dst_intfc));
	len += set_outer_header_creation(&(upd_frwdng_parms->outer_hdr_creation));

	/* Currently take as hardcoded value */
	len += 8; /* Header Size of set_destination_interface and set_outer_header_creation ie */

	pfcp_set_ie_header(&(upd_frwdng_parms->header), PFCP_IE_UPD_FRWDNG_PARMS, len);
	return len;
}

uint16_t
set_outer_header_creation(pfcp_outer_hdr_creation_ie_t *outer_hdr_creation)
{
	uint16_t len = 0;

	outer_hdr_creation->outer_hdr_creation_desc = 0x0100;
	len += sizeof(outer_hdr_creation->outer_hdr_creation_desc);

	outer_hdr_creation->teid = 0;
	len += sizeof(outer_hdr_creation->teid);

	outer_hdr_creation->ipv4_address = 0;
	len += sizeof(outer_hdr_creation->ipv4_address);

	pfcp_set_ie_header(&(outer_hdr_creation->header), PFCP_IE_OUTER_HDR_CREATION, len);

	return len;
}

uint16_t
set_destination_interface(pfcp_dst_intfc_ie_t *dst_intfc)
{
	dst_intfc->dst_intfc_spare = 0;
	dst_intfc->interface_value = 5;
	pfcp_set_ie_header(&(dst_intfc->header), PFCP_IE_DEST_INTRFACE_ID, sizeof(uint8_t));
	return sizeof(uint8_t);
}

void
set_fq_csid(pfcp_fqcsid_ie_t *fq_csid,uint32_t nodeid_value)
{
	fq_csid->fqcsid_node_id_type = IPV4_GLOBAL_UNICAST;
	//TODO identify the number of CSID
	fq_csid->number_of_csids = 1;
	memcpy(&(fq_csid->node_address), &nodeid_value, IPV4_SIZE);

	for(int i = 0; i < fq_csid->number_of_csids ;i++) {
		/*PDN CONN value is 0 when it is not used */
		fq_csid->pdn_conn_set_ident[i] = 0;
		/*fq_csid->pdn_conn_set_ident[i] = htons(pdn_conn_set_id++);*/
	}

	pfcp_set_ie_header(&(fq_csid->header),
			PFCP_IE_FQCSID,2*(fq_csid->number_of_csids) + 5);

}

void
set_trace_info(pfcp_trc_info_ie_t *trace_info)
{
	//TODO from where we will fil MCC and MNC
	trace_info->mcc_digit_1 = 1;
	trace_info->mcc_digit_2 = 2;
	trace_info->mcc_digit_3 = 3;
	trace_info->mnc_digit_1 = 4;
	trace_info->mnc_digit_2 = 5;
	trace_info->mnc_digit_3 = 6;
	trace_info->trace_id  = 11231;
	trace_info->len_of_trigrng_evnts= 1;
	trace_info->trigrng_evnts  = 1;
	trace_info->sess_trc_depth = 1;
	trace_info->len_of_list_of_intfcs = 1 ;
	trace_info->list_of_intfcs = 1;
	trace_info->len_of_ip_addr_of_trc_coll_ent = 1;

	uint32_t ipv4 = htonl(32);
	memcpy(&(trace_info->ip_addr_of_trc_coll_ent), &ipv4, IPV4_SIZE);
	//trace_info->ip_address_of_trace_collection_entity[0] = 92;
	uint32_t length = trace_info->len_of_trigrng_evnts=+
		trace_info->len_of_list_of_intfcs+
					trace_info->len_of_ip_addr_of_trc_coll_ent + 14  ;

	//As Wireshark donot have spare so reducing size with 1 byte
	pfcp_set_ie_header(&(trace_info->header), PFCP_IE_TRC_INFO, length);
}

void
set_up_inactivity_timer(pfcp_user_plane_inact_timer_ie_t *up_inact_timer)
{
	//pfcp_set_ie_header(&(up_inact_timer->header),IE_USER_PLANE_INACTIVITY_TIMER ,4);
	pfcp_set_ie_header(&(up_inact_timer->header), PFCP_IE_USER_PLANE_INACT_TIMER,
			sizeof(uint32_t));
	//TODO , check the report from DP and value inact_timer accordingly 8.2.83
	up_inact_timer->user_plane_inact_timer = 10;
}
void
set_user_id(pfcp_user_id_ie_t *user_id)
{
	user_id->user_id_spare   = 0;
	user_id->naif    = 0;
	user_id->msisdnf = 0;
	user_id->imeif   = 0;
	user_id->imsif   = 1;
	user_id->length_of_imsi   = 8;
	user_id->length_of_imei   = 0;
	user_id->len_of_msisdn = 0;
	user_id->length_of_nai    = 0;

	user_id->imsi[0] = 0x77;
	user_id->imsi[1] = 0x77;
	user_id->imsi[2] = 0x77;
	user_id->imsi[3] = 0x77;
	user_id->imsi[4] = 0x77;
	user_id->imsi[5] = 0x77;
	user_id->imsi[6] = 0x77;
	user_id->imsi[7] = 0xf7;

	//pfcp_set_ie_header(&(user_id->header),IE_USER_ID , length);
	pfcp_set_ie_header(&(user_id->header), PFCP_IE_USER_ID , 10);
}

void
set_fseid(pfcp_fseid_ie_t *fseid,uint64_t seid, uint32_t nodeid_value)
{

	fseid->fseid_spare  = 0;
	fseid->fseid_spare2 = 0;
	fseid->fseid_spare3 = 0;
	fseid->fseid_spare4 = 0;
	fseid->fseid_spare5 = 0;
	fseid->fseid_spare6 = 0;
	fseid->v4     = 1;
	fseid->v6     = 0;
	fseid->seid   = seid;
	memcpy(&(fseid->ipv4_address), &nodeid_value, IPV4_SIZE);

	fseid->ipv6_address[0] = 0;

	int size = sizeof(pfcp_fseid_ie_t) - (PFCP_IE_HDR_SIZE + IPV6_ADDRESS_LEN );
	pfcp_set_ie_header(&(fseid->header), PFCP_IE_FSEID, size);

}

int
set_cause(pfcp_cause_ie_t *cause, uint8_t cause_val)
{
	int ie_length = sizeof(pfcp_cause_ie_t);

	pfcp_set_ie_header(&(cause->header), PFCP_IE_CAUSE,
			(sizeof(pfcp_cause_ie_t) - sizeof(pfcp_ie_header_t)));
	cause->cause_value = cause_val;  /*CAUSE_VALUES_REQUESTACCEPTEDSUCCESS;*/

	return ie_length;
}

void
removing_pdr(pfcp_remove_pdr_ie_t *remove_pdr)
{
	pfcp_set_ie_header(&(remove_pdr->header), PFCP_IE_REMOVE_PDR, sizeof(pfcp_pdr_id_ie_t));
	set_pdr_id(&(remove_pdr->pdr_id));
}

void
removing_bar( pfcp_remove_bar_ie_t *remove_bar)
{
	pfcp_set_ie_header(&(remove_bar->header), PFCP_IE_REMOVE_BAR, sizeof(pfcp_bar_id_ie_t));
	set_bar_id(&(remove_bar->bar_id));
}

void
set_traffic_endpoint(pfcp_traffic_endpt_id_ie_t *traffic_endpoint_id)
{
	pfcp_set_ie_header(&(traffic_endpoint_id->header), PFCP_IE_TRAFFIC_ENDPT_ID, sizeof(uint8_t));
	traffic_endpoint_id->traffic_endpt_id_val = 2;

}
void
removing_traffic_endpoint(pfcp_rmv_traffic_endpt_ie_t *remove_traffic_endpoint)
{
	pfcp_set_ie_header(&(remove_traffic_endpoint->header),
				PFCP_IE_RMV_TRAFFIC_ENDPT, sizeof(pfcp_traffic_endpt_id_ie_t));

	set_traffic_endpoint(&(remove_traffic_endpoint->traffic_endpt_id));

}
int
set_fteid( pfcp_fteid_ie_t *local_fteid)
{
	int size = sizeof(pfcp_fteid_ie_t) - (sizeof(local_fteid->ipv4_address) +
		 sizeof(local_fteid->ipv6_address) + sizeof(local_fteid->choose_id));

	/* NC : Need to remove hard coded values
	 * This values comes in input */
	local_fteid->chid = 0;
	local_fteid->ch = 0;
	local_fteid->v6 = 0;
	local_fteid->v4 = 1;

	if ((local_fteid->v4 == 1) && (local_fteid->ch == 0)) {
		/* NC : Need to remove teid and ipv4 hard coded values */
		local_fteid->teid = 1231;
		uint32_t ipv4 = htonl(3232236600);
		memcpy(&(local_fteid->ipv4_address), &ipv4, IPV4_SIZE);
		size += sizeof(local_fteid->ipv4_address);
	}

	/* TODO: IPv6 handling
	if ((local_fteid->v6 == 1) && (local_fteid->ch == 0)) {
		IPv6 handling
		size += sizeof(local_fteid->ipv6_address);
	} */

	/* TODO: This bit is only set by CP
	if (local->fteid->ch == 1) {
		UP function will assign an F-TEID and IPv4 or IPv6 address
	} */

	/* TODO: This bit shall be set by CP
	if (local_fteid->chid == 1) {
		UP function shall assign same F-TEID to PDRs requested to be
		created in a pfcp sess est and mod request with same choose id
	} */

	local_fteid->fteid_spare = 0;
	//local_fteid->choose_id = 12;

	pfcp_set_ie_header(&(local_fteid->header), PFCP_IE_FTEID,
			(size - sizeof(pfcp_ie_header_t)));

	return size;
}
int
set_network_instance(pfcp_ntwk_inst_ie_t *network_instance)
{
	int size = sizeof(pfcp_ntwk_inst_ie_t);
	pfcp_set_ie_header(&(network_instance->header), PFCP_IE_NTWK_INST,
			(sizeof(pfcp_ntwk_inst_ie_t) - sizeof(pfcp_ie_header_t)));

	return size;
}
int
set_ue_ip(pfcp_ue_ip_address_ie_t *ue_ip, uint32_t flags)
{
	int size = sizeof(pfcp_ue_ip_address_ie_t) -
		(sizeof(ue_ip->ipv4_address) + sizeof(ue_ip->ipv6_address) + sizeof(ue_ip->ipv6_pfx_dlgtn_bits));

	/* NC : Need to remove hard coded values */
	ue_ip->ue_ip_addr_spare = 0;
    if((flags & PDN_ADDR_ALLOC_CONTROL) == 0) {
	    ue_ip->chv4 = 1;
	    ue_ip->v4 = 0;
    } else {
	    ue_ip->v4 = 1;
        ue_ip->chv4 = 0;
    }
	ue_ip->ipv6d = 0;
	ue_ip->sd = 0;
	ue_ip->v6 = 0;

	if (ue_ip->v4 == 1) {
		uint32_t ipv4 = htonl(3232236600);
		memcpy(&(ue_ip->ipv4_address), &ipv4, IPV4_SIZE);
		size += sizeof(ue_ip->ipv4_address);
	}

	/* TODO: IPv6 handling */
	if (ue_ip->v6 == 1) {
		if (ue_ip->ipv6d == 1) {
			/* Use IPv6 prefix
			size += sizeof(ue_ip->ipv6_pfx_dlgtn_bits); */
		} else {
			/* Use default 64 prefix */
		}
		/* IPv6 Handling
		size += sizeof(ue_ip->ipv6_address); */
	}

	/* TODO: Need to merge below if and else in above conditions */
	if (ue_ip->sd == 0) {
		/* Source IP Address */
	} else {
		/* Destination IP Address */
	}

	pfcp_set_ie_header(&(ue_ip->header), PFCP_IE_UE_IP_ADDRESS,
			(size - sizeof(pfcp_ie_header_t)));

	return size;
}
void
set_ethernet_pdu_sess_info( pfcp_eth_pdu_sess_info_ie_t *eth_pdu_sess_info)
{
	pfcp_set_ie_header(&(eth_pdu_sess_info->header),
			PFCP_IE_ETH_PDU_SESS_INFO, sizeof(uint8_t));
	eth_pdu_sess_info->eth_pdu_sess_info_spare = 0;
	eth_pdu_sess_info->ethi = 1;
}

void
set_framed_routing(pfcp_framed_routing_ie_t *framedrouting)
{
	pfcp_set_ie_header(&(framedrouting->header), PFCP_IE_FRAMED_ROUTING, 4);

	framedrouting->framed_routing= 2;
	//framedrouting->framed_routing[1] = 2;
	//framedrouting->framed_routing[2] = 2;
	//framedrouting->framed_routing[3] = 2;
	//framedrouting->framed_routing[4] = 2;
	//framedrouting->framed_routing[5] = 2;
	//framedrouting->framed_routing[6] = 2;
	//framedrouting->framed_routing[7] = 2;

}
int
set_qer_id(pfcp_qer_id_ie_t *qer_id)
{

	int size = sizeof(pfcp_qer_id_ie_t);

	pfcp_set_ie_header(&(qer_id->header), PFCP_IE_QER_ID,
			(sizeof(pfcp_qer_id_ie_t) - sizeof(pfcp_ie_header_t)));
	qer_id->qer_id_value = 0;

	return size;
}

int
set_qer_correl_id(pfcp_qer_corr_id_ie_t *qer_correl_id)
{
	int size = sizeof(pfcp_qer_corr_id_ie_t);
	pfcp_set_ie_header(&(qer_correl_id->header), PFCP_IE_QER_CORR_ID,
			(sizeof(pfcp_qer_corr_id_ie_t) - sizeof(pfcp_ie_header_t)));

	qer_correl_id->qer_corr_id_val = 0;

	return size;
}

int
set_gate_status( pfcp_gate_status_ie_t *gate_status)
{
	int size = sizeof(pfcp_gate_status_ie_t);

	pfcp_set_ie_header(&(gate_status->header), PFCP_IE_GATE_STATUS,
			(sizeof(pfcp_gate_status_ie_t) - sizeof(pfcp_ie_header_t)));

	gate_status->gate_status_spare = 0;
	gate_status->ul_gate = UL_GATE_OPEN;
	gate_status->ul_gate = DL_GATE_OPEN;

	return size;
}

int
set_mbr(pfcp_mbr_ie_t *mbr)
{
	int size = sizeof(pfcp_mbr_ie_t);

	pfcp_set_ie_header(&(mbr->header), PFCP_IE_MBR,
			(sizeof(pfcp_mbr_ie_t) - sizeof(pfcp_ie_header_t)));

	mbr->ul_mbr =1;
	mbr->dl_mbr =1;

	return size;
}

int
set_gbr(pfcp_gbr_ie_t *gbr)
{
	int size = sizeof(pfcp_gbr_ie_t);

	pfcp_set_ie_header(&(gbr->header), PFCP_IE_GBR,
			(sizeof(pfcp_gbr_ie_t) - sizeof(pfcp_ie_header_t)));

	gbr->ul_gbr =1;
	gbr->dl_gbr =1;

	return size ;
}

int
set_packet_rate(pfcp_packet_rate_ie_t *pkt_rate)
{
	int size = sizeof(pfcp_packet_rate_ie_t);

	pkt_rate->pckt_rate_spare = 0;
	pkt_rate->dlpr = 1;
	pkt_rate->ulpr = 1;
	pkt_rate->pckt_rate_spare2 = 0;
	pkt_rate->uplnk_time_unit =  UPLINKDOWNLINK_TIME_UNIT_MINUTE;
	pkt_rate->max_uplnk_pckt_rate = 2;
	pkt_rate->pckt_rate_spare3 = 0;
	pkt_rate->dnlnk_time_unit = UPLINKDOWNLINK_TIME_UNIT_MINUTE;
	pkt_rate->max_dnlnk_pckt_rate = 2;

	pfcp_set_ie_header(&(pkt_rate->header), PFCP_IE_PACKET_RATE,
			(sizeof(pfcp_packet_rate_ie_t) - sizeof(pfcp_ie_header_t)));

	return size;
}

int
set_dl_flow_level_mark(pfcp_dl_flow_lvl_marking_ie_t *dl_flow_level_marking)
{
	int size = sizeof(pfcp_dl_flow_lvl_marking_ie_t);

	dl_flow_level_marking->dl_flow_lvl_marking_spare = 0;
	dl_flow_level_marking->sci = 1;
	dl_flow_level_marking->ttc = 1;
	dl_flow_level_marking->tostraffic_cls = 12;
	dl_flow_level_marking->svc_cls_indctr =1;
	pfcp_set_ie_header(&(dl_flow_level_marking->header),
		PFCP_IE_DL_FLOW_LVL_MARKING, sizeof(pfcp_dl_flow_lvl_marking_ie_t) - sizeof(pfcp_ie_header_t));

	return size;
}

int
set_qfi(pfcp_qfi_ie_t *qfi)
{
	int size = sizeof(pfcp_qfi_ie_t);
	qfi->qfi_spare = 0;
	qfi->qfi_value = 3;
	pfcp_set_ie_header(&(qfi->header), PFCP_IE_QFI,
			(sizeof(pfcp_qfi_ie_t) - sizeof(pfcp_ie_header_t)));

	return size;
}

int
set_rqi(pfcp_rqi_ie_t *rqi)
{
	int size = sizeof(pfcp_rqi_ie_t);

	rqi->rqi_spare = 0;
	rqi->rqi = 0;
	pfcp_set_ie_header(&(rqi->header), PFCP_IE_RQI,
			(sizeof(pfcp_rqi_ie_t) - sizeof(pfcp_ie_header_t)));

	return size;
}

void
creating_qer(pfcp_create_qer_ie_t *qer)
{
	int size = 0;
	//set qer id
	size += set_qer_id(&(qer->qer_id));

	//set qer correlation id
	//size += set_qer_correl_id(&(qer->qer_corr_id));

	//set gate status
	size += set_gate_status(&(qer->gate_status));

	//set mbr
	size += set_mbr(&(qer->maximum_bitrate));

	//set gbr
	size += set_gbr(&(qer->guaranteed_bitrate));

	//set packet rate
	//size += set_packet_rate(&(qer->packet_rate));

	//set dl flow level
	//size += set_dl_flow_level_mark(&(qer->dl_flow_lvl_marking));

	//set qfi
	//size += set_qfi(&(qer->qos_flow_ident));

	//set rqi
	//size += set_rqi(&(qer->reflective_qos));
	//size = 79;
	//sizeof(pfcp_update_qer_ie_t) - sizeof(pfcp_ie_header_t) - 12;
	pfcp_set_ie_header(&(qer->header), PFCP_IE_CREATE_QER, size);

}

void
updating_qer(pfcp_update_qer_ie_t *up_qer)
{
	int size = 0;
	//set qer id
	size += set_qer_id(&(up_qer->qer_id));

	//set qer correlation id
	size += set_qer_correl_id(&(up_qer->qer_corr_id));

	//set gate status
	size += set_gate_status(&(up_qer->gate_status));

	//set mbr
	size += set_mbr(&(up_qer->maximum_bitrate));

	//set gbr
	size += set_gbr(&(up_qer->guaranteed_bitrate));

	//set packet rate
	size += set_packet_rate(&(up_qer->packet_rate));

	//set dl flow level
	size += set_dl_flow_level_mark(&(up_qer->dl_flow_lvl_marking));

	//set qfi
	size += set_qfi(&(up_qer->qos_flow_ident));

	//set rqi
	size += set_rqi(&(up_qer->reflective_qos));
	//size = 79;
	//sizeof(pfcp_update_qer_ie_t) - sizeof(pfcp_ie_header_t) - 12;
	pfcp_set_ie_header(&(up_qer->header), PFCP_IE_UPDATE_QER, size);
}

void
creating_traffic_endpoint(pfcp_create_traffic_endpt_ie_t  *create_traffic_endpoint)
{
    uint32_t ue_ip_flags = 0; /* TODO : handle this flag from top level */
	//set traffic endpoint id
	set_traffic_endpoint(&(create_traffic_endpoint->traffic_endpt_id));

	//set local fteid
	set_fteid(&(create_traffic_endpoint->local_fteid));

	//set network isntance
	set_network_instance(&(create_traffic_endpoint->ntwk_inst));

	//set ue ip address
	set_ue_ip(&(create_traffic_endpoint->ue_ip_address), ue_ip_flags);

	//set ethernet pdu session info
	set_ethernet_pdu_sess_info(&(create_traffic_endpoint->eth_pdu_sess_info));

	//set framed routing
	set_framed_routing(&(create_traffic_endpoint->framed_routing));

	uint16_t size = sizeof(pfcp_traffic_endpt_id_ie_t) +
			sizeof(pfcp_fteid_ie_t) +sizeof(pfcp_ntwk_inst_ie_t) +

	sizeof(pfcp_ue_ip_address_ie_t) + sizeof(pfcp_eth_pdu_sess_info_ie_t) +
	sizeof(pfcp_framed_routing_ie_t);
	size = size - 18;
	pfcp_set_ie_header(&(create_traffic_endpoint->header),
							PFCP_IE_CREATE_TRAFFIC_ENDPT, size);
}
void
updating_bar( pfcp_upd_bar_sess_mod_req_ie_t *up_bar)
{
	set_bar_id(&(up_bar->bar_id));
	set_dl_data_notification_delay(&(up_bar->dnlnk_data_notif_delay));
	set_sgstd_buff_pkts_cnt(&(up_bar->suggstd_buf_pckts_cnt));

	uint8_t size =  sizeof(pfcp_bar_id_ie_t) + sizeof(pfcp_dnlnk_data_notif_delay_ie_t)+
	sizeof(pfcp_suggstd_buf_pckts_cnt_ie_t);
	pfcp_set_ie_header(&(up_bar->header), PFCP_IE_UPD_BAR_SESS_MOD_REQ, size);
}

void
updating_far(pfcp_update_far_ie_t *up_far)
{
	uint16_t len = 0;
	len += set_far_id_mbr(&(up_far->far_id));
	len += set_apply_action(&(up_far->apply_action));
	/* Currently take as hardcoded value */
	len += 4; /* Header Size of set_apply_action */

	pfcp_set_ie_header(&(up_far->header), PFCP_IE_UPDATE_FAR, len);
}

void
updating_traffic_endpoint(pfcp_upd_traffic_endpt_ie_t *up_traffic_endpoint)
{
    uint32_t ue_ip_flags = 0; /* TODO : handle this flag from top level */
	set_traffic_endpoint(&(up_traffic_endpoint->traffic_endpt_id));
	set_fteid(&(up_traffic_endpoint->local_fteid));
	set_network_instance(&(up_traffic_endpoint->ntwk_inst));
	set_ue_ip(&(up_traffic_endpoint->ue_ip_address), ue_ip_flags);
	set_framed_routing(&(up_traffic_endpoint->framed_routing));

	uint8_t size = sizeof(pfcp_traffic_endpt_id_ie_t) + sizeof(pfcp_fteid_ie_t) +
			sizeof(pfcp_ntwk_inst_ie_t) + sizeof(pfcp_ue_ip_address_ie_t) +
			sizeof(pfcp_framed_routing_ie_t);
	size = size - (2*IPV6_SIZE) - 2;
	pfcp_set_ie_header(&(up_traffic_endpoint->header),
								PFCP_IE_UPD_TRAFFIC_ENDPT, size);

}

void
set_pfcpsmreqflags(pfcp_pfcpsmreq_flags_ie_t *pfcp_sm_req_flags)
{
	pfcp_set_ie_header(&(pfcp_sm_req_flags->header),
						PFCP_IE_PFCPSMREQ_FLAGS,sizeof(uint8_t));

	pfcp_sm_req_flags->pfcpsmreq_flgs_spare = 0;
	pfcp_sm_req_flags->pfcpsmreq_flgs_spare2 = 0;
	pfcp_sm_req_flags->pfcpsmreq_flgs_spare3 = 0;
	pfcp_sm_req_flags->pfcpsmreq_flgs_spare4 = 0;
	pfcp_sm_req_flags->pfcpsmreq_flgs_spare5 = 0;
	pfcp_sm_req_flags->qaurr = 0;
	pfcp_sm_req_flags->sndem = 0;
	pfcp_sm_req_flags->drobu = 0;
}

void
set_query_urr_refernce( pfcp_query_urr_ref_ie_t *query_urr_ref)
{
	pfcp_set_ie_header(&(query_urr_ref->header),
					PFCP_IE_QUERY_URR_REF, sizeof(uint32_t));
	query_urr_ref->query_urr_ref_val = 3;

}

void
set_pfcp_ass_rel_req(pfcp_up_assn_rel_req_ie_t *ass_rel_req)
{

	pfcp_set_ie_header(&(ass_rel_req->header),
			PFCP_IE_UP_ASSN_REL_REQ, sizeof(uint8_t));
	ass_rel_req->up_assn_rel_req_spare = 0;
	ass_rel_req->sarr = 0;
}

void
set_graceful_release_period(pfcp_graceful_rel_period_ie_t *graceful_rel_period)
{
	pfcp_set_ie_header(&(graceful_rel_period->header),
						PFCP_IE_GRACEFUL_REL_PERIOD,sizeof(uint8_t));
	graceful_rel_period->timer_unit =
				GRACEFUL_RELEASE_PERIOD_INFORMATIONLEMENT_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_2_SECONDS;

	graceful_rel_period->timer_value = 1;

}

void
set_sequence_num(pfcp_sequence_number_ie_t *seq)
{
	pfcp_set_ie_header(&(seq->header), PFCP_IE_SEQUENCE_NUMBER, sizeof(uint32_t));
	seq->sequence_number = 12;
}

void
set_metric(pfcp_metric_ie_t *metric)
{
	pfcp_set_ie_header(&(metric->header), PFCP_IE_METRIC, sizeof(uint8_t));
	metric->metric = 2;
}

void
set_period_of_validity(pfcp_timer_ie_t *pov)
{
	pfcp_set_ie_header(&(pov->header), PFCP_IE_TIMER, sizeof(uint8_t));
	pov->timer_unit =
		TIMER_INFORMATIONLEMENT_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_2_SECONDS ;
	pov->timer_value = 20;
}
void
set_oci_flag( pfcp_oci_flags_ie_t *oci)
{
	pfcp_set_ie_header(&(oci->header), PFCP_IE_OCI_FLAGS, sizeof(uint8_t));
	oci->oci_flags_spare = 0;
	oci->aoci = 1;
}

void
set_offending_ie( pfcp_offending_ie_ie_t *offending_ie, int offend_val)
{
	pfcp_set_ie_header(&(offending_ie->header), PFCP_IE_OFFENDING_IE, sizeof(uint16_t));
	offending_ie->type_of_the_offending_ie = offend_val;
}

void
set_lci(pfcp_load_ctl_info_ie_t *lci)
{
	pfcp_set_ie_header(&(lci->header),PFCP_IE_LOAD_CTL_INFO,
			sizeof(pfcp_sequence_number_ie_t) + sizeof(pfcp_metric_ie_t));
	set_sequence_num(&(lci->load_ctl_seqn_nbr));
	set_metric(&(lci->load_metric));
}

void
set_olci(pfcp_ovrld_ctl_info_ie_t *olci)
{
	pfcp_set_ie_header(&(olci->header), PFCP_IE_OVRLD_CTL_INFO,
			sizeof(pfcp_sequence_number_ie_t) +
			sizeof(pfcp_metric_ie_t)+sizeof(pfcp_timer_ie_t) + sizeof(pfcp_oci_flags_ie_t));

	set_sequence_num(&(olci->ovrld_ctl_seqn_nbr));
	set_metric(&(olci->ovrld_reduction_metric));
	set_period_of_validity(&(olci->period_of_validity));
	set_oci_flag(&(olci->ovrld_ctl_info_flgs));
}

void
set_failed_rule_id(pfcp_failed_rule_id_ie_t *rule)
{
	pfcp_set_ie_header(&(rule->header), PFCP_IE_FAILED_RULE_ID, 3);
	rule->failed_rule_id_spare = 0;
	rule ->rule_id_type = RULE_ID_TYPE_PDR;
	//rule->rule_id_value = 2;
	rule->rule_id_value[1] = 2;
}

void
set_traffic_endpoint_id(pfcp_traffic_endpt_id_ie_t *tnp)
{
	pfcp_set_ie_header(&(tnp->header), PFCP_IE_TRAFFIC_ENDPT_ID, sizeof(uint8_t));
	tnp->traffic_endpt_id_val = 12;
}

int
set_pdr_id_ie(pfcp_pdr_id_ie_t *pdr)
{
	int ie_length = sizeof(pfcp_pdr_id_ie_t);

	pfcp_set_ie_header(&(pdr->header), PFCP_IE_PDR_ID,
			sizeof(pfcp_pdr_id_ie_t) - PFCP_IE_HDR_SIZE);
	pdr->rule_id = 12;

	return ie_length;
}

int
set_created_pdr_ie(pfcp_created_pdr_ie_t *pdr)
{
	int ie_length = 0;

	ie_length += set_pdr_id_ie(&(pdr->pdr_id));
	ie_length += set_fteid(&(pdr->local_fteid));

	pfcp_set_ie_header(&(pdr->header), PFCP_IE_CREATED_PDR, ie_length);

	ie_length += PFCP_IE_HDR_SIZE;
	return ie_length;
}

void set_created_traffic_endpoint(pfcp_created_traffic_endpt_ie_t *cte)
{
	//pfcp_set_ie_header(&(cte->header),IE_CREATE_TRAFFIC_ENDPOINT,sizeof(pfcp_created_traffic_endpt_ie_t)-4);
	pfcp_set_ie_header(&(cte->header), PFCP_IE_CREATE_TRAFFIC_ENDPT, 18);
	set_traffic_endpoint_id(&(cte->traffic_endpt_id));
	set_fteid(&(cte->local_fteid));

}

void set_additional_usage(pfcp_add_usage_rpts_info_ie_t *adr)
{
	//pfcp_set_ie_header(&(adr->header),IE_ADDITIONAL_USAGE_REPORTS_INFORMATION,sizeof(pfcp_add_usage_rpts_info_ie_t)-4);
	pfcp_set_ie_header(&(adr->header), PFCP_IE_ADD_USAGE_RPTS_INFO,
			sizeof(uint16_t));
	adr->auri = 0;
	adr->nbr_of_add_usage_rpts_val = 12;
}
void
set_node_report_type( pfcp_node_rpt_type_ie_t *nrt)
{
	pfcp_set_ie_header(&(nrt->header), PFCP_IE_NODE_RPT_TYPE, sizeof(uint8_t));
	nrt->node_rpt_type_spare = 0;
	nrt->upfr = 0;
}

void
set_remote_gtpu_peer_ip( pfcp_rmt_gtpu_peer_ie_t *remote_gtpu_peer)
{

	pfcp_set_ie_header(&(remote_gtpu_peer->header), PFCP_IE_RMT_GTPU_PEER,
			sizeof(pfcp_rmt_gtpu_peer_ie_t) - (IPV6_SIZE+PFCP_IE_HDR_SIZE));
	remote_gtpu_peer->v4 = 1;
	remote_gtpu_peer->v6 = 0;
	uint32_t ipv4 = htonl(3211236600);
	memcpy(&(remote_gtpu_peer->ipv4_address), &ipv4, IPV4_SIZE);
}
void
set_user_plane_path_failure_report(pfcp_user_plane_path_fail_rpt_ie_t *uppfr)
{
	pfcp_set_ie_header(&(uppfr->header), PFCP_IE_USER_PLANE_PATH_FAIL_RPT,
			sizeof(pfcp_rmt_gtpu_peer_ie_t));
	//set remote gtpu peer
	uppfr->rmt_gtpu_peer_count = 2;
	for(int i=0; i < uppfr->rmt_gtpu_peer_count; i++ )
		set_remote_gtpu_peer_ip(&(uppfr->rmt_gtpu_peer[i]));

}

void cause_check_association(pfcp_assn_setup_req_t *pfcp_ass_setup_req,
		uint8_t *cause_id, int *offend_id)
{
	*cause_id = REQUESTACCEPTED ;
	*offend_id = 0;

	if(!(pfcp_ass_setup_req->node_id.header.len)){
		*cause_id = MANDATORYIEMISSING;
		*offend_id = PFCP_IE_NODE_ID;
	} else {

		if (pfcp_ass_setup_req->node_id.node_id_type == IPTYPE_IPV4) {
				if (NODE_ID_IPV4_LEN != pfcp_ass_setup_req->node_id.header.len) {
					*cause_id = INVALIDLENGTH;
				}
		}
		if (pfcp_ass_setup_req->node_id.node_id_type == IPTYPE_IPV6) {
				if (NODE_ID_IPV6_LEN != pfcp_ass_setup_req->node_id.header.len) {
					*cause_id = INVALIDLENGTH;
				}
		}

		//*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}


	if (!(pfcp_ass_setup_req->rcvry_time_stmp.header.len)) {

		*cause_id = MANDATORYIEMISSING;
		*offend_id =PFCP_IE_RCVRY_TIME_STMP;
	} else if(pfcp_ass_setup_req->rcvry_time_stmp.header.len != RECOV_TIMESTAMP_LEN){

		*cause_id = INVALIDLENGTH;
	}

	/*if (!(pfcp_ass_setup_req->cp_func_feat.header.len)) {

		*cause_id = CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_CP_FUNC_FEAT ;
	} else if (pfcp_ass_setup_req->cp_func_feat.header.len != CP_FUNC_FEATURES_LEN) {

		*cause_id = INVALIDLENGTH;
	}*/
}


void cause_check_sess_estab(pfcp_sess_estab_req_t *pfcp_session_request,
				 uint8_t *cause_id, int *offend_id)
{
	*cause_id  = REQUESTACCEPTED;
	*offend_id = 0;

	if(!(pfcp_session_request->node_id.header.len)) {

		*offend_id = PFCP_IE_NODE_ID;
		*cause_id = MANDATORYIEMISSING;

	} else {

		 if (pfcp_session_request->node_id.node_id_type == IPTYPE_IPV4) {
				 if (NODE_ID_IPV4_LEN != pfcp_session_request->node_id.header.len) {
					*cause_id = INVALIDLENGTH;
				 }
		 }
		 if (pfcp_session_request->node_id.node_id_type == IPTYPE_IPV6) {
			 if (NODE_ID_IPV6_LEN != pfcp_session_request->node_id.header.len) {
				 *cause_id = INVALIDLENGTH;
			 }
		 }
		 //*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_request->cp_fseid.header.len)){

		*offend_id = PFCP_IE_FSEID;
		*cause_id = MANDATORYIEMISSING;


	} else if (pfcp_session_request->cp_fseid.header.len != CP_FSEID_LEN) {

		*cause_id = INVALIDLENGTH;
	}
	if(!pfcp_session_request->create_far_count){

		*offend_id = PFCP_IE_FAR_ID;
		*cause_id = MANDATORYIEMISSING;

	}else{
		for(uint8_t i =0; i < pfcp_session_request->create_far_count; i++){
			if(!pfcp_session_request->create_far[i].far_id.header.len){

				*offend_id = PFCP_IE_FAR_ID;
				*cause_id = MANDATORYIEMISSING;
				return;
			}

			if(!pfcp_session_request->create_far[i].apply_action.header.len){

				*offend_id = PFCP_IE_APPLY_ACTION;
				*cause_id = MANDATORYIEMISSING;
				return;
			}
		}
	}

	if(!pfcp_session_request->create_pdr_count){

		*offend_id = PFCP_IE_PDR_ID;
		*cause_id = MANDATORYIEMISSING;

	}else{
		for(uint8_t i =0; i < pfcp_session_request->create_pdr_count; i++){
			if(!pfcp_session_request->create_pdr[i].pdr_id.header.len){

				*offend_id = PFCP_IE_PDR_ID;
				*cause_id = MANDATORYIEMISSING;
				return;
			}

			if(!pfcp_session_request->create_pdr[i].precedence.header.len){

				*offend_id = PFCP_IE_PRECEDENCE;
				*cause_id = MANDATORYIEMISSING;
				return;
			}

			if(!pfcp_session_request->create_pdr[i].pdi.header.len){

				*offend_id = PFCP_IE_PDI;
				*cause_id = MANDATORYIEMISSING;
				return;
			}else{
				if(!pfcp_session_request->create_pdr[i].pdi.src_intfc.header.len){
					*offend_id = PFCP_IE_SRC_INTFC;
					*cause_id = MANDATORYIEMISSING;
					return;
				}
			}
		}
	}
	/*if(!(pfcp_session_request->pgwc_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = IE_PFCP_FQ_CSID;

	} else if(pfcp_session_request->pgwc_fqcsid.header.len != PGWC_FQCSID_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;

	}*/

	/*if(!(pfcp_session_request->sgw_c_fqcsid.header.len)) {

		*cause_id = CONDITIONALIEMISSING;
		*offend_id =PFCP_IE_FQCSID;

	} else if(pfcp_session_request->sgw_c_fqcsid.header.len != SGWC_FQCSID_LEN) {
		*cause_id = INVALIDLENGTH;
	}*/

	/*if(!(pfcp_session_request->mme_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = IE_PFCP_FQ_CSID;
	} else if(pfcp_session_request->mme_fqcsid.header.len != MME_FQCSID_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_request->epdg_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = IE_PFCP_FQ_CSID;
	} else if(pfcp_session_request->epdg_fqcsid.header.len != EPDG_FQCSID_LEN ) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}


	if(!(pfcp_session_request->twan_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = IE_PFCP_FQ_CSID;
	} else if (pfcp_session_request->twan_fqcsid.header.len != TWAN_FQCSID_LEN ) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if (pfcp_session_request->sgwc_fqcsid.fq_csid_node_id_type == IPTYPE_IPV4 ) {
		if(29 != (pfcp_session_request->sgwc_fqcsid.header.len)) {
			*cause_id = CAUSE_VALUES_INVALIDLENGTH;
		}
	} else if (pfcp_session_request->sgwc_fqcsid.fq_csid_node_id_type == IPTYPE_IPV6 ) {
		if(33 != (pfcp_session_request->sgwc_fqcsid.header.len)) {
			*cause_id = CAUSE_VALUES_INVALIDLENGTH;
		}
	}*/


}

void
cause_check_sess_modification(pfcp_sess_mod_req_t *pfcp_session_mod_req,
		uint8_t *cause_id, int *offend_id)
{
	*cause_id  = REQUESTACCEPTED;
	*offend_id = 0;

	if(!(pfcp_session_mod_req->cp_fseid.header.len)){
		*cause_id = CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_FSEID;
	} else if (pfcp_session_mod_req->cp_fseid.header.len != CP_FSEID_LEN)
	{
		//TODO: IPV4 consideration only
		*cause_id = INVALIDLENGTH;
	}


	if( pfcp_ctxt.up_supported_features & UP_PDIU ) {
		if(!(pfcp_session_mod_req->rmv_traffic_endpt.header.len)) {

			*cause_id = CONDITIONALIEMISSING;
			*offend_id = PFCP_IE_RMV_TRAFFIC_ENDPT;
		} else if(pfcp_session_mod_req->rmv_traffic_endpt.header.len !=
				REMOVE_TRAFFIC_ENDPOINT_LEN) {

			//*cause_id = CAUSE_VALUES_INVALIDLENGTH;
		}


		if(!(pfcp_session_mod_req->create_traffic_endpt.header.len)) {

			*cause_id = CONDITIONALIEMISSING;
			*offend_id = PFCP_IE_CREATE_TRAFFIC_ENDPT ;
		} else if (pfcp_session_mod_req->create_traffic_endpt.header.len !=
				CREATE_TRAFFIC_ENDPOINT_LEN){
			//TODO:Consdiering IP4
			//*cause_id = CAUSE_VALUES_INVALIDLENGTH;
		}
	}

	/*
	for( int i = 0; i < pfcp_session_mod_req->create_pdr_count ; i++){
		if((pfcp_session_mod_req->create_pdr[i].header.len) &&
				(pfcp_session_mod_req->create_pdr[i].far_id.header.len)){
			for( int j = 0; j < pfcp_session_mod_req->create_far_count ; j++){
				if((pfcp_session_mod_req->create_far[j].header.len) &&
						(pfcp_session_mod_req->create_far[j].bar_id.header.len)){
					if(!(pfcp_session_mod_req->create_bar.header.len)){
						*cause_id = CONDITIONALIEMISSING;
						*offend_id = IE_CREATE_BAR;
					} else if (pfcp_session_mod_req->create_bar.header.len != CREATE_BAR_LEN) {
						*cause_id = INVALIDLENGTH;
					}
				}
			}
		}
	}
	*/

	/*if(!(pfcp_session_mod_req->update_qer.header.len)) {
		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_UPDATE_QER;
	} else if(pfcp_session_mod_req->update_qer.header.len != UPDATE_QER_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}*/

	/*
	if(!(pfcp_session_mod_req->update_bar.header.len)) {
		*cause_id = CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_UPD_BAR_SESS_MOD_REQ;

	} else if(pfcp_session_mod_req->update_bar.header.len != UPDATE_BAR_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	} */

	/*TODO: There must a different FQCSID flag which is not comming from CP,that is why
	code is commented*/

	/*if(!(pfcp_session_mod_req->update_traffic_endpt.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_UPDATE_TRAFFIC_ENDPOINT;
	} else if (pfcp_session_mod_req->update_traffic_endpoint.header.len != UPDATE_TRAFFIC_ENDPOINT_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}


	if(!(pfcp_session_mod_req->pfcpsmreqflags.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_PFCPSMREQ_FLAGS;
	} else if(pfcp_session_mod_req->pfcpsmreqflags.header.len != PFCP_SEMREQ_FLAG_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}


	if(!(pfcp_session_mod_req->query_urr_reference.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_QUERY_URR_REFERENCE ;
	} else if(pfcp_session_mod_req->query_urr_reference.header.len != QUERY_URR_REFERENCE_LEN){

		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}
	if(!(pfcp_session_mod_req->pgwc_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_PFCP_FQ_CSID;
	} else if(pfcp_session_mod_req->pgwc_fqcsid.header.len != PGWC_FQCSID_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_mod_req->sgwc_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id =PFCP_IE_PFCP_FQ_CSID;
	} else if(pfcp_session_mod_req->sgwc_fqcsid.header.len != SGWC_FQCSID_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}
	if(!(pfcp_session_mod_req->mme_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_PFCP_FQ_CSID;
	} else if(pfcp_session_mod_req->mme_fqcsid.header.len != MME_FQCSID_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_mod_req->epdg_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_PFCP_FQ_CSID;
	} else if(pfcp_session_mod_req->epdg_fqcsid.header.len != EPDG_FQCSID_LEN ) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_mod_req->twan_fqcsid.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_PFCP_FQ_CSID;
	} else if (pfcp_session_mod_req->twan_fqcsid.header.len != TWAN_FQCSID_LEN ) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}

	if(!(pfcp_session_mod_req->user_plane_inact_timer.header.len)) {

		*cause_id = CAUSE_VALUES_CONDITIONALIEMISSING;
		*offend_id = PFCP_IE_USER_PLANE_INACTIVITY_TIMER ;
	} else if(pfcp_session_mod_req->user_plane_inact_timer.header.len != USER_PLANE_INACTIV_TIMER_LEN) {
		*cause_id = CAUSE_VALUES_INVALIDLENGTH;
	}*/
}

void
cause_check_delete_session(pfcp_sess_del_req_t *pfcp_session_delete_req,
		uint8_t *cause_id, int *offend_id)
{
	*cause_id  = REQUESTACCEPTED;
	*offend_id = 0;
	if(!(pfcp_session_delete_req->header.message_len)) {
		*cause_id = MANDATORYIEMISSING;
		*offend_id = PFCP_IE_FSEID;
	} else if(pfcp_session_delete_req->header.message_len !=
			DELETE_SESSION_HEADER_LEN){
		*cause_id = INVALIDLENGTH;
	}
}





void
set_pdn_type(pfcp_pdn_type_ie_t *pdn, pdn_type_ie *pdn_mme)
{
	pfcp_set_ie_header(&(pdn->header), PFCP_IE_PDN_TYPE, sizeof(uint8_t));
	pdn->pdn_type_spare = 0;
	//pdn->pdn_type = PFCP_PDN_TYPE_NON_IP;
	/* VS: Need to check the following conditions*/
	if (pdn_mme->ipv4 == PDN_IP_TYPE_IPV4)
		pdn->pdn_type = PDN_IP_TYPE_IPV4;
	else if (pdn_mme->ipv6 == PDN_IP_TYPE_IPV6)
		pdn->pdn_type = PDN_IP_TYPE_IPV6;
	else if ((pdn_mme->ipv4 == PDN_IP_TYPE_IPV4) &&
			(pdn_mme->ipv6 == PDN_IP_TYPE_IPV6)){
		pdn->pdn_type = PDN_IP_TYPE_IPV4V6;
	}
}



/*get msg type from cstm ie string */
uint64_t
get_rule_type(pfcp_pfd_contents_ie_t *pfd_conts, uint16_t *idx)
{
	char Temp_buf[3] = {0};
	for(*idx = 0; pfd_conts->cstm_pfd_cntnt[*idx] != 32; (*idx += 1))
	{
		Temp_buf[*idx] = pfd_conts->cstm_pfd_cntnt[*idx];
	}

	*idx += 1;
	Temp_buf[*idx] = '\0';
	return atoi(Temp_buf);
}

void
creating_urr(pfcp_create_urr_ie_t *urr)
{
	int size = 0;
	size += set_urr_id(&(urr->urr_id));

	size += set_measurement_method(&(urr->meas_mthd));

	size += set_reporting_triggers(&(urr->rptng_triggers));

	//size += set_measurement_period(&(urr->meas_period));

	size += set_volume_threshold(&(urr->vol_thresh));

	size += set_volume_quota(&(urr->volume_quota));

	//size += set_quota_holding_time(&(urr->quota_hldng_time));

	//size += set_downlink_drop_traffic_threshold(&(urr->drpd_dl_traffic_thresh));

	//size += set_far_id_quota_action(&(urr->far_id_for_quota_act));

	pfcp_set_ie_header(&(urr->header), PFCP_IE_CREATE_URR , size);
}

