// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <time.h>
#include "pfcp.h"
#include "cp_log.h"
#include "gen_utils.h"
#include "pfcp_enum.h"


const uint8_t bar_base_rule_id = 0xFF;
static uint8_t bar_rule_id_offset;
const uint16_t pdr_base_rule_id = 0x0000;
static uint16_t pdr_rule_id_offset;
const uint32_t far_base_rule_id = 0x00000000;
static uint32_t far_rule_id_offset;
const uint32_t qer_base_rule_id = 0x00000000;
static uint32_t qer_rule_id_offset;
const uint32_t urr_base_rule_id = 0x00000000;
static uint32_t urr_rule_id_offset;

const uint32_t rar_base_rule_id = 0x00000000;
static uint32_t rar_rule_id_offset;

/**
 * Generate the BAR ID
 */
uint8_t
generate_bar_id(void)
{
	uint8_t id = 0;

	id = bar_base_rule_id + (++bar_rule_id_offset);

	return id;
}

/**
 * Generate the PDR ID
 */
uint16_t
generate_pdr_id(void)
{
	uint16_t id = 0;

	id = pdr_base_rule_id + (++pdr_rule_id_offset);

	return id;
}

/**
 * Generate the FAR ID
 */
uint32_t
generate_far_id(void)
{
	uint32_t id = 0;

	id = far_base_rule_id + (++far_rule_id_offset);

	return id;
}

/**
 * Generate the URR ID
 * FIXME - longevity. Make sure this URR id is not in use  
 */
uint32_t
generate_urr_id(uint32_t iface)
{
	uint32_t id = 0;

	id = urr_base_rule_id + (++urr_rule_id_offset);
    if (iface == SOURCE_INTERFACE_VALUE_ACCESS) {
        id = id | 0x20000000; 
    } else if (iface == SOURCE_INTERFACE_VALUE_CORE) {
        id = id | 0x40000000; 
    } else {
        id = id | 0x30000000; 
    } 
	return id;
}

bool isCommonURR(uint32_t id)
{
    if ((id & 0x30000000) == 0x30000000)
        return true;
    return false;
}

/**
 * Generate the QER ID
 */
// 0100 : Core
// 0010 : Access
// 0011 : common 
uint32_t
generate_qer_id(uint32_t iface)
{
	uint32_t id = 0;

    // FIXME : need to make sure we dont overflow the ID space & we don't 
    // assign duplicate IDs 

	id = qer_base_rule_id + (++qer_rule_id_offset);
    if (iface == SOURCE_INTERFACE_VALUE_ACCESS) {
        id = id | 0x20000000; // set 2 bits at MSB to 10
    } else if (iface == SOURCE_INTERFACE_VALUE_CORE) {
        id = id | 0x40000000; // set 2 bits at MSB to 01
    } else {
        // common QER 
        id = id | 0x30000000; 
    }
	return id;
}

bool isCommonQER(uint32_t id)
{
    if ((id & 0x30000000) == 0x30000000) {
        //LOG_MSG(LOG_DEBUG, "isCoreQER true = %x ", id);
        return true;
    }
    //LOG_MSG(LOG_DEBUG, "isCoreQER false = %x ", id);
    return false;
}

/**
 * Generate the Sequence
 */
uint32_t
generate_rar_seq(void)
{
	uint32_t id = 0;

	id = rar_base_rule_id + (++rar_rule_id_offset);

	return id;
}


/**
 * Convert the decimal value into the string.
 */
int
int_to_str(char *buf , uint32_t val)
{
	uint8_t tmp_buf[10] = {0};
	uint32_t cnt = 0, num = 0;
	uint8_t idx = 0;

	while(val)
	{
		num = val%10;
		tmp_buf[cnt] = (uint8_t)(num + 48);
		val/=10;
		++cnt;
	}

	tmp_buf[cnt] = '\0';
	--cnt;

	for(; tmp_buf[idx]; ++idx)
	{

		buf[idx] = tmp_buf[cnt];
		--cnt;
	}

	buf[idx] = '\0';
	return idx;

}


