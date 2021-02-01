// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <time.h>
#include <rte_hash_crc.h>
#include <rte_errno.h>
#include "pfcp.h"
#include "cp_log.h"
#include "gen_utils.h"
#include "tables/tables.h"

/*VS:TODO: Need to revist this for hash size */

#define TIMESTAMP_LEN 14

#define MAX_PDN_HASH_SIZE (1 << 8)

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

/* VS: Need to decide the base value of call id */
/* const uint32_t call_id_base_value = 0xFFFFFFFF; */
const uint32_t call_id_base_value = 0x00000000;
static uint32_t call_id_offset;
static uint64_t dp_sess_id_offset;
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

/**
 * @brief  : Get the system current timestamp.
 * @param  : timestamp is used for storing system current timestamp
 * @return : Returns 0 in case of success
 */
static uint8_t
get_timestamp(char *timestamp)
{

	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);

	strftime(timestamp, MAX_LEN, "%Y%m%d%H%M%S", tmp);
	return 0;
}

/**
 * @brief  : Generate CCR session id with the combination of timestamp and call id
 * @param  : str_buf is used to store generated session id
 * @param  : timestamp is used to pass timestamp
 * @param  : value is used to pas call id
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
gen_sess_id_string(char *str_buf, char *timestamp , uint32_t value)
{
	char buf[MAX_LEN] = {0};
	int len = 0;

	if (timestamp == NULL)
	{
		LOG_MSG(LOG_ERROR, "Time stamp is NULL ");
		return -1;
	}

	/* itoa(value, buf, 10);  10 Means base value, i.e. indicate decimal value */
	len = int_to_str(buf, value);

	if(buf[0] == 0)
	{
		LOG_MSG(LOG_ERROR, "Failed coversion of integer to string, len:%d ", len);
		return -1;
	}

	sprintf(str_buf, "%s%s", timestamp, buf);
	return 0;
}

/**
 * Generate the CALL ID
 */
uint32_t
generate_call_id(void)
{
	uint32_t call_id = 0;
	call_id = call_id_base_value + (++call_id_offset);

	return call_id;
}

/**
 * Retrieve the call id from the CCR session id.
 */
int
retrieve_call_id(char *str, uint32_t *call_id)
{
	uint8_t idx = 0, index = 0;
	char buf[MAX_LEN] = {0};

	if(str == NULL)
	{
		LOG_MSG(LOG_ERROR, "String is NULL");
		return -1;
	}

	idx = TIMESTAMP_LEN; /* TIMESTAMP STANDARD BYTE SIZE */
	for(;str[idx] != '\0'; ++idx)
	{
		buf[index] = str[idx];
		++index;
	}

	*call_id = atoi(buf);
	if (*call_id == 0) {
		LOG_MSG(LOG_ERROR, "Call ID not found");
		return -1;
	}
	return 0;
}

/**
 * Return the CCR session id.
 */
int8_t
gen_sess_id_for_ccr(char *sess_id, uint32_t call_id)
{
	char timestamp[MAX_LEN] = {0};

	get_timestamp(timestamp);

	if((gen_sess_id_string(sess_id, timestamp, call_id)) < 0)
	{
		LOG_MSG(LOG_ERROR, "Failed to generate session id for CCR");
		return -1;
	}
	return 0;
}


/**
 * Generate the SESSION ID
 */
uint64_t
generate_dp_sess_id(uint64_t cp_sess_id)
{
	uint64_t dp_sess_id = 0;

	dp_sess_id = ((++dp_sess_id_offset << 32) | cp_sess_id);

	return dp_sess_id;
}

