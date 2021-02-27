// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
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
#include "spgw_config_struct.h"
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


int create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt) 
{
    int ret;
    upf_context_t *upf_context = NULL;
	upf_context  = calloc(1, sizeof(upf_context_t));

	if (upf_context == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate upf context: ");
		return -1;
	}
    *upf_ctxt = upf_context;
    memset(upf_context->upf_sockaddr.sin_zero, '\0', sizeof(upf_context->upf_sockaddr.sin_zero));
	upf_context->upf_sockaddr.sin_family = AF_INET;
	upf_context->upf_sockaddr.sin_port = htons(cp_config->pfcp_port);
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

/* Requirement: 
 * For now I am using linux system call to do the service name dns resolution...
 * 3gpp based DNS lookup of NRF support would be required to locate UPF. 
 */
struct in_addr native_linux_name_resolve(const char *name)
{
    struct in_addr ip = {0};
    LOG_MSG(LOG_INFO, "DNS Query - %s ",name);
    struct addrinfo hints;
    struct addrinfo *result=NULL, *rp=NULL;
    int err;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    err = getaddrinfo(name, NULL, &hints, &result);
    if (err == 0)
    {
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            if(rp->ai_family == AF_INET)
            {
                struct sockaddr_in *addrV4 = (struct sockaddr_in *)rp->ai_addr;
                LOG_MSG(LOG_DEBUG, "Received DNS response. name %s mapped to  %s", name, inet_ntoa(addrV4->sin_addr));
                return addrV4->sin_addr;
            }
        }
    }
    LOG_MSG(LOG_ERROR, "DNS Query for %s failed with error %s", name, gai_strerror(err));
    return ip;
}

upf_context_t*
get_upf_context(user_plane_profile_t *upf_profile) 
{
    struct in_addr ip = {0};
    if(upf_profile == NULL) {
        return NULL;
    }

    if(upf_profile->upf_addr == 0) {
        ip = native_linux_name_resolve(upf_profile->user_plane_service); 
        upf_profile->upf_addr = ip.s_addr; 
    }

    if(upf_profile->upf_addr != 0) {
        upf_context_t *upf_context = NULL;
        upf_context = (upf_context_t*)upf_context_entry_lookup(upf_profile->upf_addr);
        if(upf_context == NULL) {
            create_upf_context(upf_profile->upf_addr, &upf_context);
        }
        return upf_context;
    }
	return NULL; 
}
