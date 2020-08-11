// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _TABLES_H
#define _TABLES_H
#include "upf_struct.h"
#include "ue.h"
#include "cp_peer_struct.h"
#include "gx_interface.h"
/**
 * @brief  : Creates upf context hash
 * @param  : No param
 * @return : Returns nothing
 */
void
create_upf_context_hash(void);


/**
 * @brief  : Add entry to upf conetxt hash
 * @param  : upf_ip, up ip address
 * @param  : entry ,entry to be added
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
upf_context_entry_add(uint32_t *upf_ip, upf_context_t *entry);

/**
 * @brief  : search entry in upf hash using ip
 * @param  : upf_ip, key to search entry
 * @param  : entry, variable to store search result
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
upf_context_entry_lookup(uint32_t upf_ip, upf_context_t **entry);


int upf_context_delete_entry(uint32_t upf_ip);


void create_ue_hash(void);

int 
ue_context_entry_add_imsiKey(ue_context_t *context);

int
ue_context_entry_lookup_imsiKey(uint64_t imsi, ue_context_t **entry);

int 
ue_context_delete_entry_imsiKey(uint64_t upf_ip);

int 
ue_context_entry_add_teidKey(uint32_t teid, ue_context_t *context);

/**
 * @brief  : Retrive UE Context entry from UE Context table.
 * @param  : teid_key, key to search context
 * @param  : context, structure to store retrived context
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_ue_context(uint32_t teid, ue_context_t **entry);

int ue_context_delete_entry_teidKey(uint32_t teid);
void create_pdn_hash(void);

int pdn_context_delete_entry_teidKey(uint32_t teid);

int 
pdn_context_entry_add_teidKey(uint32_t teid, pdn_connection_t *context);

/**
 * @brief  : Retrive PDN entry from PDN table.
 * @param  : teid_key, key for search
 * @param  : pdn, structure to store retrived pdn
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_pdn_context(uint32_t teid, pdn_connection_t **entry);


void create_bearer_hash(void);

int 
bearer_context_entry_add_teidKey(uint32_t teid, eps_bearer_t *context);

/**
 * Retrive Bearer entry from Bearer table.
 */
int
get_bearer_by_teid(uint32_t teid, eps_bearer_t **entry);

int bearer_context_delete_entry_teidKey(uint32_t teid);

/**
 * @brief  : Create recovery time hash table
 * @param  : No param
 * @return : Returns nothing
 */
void
create_heartbeat_hash_table(void);

/**
 * @brief  : Add ip address to hearbeat hash
 * @param  : peer_addr, ip address to be added
 * @param  : recover_timei, recovery time stamp
 * @return : Returns nothing
 */
void
add_ip_to_heartbeat_hash(struct sockaddr_in *peer_addr, uint32_t recover_time);

/**
 * @brief  : Delete ip address from heartbeat hash
 * @param  : peer_addr, ip address to be removed
 * @return : Returns nothing
 */
void
delete_entry_heartbeat_hash(struct sockaddr_in *peer_addr);

/**
 * @brief  : Add data to hearbeat hash table
 * @param  : ip, ip address to be added
 * @param  : recov_time, recovery timestamp
 * @return : Returns nothing
 */
int
add_data_to_heartbeat_hash_table(uint32_t *ip, uint32_t *recov_time);

/**
 * @brief  : Delete hearbeat hash table
 * @param  : No param
 * @return : Returns nothing
 */
void
clear_heartbeat_hash_table(void);

/**
 * @brief  : Initiatizes peer echo table 
 * @param  : No param
 * @return : Returns nothing
 */

void echo_table_init(void);

int 
add_peer_entry(uint32_t ipaddr, peerData_t *peer);

int
get_peer_entry(uint32_t ipaddr, peerData_t **entry);

void del_entry_from_hash(uint32_t ipAddr);

/**
 * @brief  : Add session entry in session table.
 * @param  : sess_id, session id
 * @param  : context, structure to store ue_context info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
add_sess_entry_seid(uint64_t sess_id, struct ue_context *context);

/**
 * @brief  : Retrive session entry from session table.
 * @param  : sess_id, session id
 * @param  : resp, structure to store session info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_sess_entry_seid(uint64_t sess_id, struct ue_context **context);



/**
 * @brief  : Delete session entry from session table.
 * @param  : sess_id, session id
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
del_sess_entry_seid(uint64_t sess_id);

/**
 * @brief  : Creates gx conetxt hash
 * @param  : No param
 * @return : Returns nothing
 */
void
create_gx_context_hash(void);

/**
- * @brief  : Add entry into gx context hash
- * @param  : sess_id , key to add entry
- * @param  : entry , entry to be added
- * @return : Returns 0 in case of success , -1 otherwise
- */
int
gx_context_entry_add(uint8_t *sess_id, gx_context_t *entry);

/**
- * @brief  : search entry in gx context hash
- * @param  : sess_id , key to add entry
- * @param  : entry , entry to be added
- * @return : Returns 0 in case of success , -1 otherwise
- */
int
get_gx_context(uint8_t *sess_id, gx_context_t **entry);

int remove_gx_context(uint8_t *sess_id);

void create_pdn_callid_hash(void);
/**
 * @brief  : Add PDN Connection information in the table.
 * @param  : call_id
 * @param  : pdn connection details
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
add_pdn_conn_entry(uint32_t call_id, pdn_connection_t *pdn);

/**
 * @brief  : Retrive PDN connection entry.
 * @param  : call id
 * @return : Returns pointer to pdn entry on success , NULL otherwise
 */
pdn_connection_t *get_pdn_conn_entry(uint32_t call_id);

/**
 * @brief  : Delete context entry from pfcp context table.
 * @param  : session id
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_pfcp_cntxt_entry(uint64_t session_id);

/**
 * @brief  : Delete PDN connection entry from PDN conn table.
 * @param  : call_id
 * @return : Returns 0 on success , -1 otherwise
 */
uint8_t
del_pdn_conn_entry(uint32_t call_id);


#endif
