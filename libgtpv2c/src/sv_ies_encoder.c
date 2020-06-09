// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "../include/sv_ies_encoder.h"
#include "../include/enc_dec_bits.h"
#include "../include/gtp_ies_encoder.h"

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
    uint8_t *buf)
{
    uint16_t encoded = 0;

    encoded += encode_ie_header_t(&value->header, buf + (encoded/CHAR_SIZE));
    encoded += encode_bits(value->nanpi, 8,  buf + (encoded/8), encoded % CHAR_SIZE);

    encoded += encode_bits(value->filler, 4,  buf + (encoded/8), encoded % CHAR_SIZE);

    encoded += encode_bits(value->number_digit, 4,  buf + (encoded/8), encoded % CHAR_SIZE);


    return encoded/CHAR_SIZE;
}

