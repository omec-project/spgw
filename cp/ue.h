// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UE_H
#define UE_H
/**
 * @file
 *
 * Contains all data structures required by 3GPP TS 23.401 Tables 5.7.3-1 and
 * 5.7.4-1 (that are nessecary for current implementaiton) to describe the
 * Connections, state, bearers, etc as well as functions to manage and/or
 * obtain value for their fields.
 *
 */

#include <stdint.h>
#include <arpa/inet.h>
#include "cp_common.h"
#include "gtpv2_ie.h"
#include "cp_interface.h"
#include "pfcp_cp_struct.h"
#include "cp_timer.h"
#include "cp_peer_struct.h"
#include "upf_struct.h"
#include "spgw_config_struct.h"
#include "proc_struct.h"
#include <sys/queue.h>
#include "sm_struct.h"
#include "pdn.h"
#include "bearer.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SDF_FILTER_TABLE_SIZE        (1024)
#define ADC_TABLE_SIZE               (1024)
#define PCC_TABLE_SIZE               (1025)
#define METER_PROFILE_SDF_TABLE_SIZE (2048)




#define QER_INDEX_FOR_ACCESS_INTERFACE 0
#define QER_INDEX_FOR_CORE_INTERFACE 1

#define URR_INDEX_FOR_ACCESS_INTERFACE 0
#define URR_INDEX_FOR_CORE_INTERFACE 1


#define CSR_SEQUENCE(x) (\
	(x->header.gtpc.teid_flag == 1)? x->header.teid.has_teid.seq : x->header.teid.no_teid.seq \
	)

#define DEFAULT_RULE_COUNT                  1
#define QCI_VALUE                           9
#define GX_PRIORITY_LEVEL                   1
#define PREEMPTION_CAPABILITY_DISABLED      1
#define PREEMPTION_VALNERABILITY_ENABLED    0
#define GX_ENABLE                           2
#define PRECEDENCE                          2
#define SERVICE_INDENTIFIRE                 11
#define RATING_GROUP                        1
#define REQUESTED_BANDWIDTH_UL              16500
#define REQUESTED_BANDWIDTH_DL              16500
#define GURATEED_BITRATE_UL                 0
#define GURATEED_BITRATE_DL                 0
#define RULE_NAME                           "default rule"
#define RULE_LENGTH                         strlen(RULE_NAME)
#define PROTO_ID                            0
#define LOCAL_IP_MASK                       0
#define LOCAL_IP_ADDR                       0
#define PORT_LOW                            0
#define PORT_HIGH                           65535
#define REMOTE_IP_MASK                      0
#define REMOTE_IP_ADDR                      0
#define GX_FLOW_COUNT                       1

struct eps_bearer;
struct pdn_connection;

/**
 * @brief  : Maintains CGI (Cell Global Identifier) data from user location information
 */
typedef struct cgi_t {
	uint8_t cgi_mcc_digit_2;
	uint8_t cgi_mcc_digit_1;
	uint8_t cgi_mnc_digit_3;
	uint8_t cgi_mcc_digit_3;
	uint8_t cgi_mnc_digit_2;
	uint8_t cgi_mnc_digit_1;
	uint16_t cgi_lac;
	uint16_t cgi_ci;
} cgi_t;

/**
 * @brief  : Maintains SAI (Service Area Identifier) data from user location information
 */
typedef struct sai_t {
	uint8_t sai_mcc_digit_2;
	uint8_t sai_mcc_digit_1;
	uint8_t sai_mnc_digit_3;
	uint8_t sai_mcc_digit_3;
	uint8_t sai_mnc_digit_2;
	uint8_t sai_mnc_digit_1;
	uint16_t sai_lac;
	uint16_t sai_sac;
}sai_t;

/**
 * @brief  : Maintains RAI (Routing Area Identity) data from user location information
 */
typedef struct rai_t {
	uint8_t ria_mcc_digit_2;
	uint8_t ria_mcc_digit_1;
	uint8_t ria_mnc_digit_3;
	uint8_t ria_mcc_digit_3;
	uint8_t ria_mnc_digit_2;
	uint8_t ria_mnc_digit_1;
	uint16_t ria_lac;
	uint16_t ria_rac;
} rai_t;

/**
 * @brief  : Maintains TAI (Tracking Area Identity) data from user location information
 */
