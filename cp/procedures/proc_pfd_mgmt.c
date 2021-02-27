// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdint.h>
#include "sm_hand.h"
#include "spgw_config_struct.h"
#include "cp_log.h"

int
pfd_management_handler(void *data, void *unused_param)
{
	LOG_MSG(LOG_DEBUG,
		"Pfcp Pfd Management Response Recived Successfully \n");

    LOG_MSG(LOG_NEVER, "data = %p", data);
    LOG_MSG(LOG_NEVER, "unused_param = %p", unused_param);
	return 0;
}
