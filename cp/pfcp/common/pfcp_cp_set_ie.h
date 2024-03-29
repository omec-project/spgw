// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef PFCP_CP_SET_IE_H
#define PFCP_CP_SET_IE_H

#include <stdbool.h>
#include "pfcp_messages.h"
#include "ue.h"
#include "gtp_ies.h"
#include "gtp_messages.h"
#include "cp_timer.h"
#include "gtpv2_internal.h"
#include "upf_struct.h"
#include "sm_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
/* TODO: Move following lines to another file */
#define HAS_SEID 1
#define NO_SEID  0
#define PRESENT 1
#define NO_FORW_ACTION	0

#define PFCP_VERSION                            (1)

#define OFFSET 2208988800ULL


/* TODO: Move above lines to another file */

#define MAX_HOSTNAME_LENGTH							(256)

#define MAX_GTPV2C_LENGTH (MAX_GTPV2C_UDP_LEN-12) // sizeof(struct gtpc_t))

#define ALL_CPF_FEATURES_SUPPORTED  (CP_LOAD | CP_OVRL)

/*UP FEATURES LIST*/
#define EMPU    (1 << 0)
#define PDIU    (1 << 1)
#define UDBC    (1 << 2)
#define QUOAC   (1 << 3)
#define TRACE   (1 << 4)
#define FRRT    (1 << 5)
#define BUCP    (1 << 6)
#define DDND    (1 << 9)
#define DLBD    (1 << 10)
#define TRST    (1 << 11)
#define FTUP    (1 << 12)
#define PFDM    (1 << 13)
#define HEEU    (1 << 14)
#define TREU    (1 << 15)


#define PFCP_IE_HDR_SIZE sizeof(pfcp_ie_header_t)
#define BITRATE_SIZE 10

#define BUFFERED_ENTRIES_DEFAULT (1024)
#define SWGC_S5S8_HANDOVER_ENTRIES_DEFAULT     (50)

struct msgbuf1;

#pragma pack(1)

/**
 * @brief  : Maintains the context of pfcp interface
 */
typedef struct pfcp_context_t{
	uint16_t up_supported_features;
	uint8_t  cp_supported_features;
	uint32_t s1u_ip[20];
	uint32_t s5s8_sgwu_ip;
	uint32_t s5s8_pgwu_ip;
	struct in_addr ava_ip;
	bool flag_ava_ip;

} pfcp_context_t;


#pragma pack()

/**
 * @brief  : Generates sequence number
 * @param  : No parameters
 * @return : Returns generated sequence number
 */
uint32_t
generate_seq_no(void);

/**
 * @brief  : Generates sequence number for pfcp requests
 * @param  : type , pfcp request type
 * @param  : seq , default seq number
 * @return : Returns generated sequence number
 */
uint32_t
get_pfcp_sequence_number(uint8_t type, uint32_t seq);

/**
 * @brief  : Set values in pfcp header
 * @param  : pfcp, pointer to pfcp header structure
 * @param  : type, pfcp message type
 * @param  : flag, pfcp flag
 * @return : Returns nothing
 */
void
set_pfcp_header(pfcp_header_t *pfcp, uint8_t type, bool flag );

/**
 * @brief  : Set values in pfcp header and seid value
 * @param  : pfcp, pointer to pfcp header structure
 * @param  : type, pfcp message type
 * @param  : flag, pfcp flag
 * @param  : seq, pfcp message sequence number
 * @return : Returns nothing
 */
void
set_pfcp_seid_header(pfcp_header_t *pfcp, uint8_t type, bool flag, uint32_t seq );

/**
 * @brief  : Set values in ie header
 * @param  : header, pointer to pfcp ie header structure
 * @param  : type, pfcp message type
 * @param  : length, total length
 * @return : Returns nothing
 */
void
pfcp_set_ie_header(pfcp_ie_header_t *header, uint8_t type, uint16_t length);


