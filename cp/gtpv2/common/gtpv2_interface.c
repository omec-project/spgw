// Copyright (c) 2017 Intel Corporation
// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "gtpv2_interface.h"
#include "gtp_messages_decoder.h"
#include "cp_interface.h"
#include "spgw_config_struct.h"
#include "gtp_ies.h"
#include "util.h"
#include "cp_events.h"
#include "gtpv2_error_rsp.h"
#include "cp_test.h"
#include "cp_log.h"
#include "cp_io_poll.h"

gtp_handler gtp_msg_handler[256];
extern pcap_t *pcap_reader;
struct sockaddr_in s5s8_sockaddr;

uint32_t get_gtp_sequence(void)
{
    static uint32_t sequence_num=1;
    return sequence_num++;
}


void init_gtp_msg_handlers(void)
{
	LOG_MSG(LOG_INIT,"Init gtp interface");
    for(int i=0;i<256;i++)
        gtp_msg_handler[i] = handle_unknown_msg;

    gtp_msg_handler[GTP_ECHO_REQ] =  handle_echo_request;
    gtp_msg_handler[GTP_ECHO_RSP] =  handle_echo_response;
    gtp_msg_handler[GTP_CREATE_SESSION_REQ] =  handle_create_session_request;
    gtp_msg_handler[GTP_MODIFY_BEARER_REQ] = handle_modify_bearer_request; 
    gtp_msg_handler[GTP_DELETE_SESSION_REQ] = handle_delete_session_request;
    gtp_msg_handler[GTP_RELEASE_ACCESS_BEARERS_REQ] = handle_rab_request;
    gtp_msg_handler[GTP_DOWNLINK_DATA_NOTIFICATION_ACK] = handle_ddn_ack; 
    gtp_msg_handler[GTP_CREATE_BEARER_RSP] = handle_create_bearer_response_msg;
    gtp_msg_handler[GTP_DELETE_BEARER_RSP] =  handle_delete_bearer_response_msg;
#ifdef FUTURE_NEED
    gtp_msg_handler[GTP_UPDATE_BEARER_RSP] = handle_update_bearer_response_msg;
	gtp_msg_handler[GTP_DELETE_BEARER_CMD] = handle_delete_bearer_cmd_msg;
#endif
}

int 
handle_unknown_msg(msg_info_t **msg_p, gtpv2c_header_t *rx_msg)
{
    msg_info_t *msg = *msg_p;
	LOG_MSG(LOG_ERROR, "Unhandled GTP message = %d, msg = %p ", rx_msg->gtpc.message_type, msg);
    return -1;
}

/**
 * @brief  : Initalizes S11 interface if in use
 * @param  : void
 * @return : void
 */
