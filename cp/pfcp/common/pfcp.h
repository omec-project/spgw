// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef PFCP_H
#define PFCP_H

//#include "gx_app/include/gx_struct.h"
#include "pfcp_struct.h"
#include "pfcp_cp_struct.h"
#include "gtpv2_ie.h"
#include "ue.h"
#include "pdn.h"
#include "bearer.h"
#include "cp_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file
 *
 * PFCP definitions and helper macros.
 *
 * GTP Message type definition and GTP header definition according to 3GPP
 * TS 29.274; as well as IE parsing helper functions/macros, and message
 * processing function declarations.
 *
 */

/**
 * @brief : Rule Name is key for Mapping of Rules and Bearer table.
 */
typedef struct rule_name_bearer_id_map_key {
	/** Rule Name */
	char rule_name[256];
}rule_name_key_t;

/**
 * @brief : Bearer identifier information
 */
typedef struct bearer_identifier_t {
	/* Bearer identifier */
	uint8_t bearer_id;
}bearer_id_t;

/**
 * @brief : PFCP context information for PDR, QER, BAR and FAR.
 */
struct pfcp_cntxt {
	/* TODO: THIS STRUCTURE STORED CSR INFORMATION UNTIL NOT GETTING CCA FROM GX*/
	/* Number of PDRs */
//	uint8_t create_pdr_count;
//	/* Number of FARs*/
//	uint8_t create_far_count;
//	/* Collection of PDRs */
//	pdr_t pdr[MAX_LIST_SIZE];
//	/* Collection of FARs */
//	far_t far[MAX_LIST_SIZE];

}__attribute__((packed));


/**
 * @brief  : Update PDR entry.
 * @param  : bearer context to be updated
 * @param  : teid to be updated
 * @param  : ip addr to be updated
 * @param  : iface, interface type ACCESS or CORE
 * @return : Returns 0 on success , -1 otherwise
 */
int
update_pdr_teid(eps_bearer_t *bearer, uint32_t teid, uint32_t ip, uint8_t iface);

/**
 * @brief  : Generate the PDR ID [RULE ID]
 * @param  : void
 * @return : Returns pdr id  on success , 0 otherwise
 */
uint16_t
generate_pdr_id(void);

/**
 * @brief  : Generate the BAR ID
 * @param  : void
 * @return : Returns bar id  on success , 0 otherwise
 */
uint8_t
generate_bar_id(void);

/**
 * @brief  : Generate the FAR ID
 * @param  : void
 * @return : Returns far id  on success , 0 otherwise
 */
uint32_t
generate_far_id(void);

/*
 * @brief  : Generate the URR ID
 * @param  : void
 * @return : Returns qer id  on success , 0 otherwise
 */
uint32_t
generate_urr_id(uint32_t iface);

/*
 * @brief  : Generate the QER ID
 * @param  : void
 * @return : Returns qer id  on success , 0 otherwise
 */
uint32_t
generate_qer_id(uint32_t iface);

bool isCommonURR(uint32_t id);
bool isCommonQER(uint32_t id);

/**
 * @brief  : Generate the Sequence
 * @param  : void
 * @return : Returns sequence number on success , 0 otherwise
 */
uint32_t
generate_rar_seq(void);


/**
 * Updates the already existing bearer
 */

int16_t
gx_update_bearer_req(pdn_connection_t *pdn);

void
get_charging_rule_remove_bearer_info(pdn_connection_t *pdn,
	uint8_t *lbi, uint8_t *ded_ebi, uint8_t *ber_cnt);

/**
 * @brief  : Generates new bearer id
 * @param  : pdn context
 * @return : Returns new bearer id
 */
int8_t
get_bearer_info_install_rules(pdn_connection_t *pdn,
	uint8_t *ebi);


/**
 * @brief  : Convert the decimal value into the string
 * @param  : buf , string to store output value
 * @param  : val, value to be converted.
 * @return : Returns length of new string
 */
int
int_to_str(char *buf , uint32_t val);

#ifdef __cplusplus
}
#endif
#endif /* PFCP_H */
