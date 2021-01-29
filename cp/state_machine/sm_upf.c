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
#include "csid_cp_cleanup.h"
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gx_error_rsp.h"
#include "cp_main.h"
#include "upf_struct.h"
#include "cp_events.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_enum.h"
#include "cp_transactions.h"
#include "cp_peer.h"
#include "tables/tables.h"
#include "cp/cp_log.h"


int create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt) 
{
    int ret;
    upf_context_t *upf_context = NULL;
	upf_context  = rte_zmalloc_socket(NULL, sizeof(upf_context_t),
				RTE_CACHE_LINE_SIZE, rte_socket_id());

	if (upf_context == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate upf context: "
				"%s ", rte_strerror(rte_errno));

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
		LOG_MSG(LOG_ERROR, "Error: %d ", ret);
		return -1;
	}

    LIST_INIT(&upf_context->pending_sub_procs);
    TAILQ_INIT(&upf_context->pending_node_procs);
    return 0;

} 

/* We should queue the event if required */
void
start_upf_procedure(proc_context_t *proc_ctxt, msg_info_t *msg)
{
    upf_context_t *upf_context = (upf_context_t *)proc_ctxt->upf_context;
    TAILQ_INSERT_TAIL(&upf_context->pending_node_procs, proc_ctxt, next_node_proc); 
    /* Logic here to decide if we want to run the procedure right away or delay
     * the execution */
    proc_ctxt = TAILQ_FIRST(&upf_context->pending_node_procs);
    LOG_MSG(LOG_DEBUG, "Start procedure number (%d) ",proc_ctxt->proc_type);
    switch(proc_ctxt->proc_type) {
        case PFCP_ASSOC_SETUP_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            TAILQ_REMOVE(&upf_context->pending_node_procs, proc_ctxt,next_node_proc);
        }
    }
    return;
}