static void init_s11(void)
{
    int s11_fd = -1;
	int ret;
    struct sockaddr_in s11_sockaddr;

	if (pcap_reader != NULL && pcap_dumper != NULL)
		return;

	s11_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_s11 = s11_fd;

    assert(s11_fd > 0);

	bzero(s11_sockaddr.sin_zero,
			sizeof(s11_sockaddr.sin_zero));
	s11_sockaddr.sin_family = AF_INET;
	s11_sockaddr.sin_port = htons(cp_config->s11_port);
	s11_sockaddr.sin_addr = cp_config->s11_ip;

	int flag = 1;
	if (-1 == setsockopt(s11_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
		LOG_MSG(LOG_ERROR,"setsockopt fail");
	}

	ret = bind(s11_fd, (struct sockaddr *) &s11_sockaddr,
			    sizeof(struct sockaddr_in));

    assert(ret >= 0);
    my_sock.s11_sockaddr = s11_sockaddr;
	LOG_MSG(LOG_INIT, "init_s11() s11_fd= %d :: s11_ip= %s : s11_port= %d",
			s11_fd, inet_ntoa(cp_config->s11_ip), cp_config->s11_port);
}

/**
 * @brief  : Initalizes s5s8_sgwc interface if in use
 * @param  : void
 * @return : void
 */
static void init_s5s8(void)
{
    int s5s8_fd = -1;
	int ret;
    struct sockaddr_in s5s8_recv_sockaddr;

	/* TODO: Need to think*/
	s5s8_recv_sockaddr.sin_port = htons(cp_config->s5s8_port);

	if (pcap_reader != NULL && pcap_dumper != NULL)
		return;

	s5s8_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_s5s8 = s5s8_fd;

    assert(s5s8_fd > 0);

	bzero(s5s8_sockaddr.sin_zero,
			sizeof(s5s8_sockaddr.sin_zero));
	s5s8_sockaddr.sin_family = AF_INET;
	s5s8_sockaddr.sin_port = htons(cp_config->s5s8_port);
	s5s8_sockaddr.sin_addr = cp_config->s5s8_ip;

	ret = bind(s5s8_fd, (struct sockaddr *) &s5s8_sockaddr,
			    sizeof(struct sockaddr_in));

    assert(ret >= 0);

    my_sock.s5s8_recv_sockaddr = s5s8_recv_sockaddr;

	LOG_MSG(LOG_INIT, "init_s5s8_sgwc() s5s8_fd= %d :: s5s8_ip= %s : s5s8_port= %d",
			s5s8_fd, inet_ntoa(cp_config->s5s8_ip),
			htons(cp_config->s5s8_port));

}

void 
init_gtp_msg_threads(void)
{
    // thread to read incoming socket messages from udp socket
    pthread_t readerGtp_t;
    pthread_attr_t gtpattr;
    pthread_attr_init(&gtpattr);
    pthread_attr_setdetachstate(&gtpattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&readerGtp_t, &gtpattr, &msg_handler_gtp, NULL);
    pthread_attr_destroy(&gtpattr);

    // thread to write outgoing gtp messages
    pthread_t writerGtp_t;
    pthread_attr_t gtpattr1;
    pthread_attr_init(&gtpattr1);
    pthread_attr_setdetachstate(&gtpattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&writerGtp_t, &gtpattr1, &out_handler_gtp, NULL);
    pthread_attr_destroy(&gtpattr1);
	return;
}

void
process_gtp_msg(void *data, uint16_t event)
{
    assert(event == GTP_MSG_RECEIVED );
    msg_info_t *msg = (msg_info_t *)data;    
    gtpv2c_header_t *gtpv2c_rx = (gtpv2c_header_t *)msg->raw_buf;
    if (gtpv2c_rx->gtpc.version < GTP_VERSION_GTPV2C) {
        LOG_MSG(LOG_ERROR, "Discarding packet due to gtp version is not supported..");
        free(msg->raw_buf);
        free(msg);
        return;
    }else if (gtpv2c_rx->gtpc.version > GTP_VERSION_GTPV2C) {
        send_version_not_supported(&msg->peer_addr, 
                cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE,
                gtpv2c_rx->teid.has_teid.seq);
        LOG_MSG(LOG_ERROR, "Discarding packet due to gtp version is not supported..");
        // TODO : memory leak ?
        return;
    }
    gtp_msg_handler[gtpv2c_rx->gtpc.message_type](&msg, gtpv2c_rx);
    if(msg != NULL) {
        assert(msg->magic_head == MSG_MAGIC);
        assert(msg->magic_tail == MSG_MAGIC);
        if(msg->refCnt == 0) { // no one claimed ownership of this msg 
            free(msg->raw_buf);
            free(msg);
        }
    }

}

/**
 * @brief  : Util to send or dump gtpv2c messages
 * @param  : gtpv2c_if_fd, interface indentifier
 * @param  : gtpv2c_tx_buf, buffer to store data for peer node
 * @param  : gtpv2c_pyld_len, data length
 * @param  : dest_addr, destination address
 * @param  : dest_addr_len, destination address length
 * @return : Void
 */
void
gtpv2c_send(int gtpv2c_if_fd, uint8_t *gtpv2c_tx_buf,
		uint16_t gtpv2c_pyld_len, struct sockaddr *dest_addr,
		socklen_t dest_addr_len)
{
#ifdef PCAP_DUMP
	int bytes_tx;
	if (pcap_dumper) {
		dump_pcap(gtpv2c_pyld_len, gtpv2c_tx_buf);
	} 
#endif
    queue_gtp_out_event(gtpv2c_if_fd, gtpv2c_tx_buf, gtpv2c_pyld_len, dest_addr);
}

/* Should use conditional variable ?*/
void*
out_handler_gtp(void *data)
{
	LOG_MSG(LOG_INIT,"Starting gtp out message handler thread");
    while(1) {
        outgoing_pkts_event_t *event = (outgoing_pkts_event_t*)get_gtp_out_event();
        if(event != NULL) {
            //Push packet to test chain 
            gtpv2c_header_t *temp = (gtpv2c_header_t *)event->payload;
            if(gtp_out_mock_handler[temp->gtpc.message_type] != NULL) {
                gtp_out_mock_handler[temp->gtpc.message_type](event);
                free(event->payload);
                free(event);
                continue;
            }

            int bytes_tx = sendto(event->fd, event->payload, event->payload_len, 0,
                    (struct sockaddr *) &event->dest_addr, sizeof(struct sockaddr_in));
			if(bytes_tx < 0) {
            	LOG_MSG(LOG_ERROR, "gtpv2c_send() failed on fd= %d", event->fd);
			}
            free(event->payload);
            free(event);
            continue;
        }
        usleep(100); // every pkt 0.1 ms default scheduling delay 
    }
	LOG_MSG(LOG_ERROR,"Exiting gtp out message handler thread %p", data);
    return NULL;
}

void init_gtp(void)
{
    init_gtp_msg_handlers();

	switch (cp_config->cp_type) {
	case PGWC:
		init_s5s8();
		break;
	case SAEGWC:
		init_s11();
		break;
	default:
        assert(0); 
		break;
	}
	init_gtp_msg_threads();
}

