// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/*
 * Keep only trans information in this file..No APIS to be added in this file 
 */
#ifndef __TRANS_STRUCT_H
#define __TRANS_STRUCT_H
#include <stdint.h>
#include "timer.h"

/**
 * @brief  : Maintains context of upf
 */
struct transData 
{
    uint8_t  iface;
    uint8_t  msg_type;
	uint32_t  dstIP;
	uint16_t  dstPort;
    uint32_t  sequence;
	uint16_t buf_len;
	uint8_t buf[1024];
	gstimerinfo_t  rt;
	void* upf_ctxt;
    void* ue_context;
    uint8_t itr_cnt;
};
typedef struct transData transData_t; 
#endif
