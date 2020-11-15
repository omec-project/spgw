/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef PFCP_CP_STRUCT_H
#define PFCP_CP_STRUCT_H
#include "pfcp_ies.h"
#include "pfcp_struct.h"
/**
 * @brief  : Maintains far information
 */
typedef struct far_info_t {
	//uint8_t bar_id_value;						/* BAR ID */
	uint32_t far_id_value;						/* FAR ID */
	uint64_t session_id;						/* Session ID */
	ntwk_inst_t ntwk_inst;						/* Network Instance */
	dst_intfc_t dst_intfc;						/* Destination Interface */
	outer_hdr_creation_t outer_hdr_creation;	/* Outer Header Creation */
	trnspt_lvl_marking_t trnspt_lvl_marking;	/* Transport Level Marking */
	frwdng_plcy_t frwdng_plcy;					/* Forwarding policy */
	hdr_enrchmt_t hdr_enrchmt;					/* Container for header enrichment */
	apply_action actions;						/* Apply Action parameters*/
}far_t;

/**
 * @brief  : Maintains qer information
 */
typedef struct qer_info_t {
	/*VS: TODO: Remove qer id*/
	uint32_t qer_id;							/* QER ID */
	uint32_t qer_corr_id_val;					/* QER Correlation ID */
	uint64_t session_id;						/* Session ID */
	gate_status_t gate_status;					/* Gate Status UL/DL */
	mbr_t max_bitrate;							/* Maximum Bitrate */
	gbr_t guaranteed_bitrate;					/* Guaranteed Bitrate */
	packet_rate_t packet_rate;					/* Packet Rate */
	dl_flow_lvl_marking_t dl_flow_lvl_marking;	/* Downlink Flow Level Marking */
	qfi_t qos_flow_ident;						/* QOS Flow Ident */
	rqi_t reflective_qos;						/* RQI */
	paging_plcy_indctr_t paging_plcy_indctr;	/* Paging policy */
	avgng_wnd_t avgng_wnd;						/* Averaging Window */
}qer_t;

/*
 * @brief  : Maintains urr information
 */
typedef struct urr_info_t {
	uint32_t urr_id;							/* URR ID */
}urr_t;

/**
 * @brief  : Maintains pdr information
 */
typedef struct pdr_info_t {
	uint8_t urr_id_count;						/* Number of URR */
	uint8_t qer_id_count;						/* Number of QER */
	uint8_t actvt_predef_rules_count;			/* Number of predefine rules */
	uint16_t rule_id;							/* PDR ID*/
	uint32_t prcdnc_val;						/* Precedence Value*/
	uint64_t session_id;						/* Session ID */
	pdi_t pdi;									/* Packet Detection Information */
	far_t far;									/* FAR structure info */
	qer_t qer;
    urr_t urr;  
	outer_hdr_removal_t outer_hdr_removal;		/* Outer Header Removal */
	urr urr_id[MAX_LIST_SIZE];					/* Collection of URR IDs */
	qer qer_id[MAX_LIST_SIZE];					/* Collection of QER IDs */
	actvt_predef_rules rules[MAX_LIST_SIZE];	/* Collection of active predefined rules */
}pdr_t;

/**
 * @brief  : Maintains bar information
 */
typedef struct bar_info_t {
	uint8_t bar_id;				/* BAR ID */
	dnlnk_data_notif_delay_t ddn_delay;
	suggstd_buf_pckts_cnt_t suggstd_buf_pckts_cnt;
}bar_t;

#endif /* PFCP_STRUCT_H */
