#ifndef _TABLES_H
#define _TABLES_H
#include "upf_struct.h"
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
#endif
