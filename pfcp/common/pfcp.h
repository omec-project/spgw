// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef PFCP_H
#define PFCP_H

#include "gx_app/include/gx_struct.h"
#include "pfcp_struct.h"
#include "pfcp_cp_struct.h"
#include "gtpv2_ie.h"
#include "ue.h"


struct rte_hash *pfcp_cntxt_hash;
struct rte_hash *pdr_entry_hash;
struct rte_hash *qer_entry_hash;
struct rte_hash *urr_entry_hash;
struct rte_hash *rule_name_bearer_id_map_hash;

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

}__attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));

/**
 * @brief  : Create a  hash table to maintain the PDR, QER, FAR and BAR information.
 * @param  : void
 * @return : Does not return anything
 */
void
init_pfcp_tables(void);

/**
 * @brief  : Add Rule name and bearer information in the table.
 * @param  : rule_key
 * @param  : bearer
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_rule_name_entry(const rule_name_key_t rule_key, bearer_id_t *bearer);

/**
 * @brief  : Add pfcp context information in the table.
 * @param  : session_id
 * @param  : resp, pfcp context details
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_pfcp_cntxt_entry(uint64_t session_id, struct pfcp_cntxt *resp);

/**
 * @brief  : Add PDR information in the table.
 * @param  : rule id
 * @param  : pdr context
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_pdr_entry(uint16_t rule_id, pdr_t *cntxt);

/**
 * @brief  : Add QER information in the table.
 * @param  : qer id
 * @param  : qer context
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_qer_entry(uint32_t qer_id, qer_t *cntxt);

/**
 * @brief  : Add URR information in the table.
 * @param  : urr id
 * @param  : urr context
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_urr_entry(uint32_t urr_id, urr_t *cntxt);

/**
 * @brief  : Retrive Rule Name entry.
 * @param  : rule_key
 * @return : Return bearer id on success , -1 otherwise
 */
int8_t
get_rule_name_entry(const rule_name_key_t rule_key);

/**
 * @brief  : Retrive pfcp context entry.
 * @param  : session id
 * @return : Returns pointer to pfcp context, NULL otherwise
 */
struct pfcp_cntxt *
get_pfcp_cntxt_entry(uint64_t session_id);

/**
 * @brief  : Retrive PDR entry.
 * @param  : rule id
 * @return : Returns pointer to pdr context, NULL otherwise
 */
pdr_t *get_pdr_entry(uint16_t rule_id);

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
 * @brief  : Retrive QER entry.
 * @param  : qer_id
 * @return : Returns pointer to qer context on success , NULL otherwise
 */
qer_t *get_qer_entry(uint32_t qer_id);

/**
 * @brief  : Retrive URR entry.
 * @param  : urr_id
 * @return : Returns pointer to urr context on success , NULL otherwise
 */
urr_t *get_urr_entry(uint32_t urr_id);

/**
 * @brief  : Delete Rule Name entry from Rule and Bearer Map table.
 * @param  : rule key
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_rule_name_entry(const rule_name_key_t rule_key);

/**
 * @brief  : Delete PDR entry from QER table.
 * @param  : pdr id
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_pdr_entry(uint16_t pdr_id);

/**
 * @brief  : Delete QER entry from QER table.
 * @param  : qer id
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_qer_entry(uint32_t qer_id);

/**
 * @brief  : Delete URR entry from URR table.
 * @param  : urr id
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_urr_entry(uint32_t urr_id);

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
 * @brief  : Generate the QER ID
 * @param  : void
 * @return : Returns qer id  on success , 0 otherwise
 */
uint32_t
generate_qer_id(void);

/**
 * @brief  : Generate the CALL ID
 * @param  : void
 * @return : Returns call id  on success , 0 otherwise
 */
uint32_t
generate_call_id(void);

/**
 * @brief  : Generate the Sequence
 * @param  : void
 * @return : Returns sequence number on success , 0 otherwise
 */
uint32_t
generate_rar_seq(void);

/**
 * @brief  : Retrieve Call ID from CCR Session ID
 * @param  : str represents CCR session ID
 * @param  : call_id , variable to store retrived call id
 * @return : Returns 0  on success , 0 otherwise
 */
int
retrieve_call_id(char *str, uint32_t *call_id);

/**
 * @brief  : Generate the SESSION ID
 * @param  : cp session id
 * @return : Returns dp session id  on success , 0 otherwise
 */
uint64_t
generate_dp_sess_id(uint64_t cp_sess_id);

/**
 * @brief  : Generate the CCR Session ID with combination of timestamp and call id.
 * @param  : sess id
 * @param  : call id
 * @return : Returns 0 on success
 */
int8_t
gen_sess_id_for_ccr(char *sess_id, uint32_t call_id);

/**
 * @brief  : Parse GX CCA message and fill ue context
 * @param  : cca holds data from gx cca message
 * @param  : _context , ue context to be filled
 * @return : Returns 0 on success, -1 otherwise
 */
int8_t
parse_gx_cca_msg(GxCCA *cca, pdn_connection_t **_pdn);

/**
 * Updates the already existing bearer
 */

int16_t
gx_update_bearer_req(pdn_connection_t *pdn);

/**
 * @brief  : Parse GX RAR message.
 * @param  : rar holds data from gx rar message
 * @return : Returns 0 on success, -1 otherwise
 */
int8_t
parse_gx_rar_msg(GxRAR *rar);
void pfcp_modify_rar_trigger_timeout(void *data);

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

int8_t
compare_default_bearer_qos(bearer_qos_ie *default_bearer_qos,
		bearer_qos_ie *rule_qos);

#endif /* PFCP_H */