/**
 * @brief  : Process delete session request on sgwc
 * @param  : ds_req, holds information in session deletion request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_sess_del_request_delete_bearer_rsp(del_bearer_rsp_t *db_rsp);

void 
process_pfcp_sess_del_request_delete_bearer_rsp_timeout(void *data);


/**
 * @brief  : Set values in pdn type ie
 * @param  : pdn , pdn type ie structure pointer to be filled
 * @param  : pdn_mme, use values from this structure to fill
 * @return : Returns nothing
 */
void
set_pdn_type(pfcp_pdn_type_ie_t *pdn, pdn_type_ie *pdn_mme);

/**
 * @brief  : Set values in node id ie
 * @param  : node_id, ie structure to be filled
 * @param  : nodeid_value
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_node_id(pfcp_node_id_ie_t *node_id, uint32_t nodeid_value);

/**
 * @brief  : Create and set values in create bar ie
 * @param  : create_bar, ie structure to be filled
 * @return : Returns nothing
 */
void
creating_bar(pfcp_create_bar_ie_t *create_bar);

/**
 * @brief  : Set values in trace info ie
 * @param  : trace_info, ie structure to be filled
 * @return : Returns nothing
 */
void
set_trace_info(pfcp_trc_info_ie_t *trace_info);

/**
 * @brief  : Set values in bar id ie
 * @param  : bar_id, ie structure to be filled
 * @return : Returns nothing
 */
void
set_bar_id(pfcp_bar_id_ie_t *bar_id);

/**
 * @brief  : Set values in downlink data notification delay ie
 * @param  : dl_data_notification_delay, ie structure to be filled
 * @return : Returns nothing
 */
void
set_dl_data_notification_delay(pfcp_dnlnk_data_notif_delay_ie_t
		*dl_data_notification_delay);

/**
 * @brief  : Set values in buffer packets count ie
 * @param  : sgstd_buff_pkts_cnts, ie structure to be filled
 * @return : Returns nothing
 */
void
set_sgstd_buff_pkts_cnt( pfcp_suggstd_buf_pckts_cnt_ie_t
		*sgstd_buff_pkts_cnt);

/**
 * @brief  : Set values in user plave inactivity timer ie
 * @param  : up_inact_timer, ie structure to be filled
 * @return : Returns nothing
 */
void
set_up_inactivity_timer(pfcp_user_plane_inact_timer_ie_t *up_inact_timer);

/**
 * @brief  : Set values in user id ie
 * @param  : user_id, ie structure to be filled
 * @return : Returns nothing
 */
void
set_user_id(pfcp_user_id_ie_t *user_id);

/**
 * @brief  : Set values in fseid ie
 * @param  : fseid, ie structure to be filled
 * @param  : seid, seid value
 * @param  : nodeid_value
 * @return : Returns nothing
 */
void
set_fseid(pfcp_fseid_ie_t *fseid, uint64_t seid, uint32_t nodeid_value);

/**
 * @brief  : Set values in recovery time stamp ie
 * @param  : rec_time_stamp, ie structure to be filled
 * @return : Returns nothing
 */
void
set_recovery_time_stamp(pfcp_rcvry_time_stmp_ie_t *rec_time_stamp);

/**
 * @brief  : Set values in upf features ie
 * @param  : upf_feat, ie structure to be filled
 * @return : Returns nothing
 */
void
set_upf_features(pfcp_up_func_feat_ie_t *upf_feat);

/**
 * @brief  : Set values in control plane function feature ie
 * @param  : cpf_feat, ie structure to be filled
 * @return : Returns nothing
 */
void
set_cpf_features(pfcp_cp_func_feat_ie_t *cpf_feat);

/**
 * @brief  : Set values in caues ie
 * @param  : cause, ie structure to be filled
 * @param  : cause_val, cause value to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_cause(pfcp_cause_ie_t *cause, uint8_t cause_val);

/**
 * @brief  : Set values in remove bar ie
 * @param  : remove_bar, ie structure to be filled
 * @return : Returns nothing
 */
void
removing_bar( pfcp_remove_bar_ie_t *remove_bar);

/**
 * @brief  : Set values in
 * @param  : ie structure to be filled
 * @return : Returns nothing
 */
