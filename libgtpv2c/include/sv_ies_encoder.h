// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __SV_IES_ENCODE_H__
#define __SV_IES_ENCODE_H__
#include "sv_ies.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
* Encodes stn_sr to buffer.
* @param buf
*   buffer to store encoded values.
* @param value
    stn_sr
* @return
*   number of encoded bytes.
*/
int encode_gtp_stn_sr_ie(gtp_stn_sr_ie_t *value,
    uint8_t *buf);

#ifdef __cplusplus
}
#endif
#endif /*__GTP_IES_ENCODE_H__*/
