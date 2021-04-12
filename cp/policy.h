// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef POLICY_H
#define POLICY_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  : Maintains sdf packet filter information
 */
typedef struct sdf_pkt_fltr {
	uint8_t proto_id;
	uint8_t proto_mask;
	uint8_t direction;
	uint8_t action;
	uint8_t local_ip_mask;
	uint8_t remote_ip_mask;
	uint16_t local_port_low;
	uint16_t local_port_high;
	uint16_t remote_port_low;
	uint16_t remote_port_high;
	struct in_addr local_ip_addr;
	struct in_addr remote_ip_addr;
} sdf_pkt_fltr_t;


/**
 * @brief  : Maintains flow description data
 */
typedef struct flow_description {
	int32_t flow_direction;
	sdf_pkt_fltr_t sdf_flw_desc;
	char sdf_flow_description[512];
	uint16_t flow_desc_len;
}flow_desc_t;


/**
 * @brief  : Maintains information about dynamic rule
 */
struct dynamic_rule {
	int32_t online;
	int32_t offline;
	int32_t flow_status;
	int32_t reporting_level;
	uint32_t precedence;
	uint32_t service_id;
	uint32_t rating_group;
	uint32_t def_bearer_indication;
	char rule_name[256];
	char af_charging_id_string[256];
	bearer_qos_ie qos;
    uint32_t ul_ambr;
    uint32_t dl_ambr;
	/* Need to think on it */
	uint8_t num_flw_desc;
	flow_desc_t flow_desc[32];
	pdr_t *pdr[2];
};
typedef struct dynamic_rule dynamic_rule_t;

#ifdef __cplusplus
}
#endif
#endif /* POLICY_H */
