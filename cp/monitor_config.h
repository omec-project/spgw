// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __MONITOR_CONFIG__
#define __MONITOR_CONFIG__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*configCbk) (char *, uint32_t flags);
void watch_config_change(const char *config_file, configCbk cbk);

#ifdef __cplusplus
}
#endif
#endif