void
removing_pdr( pfcp_remove_pdr_ie_t *remove_pdr);

void
set_traffic_endpoint(pfcp_traffic_endpt_id_ie_t *traffic_endpoint_id);

/**
 * @brief  : Set values in remove traffic endpoint ie
 * @param  : remove_traffic_endpoint, ie structure to be filled
 * @return : Returns nothing
 */
void
removing_traffic_endpoint(pfcp_rmv_traffic_endpt_ie_t
		*remove_traffic_endpoint);

/**
 * @brief  : Set values in create traffic endpoint ie
 * @param  : create_traffic_endpoint, ie structure to be filled
 * @return : Returns nothing
 */
void
creating_traffic_endpoint(pfcp_create_traffic_endpt_ie_t  *
		create_traffic_endpoint);

/**
 * @brief  : Set values in fteid ie
 * @param  : local_fteid, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_fteid(pfcp_fteid_ie_t *local_fteid);

/**
 * @brief  : Set values in network instance ie
 * @param  : network_instance, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_network_instance(pfcp_ntwk_inst_ie_t *network_instance);

/**
 * @brief  : Set values in ue ip address ie
 * @param  : ue_ip, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_ue_ip(pfcp_ue_ip_address_ie_t *ue_ip, uint32_t ue_ip_flags);

/**
 * @brief  : Set values in ethernet pdu session info ie
 * @param  : eth_pdu_sess_info, ie structure to be filled
 * @return : Returns nothing
 */
void
set_ethernet_pdu_sess_info( pfcp_eth_pdu_sess_info_ie_t
		*eth_pdu_sess_info);

/**
 * @brief  : Set values in framed routing ie
 * @param  : framedrouting, ie structure to be filled
 * @return : Returns nothing
 */
void
set_framed_routing(pfcp_framed_routing_ie_t *framedrouting);


/**
 * @brief  : Set values in qer id ie
 * @param  : qer_id, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_qer_id(pfcp_qer_id_ie_t *qer_id);

/**
 * @brief  : Set values in qer correlation id ie
 * @param  : qer_correl_id, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_qer_correl_id(pfcp_qer_corr_id_ie_t *qer_correl_id);

/**
 * @brief  : Set values in gate status ie
 * @param  : gate_status, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_gate_status( pfcp_gate_status_ie_t *gate_status);

/**
 * @brief  : Set values in mbr ie
 * @param  : mbr, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_mbr(pfcp_mbr_ie_t *mbr);

/**
 * @brief  : Set values in gbr ie
 * @param  : gbr, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_gbr(pfcp_gbr_ie_t *gbr);

/**
 * @brief  : Set values in packet rate ie
 * @param  : pkt_rate, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_packet_rate(pfcp_packet_rate_ie_t *pkt_rate);

/**
 * @brief  : Set values in downlink flow level marking ie
 * @param  : dl_flow_level_marking, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_dl_flow_level_mark(pfcp_dl_flow_lvl_marking_ie_t *dl_flow_level_marking);

/**
 * @brief  : Set values in qfi ie
 * @param  : qfi, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_qfi(pfcp_qfi_ie_t *qfi);

/**
 * @brief  : Set values in rqi ie
 * @param  : rqi, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_rqi(pfcp_rqi_ie_t *rqi);

/**
 * @brief  : Set values in create qer ie
 * @param  : qer, ie structure to be filled
 * @return : Returns nothing
 */
void
creating_qer(pfcp_create_qer_ie_t *qer);

/**
 * @brief  : Set values in update qer ie
 * @param  : up_qer, ie structure to be filled
 * @return : Returns nothing
 */
void
updating_qer(pfcp_update_qer_ie_t *up_qer);

/**
 * @brief  : Set values in update bar ie
 * @param  : up_bar, ie structure to be filled
 * @return : Returns nothing
 */
void
updating_bar( pfcp_upd_bar_sess_mod_req_ie_t *up_bar);

/**
 * @brief  : Set values in update far ie
 * @param  : up_far, ie structure to be filled
 * @return : Returns nothing
 */
