// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdint.h>

void generate_teid(uint32_t ue, uint8_t bearer_id,
			uint32_t max_ue_sess, uint32_t *teid)
{
	*teid = max_ue_sess - ue + bearer_id;
}
