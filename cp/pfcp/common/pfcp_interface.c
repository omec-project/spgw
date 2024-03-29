// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include "pfcp_cp_interface.h"
#include "pfcp_messages_decoder.h"
#include "spgw_config_struct.h"
#include "cp_events.h"
#include "cp_test.h"
#include "cp_log.h"
#include "cp_io_poll.h"


pfcp_handler pfcp_msg_handler[256];


void init_pfcp_msg_handlers(void)
{
    for(int i=0;i<256;i++)
        pfcp_msg_handler[i] = handle_unknown_pfcp_msg;

    pfcp_msg_handler[PFCP_HEARTBEAT_REQUEST] =  handle_pfcp_heartbit_req_msg;
    pfcp_msg_handler[PFCP_HEARTBEAT_RESPONSE] =  handle_pfcp_heartbit_rsp_msg;
    pfcp_msg_handler[PFCP_ASSOCIATION_SETUP_REQUEST] =  handle_pfcp_association_setup_request_msg;
    pfcp_msg_handler[PFCP_ASSOCIATION_SETUP_RESPONSE] =  handle_pfcp_association_setup_response_msg;
    pfcp_msg_handler[PFCP_SESSION_ESTABLISHMENT_RESPONSE] =  handle_pfcp_session_est_response_msg;
    pfcp_msg_handler[PFCP_SESSION_MODIFICATION_RESPONSE] =  handle_pfcp_session_mod_response_msg;
    pfcp_msg_handler[PFCP_SESSION_DELETION_RESPONSE] = handle_pfcp_session_delete_response_msg; 
    pfcp_msg_handler[PFCP_SESSION_REPORT_REQUEST] = handle_session_report_msg;
    pfcp_msg_handler[PFCP_PFD_MANAGEMENT_RESPONSE] = handle_pfcp_pfd_management_response_msg;
    return;
}

int 
handle_unknown_pfcp_msg(msg_info_t **msg_p, pfcp_header_t *pfcp_rx)
{
    msg_info_t *msg = *msg_p;
    LOG_MSG(LOG_ERROR, "process_msgs- case: spgw_cfg= %d;"
            "\tReceived unprocessed PFCP Message_Type:%u"
            "... Discarding msg %p ", cp_config->cp_type, pfcp_rx->message_type, msg);

    return -1;
}

void
process_pfcp_msg(void *data, uint16_t event)
{
    assert(event == PFCP_MSG_RECEIVED );
    msg_info_t *msg = (msg_info_t *)data;    
    assert(msg->magic_head == MSG_MAGIC);
    assert(msg->magic_tail == MSG_MAGIC);
    pfcp_header_t *pfcp_header = (pfcp_header_t *)msg->raw_buf;
    pfcp_msg_handler[pfcp_header->message_type](&msg, pfcp_header);
    if(msg != NULL) {
        assert(msg->magic_head == MSG_MAGIC);
        assert(msg->magic_tail == MSG_MAGIC);
        free(msg->raw_buf);
        if(msg->refCnt == 0) // no one claimed ownership of this msg 
            free(msg);
    }
    return;
}

int
pfcp_send(int fd, void *msg_payload, uint32_t size,
		struct sockaddr_in *peer_addr)
{
    queue_pfcp_out_event(fd, (uint8_t*)msg_payload, size, (struct sockaddr *)peer_addr);
	return 1;
}

/* PERFORAMANCE : Should use conditional variable ?*/
void*
out_handler_pfcp(void *data)
{
	LOG_MSG(LOG_INIT,"Starting pfcp out message handler thread");
    while(1) {
        outgoing_pkts_event_t *event = (outgoing_pkts_event_t *)get_pfcp_out_event();
        if(event != NULL) {
            //Push packet to test chain 
            pfcp_header_t *temp = (pfcp_header_t *)event->payload;
            if(pfcp_out_mock_handler[temp->message_type] != NULL) {
                pfcp_out_mock_handler[temp->message_type](event);
                free(event->payload);
                free(event);
                continue;
            }
            
            int bytes_tx = sendto(event->fd, event->payload, event->payload_len, 0,
                    (struct sockaddr *) &event->dest_addr, sizeof(struct sockaddr_in));
            if(bytes_tx <= 0) {
                struct sockaddr_in *p = (struct sockaddr_in*)&event->dest_addr;
                LOG_MSG(LOG_ERROR, "pfcp_send() on fd = %d bytes_tx = %d , error = %s payload len = %d peer %s", 
                                    event->fd, bytes_tx, strerror(errno), event->payload_len, inet_ntoa(p->sin_addr));
            }
            free(event->payload);
            free(event);
            continue;
        }
        //PERFORAMANCE ISSUE - use conditional variable 
        usleep(100); // every pkt 0.1 ms default scheduling delay 
    }
	LOG_MSG(LOG_INIT,"Exiting : pfcp out message handler thread data = %p ", data);
    return NULL;
}

void init_pfcp_msg_threads(void)
{
    // thread to read incoming socket messages from udp socket
    pthread_t readerPfcp_t;
    pthread_attr_t pfcpattr;
    pthread_attr_init(&pfcpattr);
    pthread_attr_setdetachstate(&pfcpattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&readerPfcp_t, &pfcpattr, &msg_handler_pfcp, NULL);
    pthread_attr_destroy(&pfcpattr);

    // thread to write outgoing pfcp messages
    pthread_t writerPfcp_t;
    pthread_attr_t pfcpattr1;
    pthread_attr_init(&pfcpattr1);
    pthread_attr_setdetachstate(&pfcpattr1, PTHREAD_CREATE_DETACHED);
    pthread_create(&writerPfcp_t, &pfcpattr1, &out_handler_pfcp, NULL);
    pthread_attr_destroy(&pfcpattr1);
}

void init_pfcp(void)
{
	int ret;
    int pfcp_fd = -1;
    struct sockaddr_in pfcp_sockaddr;

    init_pfcp_msg_handlers();

	pfcp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_pfcp = pfcp_fd;

    assert(pfcp_fd > 0);

	bzero(pfcp_sockaddr.sin_zero,
			sizeof(pfcp_sockaddr.sin_zero));
	pfcp_sockaddr.sin_family = AF_INET;
	pfcp_sockaddr.sin_port = htons(cp_config->pfcp_port);
	pfcp_sockaddr.sin_addr = cp_config->pfcp_ip;

	int flag = 1;
	if (-1 == setsockopt(pfcp_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
		LOG_MSG(LOG_ERROR,"setsockopt fail");
	}

	ret = bind(pfcp_fd, (struct sockaddr *) &pfcp_sockaddr,
			sizeof(struct sockaddr_in));

    assert(ret >= 0);

	LOG_MSG(LOG_INIT,  "init_pfcp() pfcp_fd = %d :: pfcp_ip = %s : pfcp_port = %d",
			pfcp_fd, inet_ntoa(cp_config->pfcp_ip),
			cp_config->pfcp_port);

    my_sock.pfcp_sockaddr = pfcp_sockaddr; 

	init_pfcp_msg_threads();
}

