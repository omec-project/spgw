// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __SV_IES_H
#define __SV_IES_H

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "gtp_ies.h"
#define CHAR_SIZE 8

#define GTP_IE_STN_SR 51

typedef struct gtp_stn_sr_ie_t { 
    ie_header_t header;
    uint8_t nanpi;
    uint8_t filler :4;
    uint8_t number_digit :4;
} gtp_stn_sr_ie_t;
#endif