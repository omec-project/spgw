/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __APN_STRUCT_H
#define __APN_STRUCT_H
#include <stdint.h>
#include <stdlib.h>
/**
 * @brief  : Maintains apn related information
 */

#define MAX_NETCAP_LEN               (64)
typedef struct apn {
	char *apn_name_label;
	int apn_usage_type;
	char apn_net_cap[MAX_NETCAP_LEN];
	size_t apn_name_length;
	uint8_t apn_idx;
} apn_t;

#endif