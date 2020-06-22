// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef __NSB_TEST_UTIL_H
#define __NSB_TEST_UTIL_H
/**
 * Generate randum tunnel id
 *
 * @param ue
 *	ue - ue id.
 * @param bearer_id
 *	bearer_id - ue bearer id.
 * @param max_ue_sess
 *	max_ue_sess - max ue session.
 * @param teid
 *	teid - tunnel id.
 *
 * @return
 *	None
*/
void generate_teid(uint32_t ue, uint8_t bearer_id,
			uint32_t max_ue_sess, uint32_t *teid);

#endif
