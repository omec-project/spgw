// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _SESION_CDR_H
#define _SESION_CDR_H
/**
 * @file
 * This file contains function prototypes of User data
 * charging per session records.
 */
#include "main.h"


/**
 * @brief  : Open Session Charging data record file.
 * @param  : No param
 * @return : Returns nothing
 */
void
sess_cdr_init(void);

/**
 * @brief  : Clear the record file content.
 * @param  : No param
 * @return : Returns nothing
 */
void
sess_cdr_reset(void);

/**
 * @brief  : Export CDR record to file.
 * @param  : session, dp bearer session.
 * @param  : name, string to identify the type of CDR.
 * @param  : id, identification number based on cdr type. It can be
 *           either bearerid, adc rule id, flow id or rating group.
 * @param  : charge_record, cdr structure which holds the pkt counts and bytes.
 * @return : Returns nothing
 */
void
export_cdr_record(struct dp_session_info *session, char *name,
			uint32_t id, struct ipcan_dp_bearer_cdr *charge_record);
#endif /* _SESION_CDR_H */