typedef struct tai_t {
	uint8_t tai_mcc_digit_2;
	uint8_t tai_mcc_digit_1;
	uint8_t tai_mnc_digit_3;
	uint8_t tai_mcc_digit_3;
	uint8_t tai_mnc_digit_2;
	uint8_t tai_mnc_digit_1;
	uint16_t tai_tac;
} tai_t;

/**
 * @brief  : Maintains LAI (Location Area Identifier) data from user location information
 */
typedef struct lai_t {
	uint8_t lai_mcc_digit_2;
	uint8_t lai_mcc_digit_1;
	uint8_t lai_mnc_digit_3;
	uint8_t lai_mcc_digit_3;
	uint8_t lai_mnc_digit_2;
	uint8_t lai_mnc_digit_1;
	uint16_t lai_lac;
} lai_t;

/**
 * @brief  : Maintains ECGI (E-UTRAN Cell Global Identifier) data from user location information
 */
typedef struct ecgi_t {
	uint8_t ecgi_mcc_digit_2;
	uint8_t ecgi_mcc_digit_1;
	uint8_t ecgi_mnc_digit_3;
	uint8_t ecgi_mcc_digit_3;
	uint8_t ecgi_mnc_digit_2;
	uint8_t ecgi_mnc_digit_1;
	uint8_t ecgi_spare;
	uint32_t eci;
} ecgi_t;

/**
 * @brief  : Maintains Macro eNodeB ID data from user location information
 */
typedef struct macro_enb_id_t {
	uint8_t menbid_mcc_digit_2;
	uint8_t menbid_mcc_digit_1;
	uint8_t menbid_mnc_digit_3;
	uint8_t menbid_mcc_digit_3;
	uint8_t menbid_mnc_digit_2;
	uint8_t menbid_mnc_digit_1;
	uint8_t menbid_spare;
	uint8_t menbid_macro_enodeb_id;
	uint16_t menbid_macro_enb_id2;
} macro_enb_id_t;

/**
 * @brief  : Maintains Extended Macro eNodeB ID data from user location information
 */
typedef struct  extnded_macro_enb_id_t {
	uint8_t emenbid_mcc_digit_2;
	uint8_t emenbid_mcc_digit_1;
	uint8_t emenbid_mnc_digit_3;
	uint8_t emenbid_mcc_digit_3;
	uint8_t emenbid_mnc_digit_2;
	uint8_t emenbid_mnc_digit_1;
	uint8_t emenbid_smenb;
	uint8_t emenbid_spare;
	uint8_t emenbid_extnded_macro_enb_id;
	uint16_t emenbid_extnded_macro_enb_id2;
} extnded_macro_enb_id_t;

/**
 * @brief  : Maintains user location information data
 */
typedef struct user_loc_info_t {
	uint8_t lai;
	uint8_t tai;
	uint8_t rai;
	uint8_t sai;
	uint8_t cgi;
	uint8_t ecgi;
	uint8_t macro_enodeb_id;
	uint8_t extnded_macro_enb_id;
	cgi_t cgi2;
	sai_t sai2;
	rai_t rai2;
	tai_t tai2;
	lai_t lai2;
	ecgi_t ecgi2;
	macro_enb_id_t macro_enodeb_id2;
	extnded_macro_enb_id_t extended_macro_enodeb_id2;
} user_loc_info_t;

/**
 * @brief  : Maintains serving network mcc and mnc information
 */
typedef struct serving_nwrk_t {
	uint8_t mcc_digit_2;
	uint8_t mcc_digit_1;
	uint8_t mnc_digit_3;
	uint8_t mcc_digit_3;
	uint8_t mnc_digit_2;
	uint8_t mnc_digit_1;
} serving_nwrk_t;

/**
 * @brief  : Maintains rat type information
 */
typedef struct rat_type_t {
	uint8_t rat_type;
	uint16_t len;
}rat_type_t;


/**
 * @brief  : Maintains eps bearer id
 */
typedef struct ebi_id_t {
	uint64_t ebi_id;
}ebi_id;


/**
 * @brief  : Maintains selection mode info
 */
typedef struct selection_mode{
	uint8_t spare2:6;
	uint8_t selec_mode:2;
}selection_mode;

/**
 * @brief  : Maintains indication flag oi value
 */
typedef struct indication_flag_t {
	uint8_t oi:1;
}indication_flag_t;


