// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "pfcp.h"
#include "gx_interface.h"
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
#include "gtpv2_interface.h"
#include "upf_struct.h"
#include "gen_utils.h"
#include "gx_error_rsp.h"
#include "upf_struct.h"
#include "cp_events.h"
#include "spgw_cpp_wrapper.h"
#include "pfcp_messages_encoder.h"
#include "pfcp_enum.h"
#include "cp_transactions.h"
#include "cp_peer.h"
#include "proc_pfcp_assoc_setup.h"
#include "cp_timer.h"
#include "upf_apis.h"
#include "poll.h"

struct threadData {
    int fd;
};

struct dnsMsg {
    pthread_mutex_t msg_mutex;
    bool req_valid;
    bool rsp_sent;
    char name[64];
    struct in_addr ip_addr;
};
typedef struct dnsMsg dnsMsg_t;

struct msgDataEnv {
    dnsMsg_t *data;
};
typedef struct msgDataEnv msgDataEnv_t;

void* dns_handler(void *data);

static struct in_addr native_linux_name_resolve(const char *name);

int 
create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt, const char *service_name) 
{
    int ret;
    upf_context_t *upf_context;

	upf_context  = (upf_context_t *)calloc(1, sizeof(upf_context_t));

	if (upf_context == NULL) {
		LOG_MSG(LOG_ERROR, "Failure to allocate upf context: ");
		return -1;
	}
    *upf_ctxt = upf_context;
    memset(upf_context->upf_sockaddr.sin_zero, '\0', sizeof(upf_context->upf_sockaddr.sin_zero));
	upf_context->upf_sockaddr.sin_family = AF_INET;
	upf_context->upf_sockaddr.sin_port = htons(cp_config->pfcp_port);
	upf_context->upf_sockaddr.sin_addr.s_addr = upf_ip; 

	// ADD only if IP address not zero
	if(upf_ip != 0) {
		ret = upf_context_entry_add(&upf_ip, upf_context);
		if (ret) {
			LOG_MSG(LOG_ERROR, "Failed to add UPF addresss [%s] in UPF context map.Error: %d ", inet_ntoa(*((struct in_addr *)&upf_ip)), ret);
			free(upf_context);
			return -1;
		}
	}
	ret = upf_context_entry_add_service(service_name, upf_context);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Failed to add service name [%s] in UPF context map.Error: %d ", service_name, ret);
		if(upf_ip != 0) {
			upf_context_delete_entry(upf_ip);
		}
		free(upf_context);
		return -1;
	}
    LIST_INIT(&upf_context->pending_sub_procs);
    TAILQ_INIT(&upf_context->pending_node_procs);
    return 0;

} 

// no response to echo or 
// echo rsp has new timestamp 
void
upf_down_event(uint32_t upf_ip)
{
    upf_context_t *upf_context = NULL; 
    upf_context = (upf_context_t *)upf_context_entry_lookup(upf_ip);

    if(upf_context != NULL) {
        upf_context->state = 0;
        upf_context->upf_sockaddr.sin_addr.s_addr = 0;
    }
    struct sockaddr_in upf_addr = {0};
    upf_addr.sin_addr.s_addr = upf_ip;

	delete_entry_heartbeat_hash(&upf_addr);

    uint16_t timeout = rand() % 30;
    schedule_pfcp_association(timeout, upf_context);
}

void
schedule_pfcp_association(uint16_t timeout, upf_context_t *upf_ctxt)
{
    static gstimerinfo_t  upf_pt;
    gstimerinfo_t  *timer;

    if(upf_ctxt == NULL) {
        timer = &upf_pt;
    } else {
        timer = &upf_ctxt->upf_pt; 
    }

    if (!gst_timer_init(timer, ttSingleShot, upfAssociationTimerCallback, timeout*1000, upf_ctxt)) {
        LOG_MSG(LOG_ERROR,"Failed to start timer ");
	    return;
    }

	if (startTimer(timer) < 0) {
		LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
	}
}

