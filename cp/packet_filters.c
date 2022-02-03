// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "ue.h"
#include "util.h"
#include "packet_filters.h"
#include "vepc_cp_dp_api.h"
#include "gen_utils.h"
#include "cp_config_defs.h"
#include "cp_log.h"
#include "assert.h"

const char *direction_str[] = {
        "TFT_DIRECTION_NONE",
		"DOWNLINK_ONLY",
		"UPLINK_ONLY",
		"BIDIRECTIONAL",
};


struct mtr_entry *mtr_profiles[METER_PROFILE_SDF_TABLE_SIZE] = {
		[0] = NULL, /* index = 0 is invalid */
};

struct pcc_rules *pcc_filters[PCC_TABLE_SIZE] = {
		[0] = NULL, /* index = 0 is invalid */
};

pkt_fltr_t *sdf_filters[SDF_FILTER_TABLE_SIZE] = {
		[0] = NULL, /* index = 0 is invalid */
};
packet_filter_t *packet_filters[SDF_FILTER_TABLE_SIZE] = {
		[0] = NULL, /* index = 0 is invalid */
};

uint16_t num_mtr_profiles;
uint16_t num_packet_filters = FIRST_FILTER_ID;
uint16_t num_sdf_filters = FIRST_FILTER_ID;
uint16_t num_pcc_filter = FIRST_FILTER_ID;
uint32_t num_adc_rules;
uint32_t adc_rule_id[MAX_ADC_RULES];
uint64_t cbs;
uint64_t ebs;
uint16_t ulambr_idx;
uint16_t dlambr_idx;
#define 	likely(x)   __builtin_expect(!!(x), 1)
#define 	unlikely(x)   __builtin_expect(!!(x), 0)

int
get_packet_filter_id(const pkt_fltr_t *pf)
{
       uint16_t index;
       for (index = FIRST_FILTER_ID; index < num_packet_filters; ++index) {
               if (!memcmp(pf, &packet_filters[index]->pkt_fltr,
                               sizeof(pkt_fltr_t)))
                       return index;
       }
       return -ENOENT;
}

uint8_t
get_packet_filter_direction(uint16_t index)
{
       return packet_filters[index]->pkt_fltr.direction;
}

packet_filter_t *
get_packet_filter(uint16_t index)
{
       if (unlikely(index >= num_packet_filters))
               return NULL;
       return packet_filters[index];
}

void
reset_packet_filter(pkt_fltr_t *pf)
{
	pf->direction = TFT_DIRECTION_BIDIRECTIONAL;
    pf->remote_ip_addr.s_addr = 0;
    pf->remote_ip_mask = 0;
    pf->remote_port_low = 0;
    pf->remote_port_high = UINT16_MAX;
    pf->proto = 0;
    pf->proto_mask = 0;
    pf->local_ip_addr.s_addr = 0;
    pf->local_ip_mask = 0;
    pf->local_port_low = 0;
    pf->local_port_high = UINT16_MAX; 
    return;
}

