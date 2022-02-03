// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SV_IES_DECODE_H__
#define __SV_IES_DECODE_H__
#include "sv_ies.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
* Decodes stn_sr to buffer.
* @param buf
*   buffer to store decoded values.
* @param value
    stn_sr
* @return
*   number of decoded bytes.
*/
int decode_gtp_stn_sr_ie(uint8_t *buf,
    gtp_stn_sr_ie_t *value);

#ifdef __cplusplus
}
#endif
#endif /*__GTP_IES_DECODE_H__*/