void
initiate_pfcp_association(upf_context_t *upf_context)
{
    struct in_addr peer = upf_context->upf_sockaddr.sin_addr;
    if(upf_context->state == 0) {
        LOG_MSG(LOG_INFO, "Initiate association setup with UPF %s", inet_ntoa(peer));
        proc_context_t *proc = alloc_pfcp_association_setup_proc(upf_context);
        start_upf_procedure(proc, (msg_info_t *)proc->msg_info);
    } else {
        LOG_MSG(LOG_INFO, "Don't Initiate association setup with UPF %s, state = %d ", inet_ntoa(peer), upf_context->state);
    }

}

void config_disable_upf(char* upf_service_name) 
{

    upf_context_t *upf_context = NULL;
    upf_context = (upf_context_t *)upf_context_entry_lookup_service(upf_service_name);
    if (upf_context == NULL) {
        LOG_MSG(LOG_ERROR,"NO UPF context found for service name - %s ", upf_service_name);
        return;
    }


    uint32_t upf_addr = upf_context->upf_sockaddr.sin_addr.s_addr;
    struct sockaddr_in upf_address = {0};
    upf_address.sin_addr.s_addr = upf_addr;

    /*FIXME :  peerData and upf context are not freed yet.*/
    peerData_t *md = (peerData_t *)get_peer_entry(upf_addr);
    del_entry_from_hash(upf_addr);
    if(md != NULL) {
        /* Stop transmit timer for specific Peer Node */
        stopTimer( &md->tt );
        /* Stop periodic timer for specific Peer Node */
        stopTimer( &md->pt );
        /* Deinit transmit timer for specific Peer Node */
        deinitTimer( &md->tt );
        /* Deinit transmit timer for specific Peer Node */
        deinitTimer( &md->pt );
    }

    upf_context->state = 0;
    upf_context_delete_entry(upf_addr);
    delete_entry_heartbeat_hash(&upf_address);
}

void
initiate_all_pfcp_association(void)
{
    bool schedule_association = false;

    #define MAX_UPF 100
    user_plane_service_names_t services[MAX_UPF]; // max
    memset(&services[0], 0, sizeof(services));

    int num = get_user_plane_services(&services[0], MAX_UPF);

    for(int i=0; i<num; i++) {
        upf_context_t *upf_context = get_upf_context(services[i].user_plane_service, services[i].global_address);
		if(upf_context == NULL) {
			schedule_association = true;
			continue;
		}
        if(upf_context->upf_sockaddr.sin_addr.s_addr == 0) {
            uint16_t timeout = rand() % 30;
            schedule_pfcp_association(timeout, upf_context); 
            LOG_MSG(LOG_DEBUG, "Failed to create UPF Context %s. Try after %d seconds", services[i].user_plane_service, timeout);
            continue;
        }

		if(upf_context->state == 0) {
		    LOG_MSG(LOG_INFO, "Initiate association setup with UPF %s", services[i].user_plane_service);
		    proc_context_t *proc = alloc_pfcp_association_setup_proc(upf_context);
		    start_upf_procedure(proc, (msg_info_t *)proc->msg_info);
		}
    }
    if(schedule_association == true) {
        uint16_t timeout = rand() % 30;
        schedule_pfcp_association(timeout, NULL);
    }
  
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
    LOG_MSG(LOG_DEBUG, "Start procedure %s ",proc_ctxt->proc_name);
    switch(proc_ctxt->proc_type) {
        case PFCP_ASSOC_SETUP_PROC: {
            proc_ctxt->handler(proc_ctxt, msg);
            TAILQ_REMOVE(&upf_context->pending_node_procs, proc_ctxt,next_node_proc);
        }
    }
    return;
}

