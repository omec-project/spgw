// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef PFCP_CP_UTIL_H
#define PFCP_CP_UTIL_H

#include <sys/sysinfo.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "ue.h"
#include "gtp_messages.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  : Returns system seconds since boot
 * @param  : No param
 * @return : Returns number of system seconds since boot
 */
long
uptime(void);

/**
 * @brief  : Checks current ntp timestamp
 * @param  : No param
 * @return : Returns timestamp value
 */
uint32_t
current_ntp_timestamp(void);

/**
 * @brief  : Converts timeval to ntp format
 * @param  : tv, input timeval
 * @param  : ntp, converted ntp time
 * @return : Returns nothing
 */
void
time_to_ntp(struct timeval *tv, uint8_t *ntp);

#ifdef __cplusplus
}
#endif
#endif /* PFCP_UTIL_H */
