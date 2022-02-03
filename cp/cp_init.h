// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#ifndef _CP_H_
#define _CP_H_

#include <pcap.h>
#include <byteswap.h>
#include "cp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif


extern int s11_pcap_fd;
extern int s5s8_sgwc_fd;
extern int s5s8_pgwc_fd;
extern int pfcp_sgwc_fd ;

/**
 * @brief  : Initializes Control Plane data structures, packet filters, and calls for the
 *           Data Plane to create required tables
 * @param  : void
 * @return : Void
 */
void
init_cp(void);

/**
 * @brief  : Updates restart counter Value
 * @param  : No param
 * @return : Returns nothing
 */
uint8_t
update_rstCnt(void);

/**
 * @brief  : starts the timer thread
 * @param  : No param
 * @return : Returns nothing
 */


void init_timer_thread(void);

void *delayed_task_handler(void *data);

#ifdef __cplusplus
}
#endif
#endif