void
updating_far(pfcp_update_far_ie_t *up_far);

/**
 * @brief  : Set values in update traffic endpoint ie
 * @param  : up_traffic_endpoint, ie structure to be filled
 * @return : Returns nothing
 */
void
updating_traffic_endpoint(pfcp_upd_traffic_endpt_ie_t *up_traffic_endpoint);

/**
 * @brief  : Set values in create qer ie
 * @param  : qer, ie structure to be filled
 * @return : Returns nothing
 */
void
creating_urr(pfcp_create_urr_ie_t *qer);

/**
 * @brief  : Set values in pfcpsmreq flags ie
 * @param  : pfcp_sm_req_flags, ie structure to be filled
 * @return : Returns nothing
 */
void
set_pfcpsmreqflags(pfcp_pfcpsmreq_flags_ie_t *pfcp_sm_req_flags);

/**
 * @brief  : Set values in query urr refference ie
 * @param  : query_urr_ref, ie structure to be filled
 * @return : Returns nothing
 */
void
set_query_urr_refernce(pfcp_query_urr_ref_ie_t *query_urr_ref);


/**
 * @brief  : Set values in user plane association relation request ie
 * @param  : ass_rel_req, ie structure to be filled
 * @return : Returns nothing
 */
void
set_pfcp_ass_rel_req(pfcp_up_assn_rel_req_ie_t *ass_rel_req);

/**
 * @brief  : Set values in graceful relation period ie
 * @param  : graceful_rel_period, ie structure to be filled
 * @return : Returns nothing
 */
void
set_graceful_release_period(pfcp_graceful_rel_period_ie_t *graceful_rel_period);

/**
 * @brief  : Set values in sequence number ie
 * @param  : seq, ie structure to be filled
 * @return : Returns nothing
 */
void
set_sequence_num(pfcp_sequence_number_ie_t *seq);

/**
 * @brief  : Set values in metric ie
 * @param  : metric, ie structure to be filled
 * @return : Returns nothing
 */
void
set_metric(pfcp_metric_ie_t *metric);

/**
 * @brief  : Set values in timer ie
 * @param  : pov, ie structure to be filled
 * @return : Returns nothing
 */
void
set_period_of_validity(pfcp_timer_ie_t *pov);

/**
 * @brief  : Set values in oci flags ie
 * @param  : oci, ie structure to be filled
 * @return : Returns nothing
 */
void
set_oci_flag( pfcp_oci_flags_ie_t *oci);

/**
 * @brief  : Set values in offending ie
 * @param  : offending_ie, ie structure to be filled
 * @param  : offend,  offending ie type
 * @return : Returns nothing
 */
void
set_offending_ie( pfcp_offending_ie_ie_t *offending_ie, int offend);

/**
 * @brief  : Set values in load control info ie
 * @param  : lci, ie structure to be filled
 * @return : Returns nothing
 */
void
set_lci(pfcp_load_ctl_info_ie_t *lci);

/**
 * @brief  : Set values in overload control info ie
 * @param  : olci, ie structure to be filled
 * @return : Returns nothing
 */
void
set_olci(pfcp_ovrld_ctl_info_ie_t *olci);

/**
 * @brief  : Set values in failed rule id ie
 * @param  : rule, ie structure to be filled
 * @return : Returns nothing
 */
void
set_failed_rule_id(pfcp_failed_rule_id_ie_t *rule);

/**
 * @brief  : Set values in traffic endpoint id ie
 * @param  : tnp, ie structure to be filled
 * @return : Returns nothing
 */
void
set_traffic_endpoint_id(pfcp_traffic_endpt_id_ie_t *tnp);

/**
 * @brief  : Set values in pdr id ie
 * @param  : pdr, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_pdr_id_ie(pfcp_pdr_id_ie_t *pdr);

/**
 * @brief  : Set values in created pdr ie
 * @param  : pdr, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_created_pdr_ie(pfcp_created_pdr_ie_t *pdr);

/**
 * @brief  : Set values in created traffic endpoint ie
 * @param  : cte, ie structure to be filled
 * @return : Returns nothing
 */
