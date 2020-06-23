// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <string.h>

#include "util.h"

int
gtpv2c_buf_memcpy(gtpv2c_buffer_t *buf, void *src, uint16_t src_len)
{
	if (src_len > (GTPV2C_BUF_MAX_LEN - buf->len))
		return -1;

	memcpy(buf->val + buf->len, src, src_len);
	buf->len += src_len;

	return 0;
}

