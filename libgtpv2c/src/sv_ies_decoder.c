// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "../include/sv_ies_decoder.h"
#include "../include/enc_dec_bits.h"

/**
* Decodes stn_sr to buffer.
* @param buf
*   buffer to store decoded values.
* @param value
    stn_sr
* @return
*   number of encoded bytes.
*/
int decode_gtp_stn_sr_ie(uint8_t *buf,
    gtp_stn_sr_ie_t *value)
{
    uint16_t total_decoded = 0;
    uint16_t decoded = 0;

    value->nanpi = decode_bits(buf, total_decoded, 8, &decoded);
    total_decoded += decoded;
    value->filler = decode_bits(buf, total_decoded, 4, &decoded);
    total_decoded += decoded;
    value->number_digit = decode_bits(buf, total_decoded, 4, &decoded);
    total_decoded += decoded;

    return total_decoded/CHAR_SIZE;
}

