// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "rte_common.h"
#include "rte_errno.h"
#include "pfcp.h"
#include "gx_interface.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "ipc_api.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_session.h"
#include "cp_config.h"
#include "clogger.h"
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gw_adapter.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "upf_struct.h"
#include "cp_events.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_enum.h"
#include "cp_transactions.h"
#include "cp_peer.h"


int create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt) 
{
    int ret;
    upf_context_t *upf_context = NULL;
	upf_context  = rte_zmalloc_socket(NULL, sizeof(upf_context_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());

	if (upf_context == NULL) {
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate upf context: "
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);

		return -1;
	}
    memset(upf_context, 0, sizeof(upf_context_t));
    *upf_ctxt = upf_context;
	bzero(upf_context->upf_sockaddr.sin_zero, sizeof(upf_context->upf_sockaddr.sin_zero));
	upf_context->upf_sockaddr.sin_family = AF_INET;
	upf_context->upf_sockaddr.sin_port = htons(cp_config->upf_pfcp_port);
	upf_context->upf_sockaddr.sin_addr.s_addr = upf_ip; 

	ret = upf_context_entry_add(&upf_ip, upf_context);
	if (ret) {
		clLog(clSystemLog, eCLSeverityCritical, "%s : Error: %d \n", __func__, ret);
		return -1;
	}

    LIST_INIT(&upf_context->pendingProcs);
    return 0;

} 