void 
end_upf_procedure(proc_context_t *proc_ctxt)
{
    upf_context_t *upf_context = (upf_context_t*)proc_ctxt->upf_context;

    LOG_MSG(LOG_DEBUG,"End procedure %s ", proc_ctxt->proc_name);
    if(proc_ctxt->pfcp_trans != NULL) {
        cleanup_pfcp_trans(proc_ctxt->pfcp_trans);
    }

    msg_info_t *msg = (msg_info_t *)proc_ctxt->msg_info;
    if(msg != NULL) {
        msg->refCnt--;
        if(msg->refCnt == 1) { // i m the only one using this 
            free(msg->raw_buf);
            free(msg); 
        }
    }

    if(proc_ctxt->result == PROC_RESULT_FAILURE) {
        //upf_context_delete_entry(upf_context->upf_sockaddr.sin_addr.s_addr);
        //free(upf_context);
        // Dont delete upf context 
        upf_context->proc = NULL;
    } else {
        upf_context->proc = NULL;
    }

    free(proc_ctxt);
}

/* Requirement: 
 * For now I am using linux system call to do the service name dns resolution...
 * 3gpp based DNS lookup of NRF support would be required to locate UPF. 
 */
static struct in_addr 
native_linux_name_resolve(const char *name)
{
    struct in_addr ip = {0};
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
                freeaddrinfo(result);
                return addrV4->sin_addr;
            }
        }
    }
    LOG_MSG(LOG_ERROR, "DNS Query for %s failed with error %s", name, gai_strerror(err));
    return ip;
}

void*
dns_handler(void *data)
{
    struct threadData *inp = (struct threadData *)data;
    int n;

    LOG_MSG(LOG_INFO,"Name resolution thread fd %d", inp->fd);
    while(1) {
        msgDataEnv_t control = {0};
        //printf("waiting for message read\n");
        n = read(inp->fd, &control, sizeof(msgDataEnv_t));
        dnsMsg_t *rsp = control.data;

        LOG_MSG(LOG_DEBUG, "DNS Request received for %s", rsp->name);
        rsp->ip_addr = native_linux_name_resolve(rsp->name);
        pthread_mutex_lock(&rsp->msg_mutex);
        if(rsp->req_valid == false) {
            LOG_MSG(LOG_ERROR,"DNS Request is not valid. Client timed out. Don't send response %s",rsp->name);
            pthread_mutex_unlock(&rsp->msg_mutex);
            free(rsp);
            continue; // client is not waiting for response so dont write response
        }
        // req is still valid so we need to write response
        n = write(inp->fd, rsp, sizeof(dnsMsg_t));
        LOG_MSG(LOG_DEBUG,"Sending DNS Response to query thread bytes write %d for %s", n, rsp->name);
        rsp->rsp_sent = true;
        pthread_mutex_unlock(&rsp->msg_mutex);
    }
}

static struct in_addr 
upf_name_resolve(const char *name)
{
    struct in_addr upf_addr = {0};
    static bool thread_started = false;
    static int fd[2];
    if (thread_started == false) {
      if(socketpair(AF_LOCAL, SOCK_DGRAM, 0, fd) == -1) {
          assert(0);
      }
      struct threadData *td;
      td = (struct threadData *) calloc(1, sizeof(struct threadData));
      td->fd = fd[1];
      pthread_t readerLocal_t;
      pthread_attr_t localattr;
      pthread_attr_init(&localattr);
      pthread_attr_setdetachstate(&localattr, PTHREAD_CREATE_DETACHED);
      pthread_create(&readerLocal_t, &localattr, &dns_handler, (void*)td);
      pthread_attr_destroy(&localattr);
      thread_started = true;
    }
    LOG_MSG(LOG_DEBUG, "Resolve UPF name %s", name);

    dnsMsg_t *req = (dnsMsg_t *) calloc(1, sizeof(dnsMsg_t));
    strncpy(&req->name[0], name, strlen(name)+1);
    req->ip_addr.s_addr = 0;
    req->req_valid = true;
    req->rsp_sent = false;

    msgDataEnv_t msg = {0};
    msg.data = req;
    int n = write(fd[0], &msg, sizeof(msgDataEnv_t));

    if(n < 0) {
        LOG_MSG(LOG_ERROR,"Failed to write request message to peer socket = %d, UPF %s ", n, name);
        free(req);
        return upf_addr;
    }

    // Wait for response
    do {
        struct pollfd pfds[1];
        pfds[0].fd = fd[0];
        pfds[0].events = POLLIN;
        uint32_t timeout = cp_config->upfdnstimeout ? cp_config->upfdnstimeout:100;
        int event_count = poll(pfds, 1, timeout); // Wait for max 100ms
        //printf("event_count = %d \n", event_count);
        pthread_mutex_lock(&req->msg_mutex);
        if(event_count <=0) {
            LOG_MSG(LOG_ERROR,"DNS Request timeout for %s", name);
            // dont' free request
            if(req->rsp_sent == false) {
                req->req_valid = false;
                // req will be freed by dns thread
                LOG_MSG(LOG_ERROR, "timeout socket event. Mark req invalid %s", name);
                pthread_mutex_unlock(&req->msg_mutex);
                return upf_addr;
            } else {
                // this can happen if peer send response and same time poll timeout happened.
                LOG_MSG(LOG_ERROR,"Go ahead read message from socket for UPF %s ", name);
                break;
            }
        }
    }while(0);

    dnsMsg_t rsp = {0};
    n = read(fd[0], &rsp, sizeof(dnsMsg_t));
    LOG_MSG(LOG_INFO,"Read DNS response %s -> %s", rsp.name, inet_ntoa(rsp.ip_addr));
    pthread_mutex_unlock(&req->msg_mutex);
    upf_addr = rsp.ip_addr;
    free(req);
    return upf_addr;
}