enum UE_CONTEXT_STATE {
    UE_STATE_UNKNOWN,
    UE_STATE_ACTIVE_PENDING,
    UE_STATE_ACTIVE,
    UE_STATE_IDLE_PENDING, 
    UE_STATE_IDLE,
    UE_STATE_DETACH_PENDING,
    UE_STATE_PAGING_PENDING,
    UE_STATE_DETACH
};


/**
 * @brief  : Maintains ue related information
 */
struct ue_context {
	uint8_t state;
	uint64_t imsi;
    uint64_t imsi64; // this is printable...logically we should get rid of above variable 
	uint8_t imsi_len;
	uint8_t unathenticated_imsi;
	uint64_t mei;
	uint64_t msisdn;
	uint8_t msisdn_len;

	ambr_ie mn_ambr;
	/*TODO: Move below 3 lines into PDN*/
	user_loc_info_t uli;
	user_loc_info_t old_uli;
	bool old_uli_valid;

	serving_nwrk_t serving_nw;
	rat_type_t rat_type;
	indication_flag_t indication_flag;


	int16_t mapped_ue_usage_type;

	uint8_t selection_flag;
	selection_mode select_mode;

	uint32_t s11_sgw_gtpc_teid;
	struct in_addr s11_sgw_gtpc_ipv4;
	uint32_t s11_mme_gtpc_teid;
	struct in_addr s11_mme_gtpc_ipv4;

	uint16_t bearer_bitmap;
	uint16_t teid_bitmap;
	uint8_t num_pdns;

	struct pdn_connection *pdns[MAX_BEARERS];

	/*VS: TODO: Move bearer information in pdn structure and remove from UE context */
	struct eps_bearer *eps_bearers[MAX_BEARERS]; /* index by ebi - 5 */

	/* temporary bearer to be used during resource bearer cmd -
	 * create/deletee bearer req - rsp */
	struct eps_bearer *ded_bearer;
	uint64_t event_trigger;

    /* UE association with UPF context */
    upf_context_t  *upf_context;

    TAILQ_HEAD(proc_sub_head, proc_context) pending_sub_procs;
};
typedef struct ue_context ue_context_t;


/**
 * @brief  : sets base teid value given range by DP
 * @param  : val
 *           teid range assigned by DP
 * @return : Returns nothing
 */
void
set_base_teid(uint8_t val);

/**
 * @brief  : sets the s1u_sgw gtpu teid given the bearer
 * @param  : bearer
 *           bearer whose tied is to be set
 * @param  : context
 *           ue context of bearer, whose teid is to be set
 * @return : Returns nothing
 */
void
set_s1u_sgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context);


/**
 * @brief  : sets the s5s8_pgw gtpu teid given the bearer
 * @param  : bearer
 *           bearer whose tied is to be set
 * @param  : context
 *           ue context of bearer, whose teid is to be set
 * @return : Returns nothing
 */
void
set_s5s8_pgw_gtpu_teid_using_pdn(eps_bearer_t *bearer, pdn_connection_t *pdn);

void
set_s5s8_pgw_gtpu_teid(eps_bearer_t *bearer, ue_context_t *context);

/**
 * @brief  : sets the s5s8_pgw gtpc teid given the pdn_connection
 * @param  : pdn
 *           pdn_connection_t whose s5s8 tied is to be set
 * @return : Returns nothing
 */
void
set_s5s8_pgw_gtpc_teid(pdn_connection_t *pdn);

/**
 * @brief  : creates an UE Context (if needed), and pdn connection with a default bearer
 *           given the UE IMSI, and EBI
 * @param  : imsi
 *           value of information element of the imsi
 * @param  : imsi_len
 *           length of information element of the imsi
 * @param  : ebi
 *           Eps Bearer Identifier of default bearer
 * @param  : context
 *           UE context to be created
 * @return : - 0 if successful
 *           - > 0 if error occurs during packet filter parsing corresponds to
 *           3gpp specified cause error value
 *           - < 0 for all other errors
 */
int
create_ue_context(uint64_t *imsi_val, uint16_t imsi_len,
		uint8_t ebi, ue_context_t **context, uint8_t *apn, uint8_t apn_len);

int
del_rule_entries(ue_context_t *context, uint8_t ebi_index);

int
cleanup_ue_context(ue_context_t **context_t);

/**
 * Retrive ue context entry from Bearer table,using sgwc s5s8 teid.
 */
int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context_t **context);

/* This function use only in clean up while error */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context_t **context);


#ifdef __cplusplus
}
#endif
#endif /* UE_H */
