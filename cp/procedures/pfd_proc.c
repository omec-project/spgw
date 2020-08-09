
// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdint.h>
#include "clogger.h"
#include "sm_hand.h"
#include "rte_common.h"
#include "cp_config.h"
#include "gw_adapter.h"

int
pfd_management_handler(void *data, void *unused_param)
{
	clLog(sxlogger, eCLSeverityDebug,
		"Pfcp Pfd Management Response Recived Successfully \n");

	RTE_SET_USED(data);
	RTE_SET_USED(unused_param);
	return 0;
}