upf_context_t*
get_upf_context(const char *user_plane_service, bool global_address)
{
    struct in_addr upf_addr = {0};

    // Search UPF with name context
    upf_context_t *upf_context = NULL;
    upf_context = (upf_context_t*)upf_context_entry_lookup_service(user_plane_service);

    // if UPF not found, create context
    if(upf_context == NULL) {
        upf_addr = upf_name_resolve(user_plane_service);
        // found name to address mapping. Create context and update details.
        LOG_MSG(LOG_DEBUG, "UPF context for upf [%s] - address resolved to %s ", user_plane_service, inet_ntoa(upf_addr));
        create_upf_context(upf_addr.s_addr, &upf_context, user_plane_service);
        if (upf_context != NULL) {
            strcpy(upf_context->fqdn, user_plane_service);
            upf_context->global_address = global_address;
        }
    } else if (upf_context->upf_sockaddr.sin_addr.s_addr == 0) {
        upf_context->global_address = global_address;
        upf_addr = upf_name_resolve(user_plane_service);
        upf_context->upf_sockaddr.sin_addr.s_addr = upf_addr.s_addr;
        if(upf_addr.s_addr != 0) {
	        int ret = upf_context_entry_add(&upf_addr.s_addr, upf_context);
    	    if (ret) {
		        LOG_MSG(LOG_ERROR, "Failed to add UPF addresss [%s] in UPF context map.Error: %d ", inet_ntoa(*((struct in_addr *)&upf_addr.s_addr)), ret);
		        return NULL;
	        }
        }
    }
    return upf_context;
}

void 
handleUpfAssociationTimeoutEvent(void *data, uint16_t event)
{
    upf_context_t *upf = (upf_context_t *)data;
    if(upf == NULL) {
        //LOG_MSG(LOG_DEBUG,"Initiate association with all UPFs");
        initiate_all_pfcp_association();
    } else {
        // default schedule timeout of 10 seconds
        LOG_MSG(LOG_DEBUG,"Initiate association with specific UPF [%s]", upf->fqdn);
        initiate_pfcp_association(upf);
    }
    return;
}

void 
upfAssociationTimerCallback( gstimerinfo_t *ti, const void *data_t )
{
    upf_context_t *upf = (upf_context_t*)data_t;
    stopTimer(ti);
    deinitTimer(ti);
    LOG_MSG(LOG_INFO,"Upf Association Timeout event for UPF [%s] %p", (upf != NULL) ? upf->fqdn:"all UPFs", ti);
    queue_stack_unwind_event(UPF_ASSOCIATION_SETUP, (void *)data_t, handleUpfAssociationTimeoutEvent); 
    return;
}