void
set_created_traffic_endpoint(pfcp_created_traffic_endpt_ie_t *cte);

/**
 * @brief  : Set values in additional usage ie
 * @param  : adr, ie structure to be filled
 * @return : Returns nothing
 */
void
set_additional_usage(pfcp_add_usage_rpts_info_ie_t *adr);

/**
 * @brief  : Set values in node report type ie
 * @param  : nrt, ie structure to be filled
 * @return : Returns nothing
 */
void
set_node_report_type(pfcp_node_rpt_type_ie_t *nrt);

/**
 * @brief  : Set values in user plane path failure report ie
 * @param  : uppfr, ie structure to be filled
 * @return : Returns nothing
 */
void
set_user_plane_path_failure_report(pfcp_user_plane_path_fail_rpt_ie_t *uppfr);

/**
 * @brief  : Set values in remote gtpu peer ie
 * @param  : remote_gtpu_peer, ie structure to be filled
 * @return : Returns nothing
 */
void
set_remote_gtpu_peer_ip(pfcp_rmt_gtpu_peer_ie_t *remote_gtpu_peer);

/**
 * @brief  : valiadates pfcp session association setup request and set cause and offending ie accordingly
 * @param  : pfcp_ass_setup_req, hold information from pfcp session association setup request
 * @param  : cause_id , param to set cause id
 * @param  : offend_id, param to set offending ie id
 * @return : Returns nothing
 */
void
cause_check_association(pfcp_assn_setup_req_t *pfcp_ass_setup_req,
						uint8_t *cause_id, int *offend_id);

/**
 * @brief  : valiadates pfcp session establishment request and set cause and offending ie accordingly
 * @param  : pfcp_session_request, hold information from pfcp session establishment request
 * @param  : cause_id , param to set cause id
 * @param  : offend_id, param to set offending ie id
 * @return : Returns nothing
 */
void
cause_check_sess_estab(pfcp_sess_estab_req_t
			*pfcp_session_request, uint8_t *cause_id, int *offend_id);

/**
 * @brief  : valiadates pfcp session modification request and set cause and offending ie accordingly
 * @param  : pfcp_session_mod_req, hold information from pfcp session modification request
 * @param  : cause_id , param to set cause id
 * @param  : offend_id, param to set offending ie id
 * @return : Returns nothing
 */
void
cause_check_sess_modification(pfcp_sess_mod_req_t
		*pfcp_session_mod_req, uint8_t *cause_id, int *offend_id);

/**
 * @brief  : valiadates pfcp session deletion request and set cause and offending ie accordingly
 * @param  : pfcp_session_delete_req, hold information from pfcp session request
 * @param  : cause_id , param to set cause id
 * @param  : offend_id, param to set offending ie id
 * @return : Returns nothing
 */
void
cause_check_delete_session(pfcp_sess_del_req_t
		*pfcp_session_delete_req, uint8_t *cause_id, int *offend_id);

/**
 * @brief  : Set values in create pdr ie
 * @param  : create_pdr, ie structure to be filled
 * @param  : source_iface_value, interface type
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
creating_pdr(pfcp_create_pdr_ie_t *create_pdr, int source_iface_value, uint32_t flags);

/**
 * @brief  : Set values in create far ie
 * @param  : create_far, ie structure to be filled
 * @return : Returns nothing
 */
void
creating_far(pfcp_create_far_ie_t *create_far);

int
updating_pdr(pfcp_update_pdr_ie_t *update_pdr, int source_iface_value);

void
updating_far(pfcp_update_far_ie_t *update_far);

