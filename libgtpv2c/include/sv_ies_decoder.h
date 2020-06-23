// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SV_IES_DECODE_H__
#define __SV_IES_DECODE_H__
#include "sv_ies.h"
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

#endif /*__GTP_IES_DECODE_H__*/