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
 */
uint32_t
generate_urr_id(void)
{
	uint32_t id = 0;

	id = urr_base_rule_id + (++urr_rule_id_offset);

	return id;
}

/**
 * Generate the QER ID
 */
uint32_t
generate_qer_id(void)
{
	uint32_t id = 0;

	id = qer_base_rule_id + (++qer_rule_id_offset);

	return id;
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