/**
 * @brief  : Set values in forwarding params ie
 * @param  : frwdng_parms, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint16_t
set_forwarding_param(pfcp_frwdng_parms_ie_t *frwdng_parms);

/**
 * @brief  : Set values in upd forwarding params ie
 * @param  : upd_frwdng_parms, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint16_t
set_upd_forwarding_param(pfcp_upd_frwdng_parms_ie_t *upd_frwdng_parms);

/**
 * @brief  : Set values in apply action ie
 * @param  : apply_action, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint16_t
set_apply_action(pfcp_apply_action_ie_t *apply_action);

/**
 * @brief  : Set values in outer header creation ie
 * @param  : outer_hdr_creation, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint16_t
set_outer_header_creation(pfcp_outer_hdr_creation_ie_t *outer_hdr_creation);

/**
 * @brief  : Set values in destination interface ie
 * @param  : dst_intfc, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint16_t
set_destination_interface(pfcp_dst_intfc_ie_t *dst_intfc);

/**
 * @brief  : Set values in pdr id ie
 * @param  : pdr_id, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_pdr_id(pfcp_pdr_id_ie_t *pdr_id);

/**
 * @brief  : Set values in far id ie
 * @param  : far_id, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_far_id(pfcp_far_id_ie_t *far_id);

/**
 * @brief  : Set values in far id ie for modify bearer request case
 * @param  : far_id, ie structure to be filled
 * @return : Returns nothing
 */
int
set_far_id_mbr(pfcp_far_id_ie_t *far_id);

/**
 * @brief  : Set values in outer header removal ie
 * @param  : out_hdr_rem, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_outer_hdr_removal(pfcp_outer_hdr_removal_ie_t *out_hdr_rem);

/**
 * @brief  : Set values in precedence ie
 * @param  : prec, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_precedence(pfcp_precedence_ie_t *prec);

/**
 * @brief  : Set values in pdi ie
 * @param  : pdi, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_pdi(pfcp_pdi_ie_t *pdi, uint32_t ue_ip_flags);

/**
 * @brief  : Set values in application id ie
 * @param  : app_id, ie structure to be filled
 * @return : Returns nothing
 */
void
set_application_id(pfcp_application_id_ie_t *app_id);

/**
 * @brief  : Set values in source interface ie
 * @param  : src_intf, ie structure to be filled
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
set_source_intf(pfcp_src_intfc_ie_t *src_intf);

/**
 * @brief  : Set values in activate predeined rules ie
 * @param  : act_predef_rule, ie structure to be filled
 * @return : Returns nothing
 */
void
set_activate_predefined_rules(pfcp_actvt_predef_rules_ie_t *act_predef_rule);

/**
 * @brief  : Fill pfcp pfd management request
 * @param  : pfcp_pfd_req, pointer to structure to be filled
 * @param  : len, Total len
 * @return : Returns nothing
 */
void
fill_pfcp_pfd_mgmt_req(pfcp_pfd_mgmt_req_t *pfcp_pfd_req, uint16_t len);

/**
 * @brief  : Process pfcp pfd management request
 * @param  : No param
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
process_pfcp_pfd_mgmt_request(void);

/**
 * @brief  : Generate and Send CCRU message
 * @param  : Modify Bearer Request
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
send_ccr_u_msg(mod_bearer_req_t *mb_req);

/**
 * @brief  : get msg type from cstm ie string
 * @param  : pfd_conts, holds pfc contents data
 * @param  : idx, index in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint64_t
get_rule_type(pfcp_pfd_contents_ie_t *pfd_conts, uint16_t *idx);

int
set_urr_id(pfcp_urr_id_ie_t *urr_id);

int
set_measurement_method(pfcp_meas_mthd_ie_t *m_method);

int
set_reporting_triggers(pfcp_rptng_triggers_ie_t *triggers);

int
set_measurement_period(pfcp_meas_period_ie_t *meas_period);

int
set_volume_threshold(pfcp_vol_thresh_ie_t *vol_thresh);

int
set_volume_quota(pfcp_volume_quota_ie_t *volume_quota);

int
set_quota_holding_time(pfcp_quota_hldng_time_ie_t *quota_hldng_time);

int
set_downlink_drop_traffic_threshold(pfcp_drpd_dl_traffic_thresh_ie_t *drpd_dl_traffic_thresh);

int
set_far_id_quota_action(pfcp_far_id_ie_t *far_id_for_quota_act);

#ifdef __cplusplus
}
#endif
#endif /* PFCP_CP_SET_IE_H */
