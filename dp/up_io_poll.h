// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _UP_IO_POLL_H_
#define _UP_IO_POLL_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of Interface message parsing.
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "up_interface.h"
#include "up_main.h"
/**
 * @brief udp socket structure.
 */
typedef struct udp_sock_t {
	struct sockaddr_in my_addr;
	struct sockaddr_in other_addr;
	int sock_fd_pfcp;
} udp_sock_t;

/**
 * @brief API to create udp socket.
 */
int
create_udp_socket(struct in_addr recv_ip, uint16_t recv_port,
		udp_sock_t *sock);


/* message types */
enum dp_msg_type {
	/* Session Bearer Map Hash Table*/
	MSG_SESS_TBL_CRE,
	MSG_SESS_TBL_DES,
	MSG_SESS_CRE,
	MSG_SESS_MOD,
	MSG_SESS_DEL,
	/* ADC Rule Table*/
	MSG_ADC_TBL_CRE,
	MSG_ADC_TBL_DES,
	MSG_ADC_TBL_ADD,
	MSG_ADC_TBL_DEL,
	/* PCC Rule Table*/
	MSG_PCC_TBL_CRE,
	MSG_PCC_TBL_DES,
	MSG_PCC_TBL_ADD,
	MSG_PCC_TBL_DEL,
	/* Meter Tables*/
	MSG_MTR_CRE,
	MSG_MTR_DES,
	MSG_MTR_ADD,
	MSG_MTR_DEL,
	MSG_MTR_CFG,
	/* Filter Table for SDF & ADC*/
	MSG_SDF_CRE,
	MSG_SDF_DES,
	MSG_SDF_ADD,
	MSG_SDF_DEL,
	MSG_EXP_CDR,
	/* DDN from DP to CP*/
	MSG_DDN,
	MSG_DDN_ACK,

	MSG_END,
};

/* Table Callback msg payload */
struct cb_args_table {
	char name[MAX_LEN];	/* table name */
	uint32_t max_elements;	/* rule id */
};

/*
 * Message Structure
 */
struct msgbuf {
	long mtype;
	struct dp_id dp_id;
	union __attribute__ ((packed)) {
		struct pkt_filter pkt_filter_entry;
		struct adc_rules adc_filter_entry;
		struct pcc_rules pcc_entry;
		struct session_info sess_entry;
		struct mtr_entry mtr_entry;
		struct cb_args_table msg_table;
		struct msg_ue_cdr ue_cdr;

		struct downlink_data_notification_ack_t dl_ddn; /** Downlink data notification info */
	} msg_union;
};
struct msgbuf sbuf;
struct msgbuf rbuf;

uint8_t pfcp_rx[1024]; /* TODO: Decide size */

/* IPC msg node */
struct ipc_node {
	int msg_id;	/*msg type*/
	int (*msg_cb)(struct msgbuf *msg_payload);	/*callback function*/
};
struct ipc_node *basenode;

/**
 * @brief Function to recv the IPC message and process them.
 *
 * This function is not thread safe and should only be called once by DP.
 */
void iface_process_ipc_msgs(void);

/**
 * @brief Function to Inilialize memory for IPC msg.
 *
 * @param
 *	void
 */
void iface_init_ipc_node(void);
/**
 * @brief Functino to register call back apis with msg id..
 *
 * @param msg_id
 *	msg_id - id number on which the call back function should
 *	invoked.
 * @param msg_payload
 *	msg_payload - payload of the message
 *
 * This function is thread safe due to message queue implementation.
 */
void
iface_ipc_register_msg_cb(int msg_id,
		int (*msg_cb)(struct msgbuf *msg_payload));


//#ifdef DP_BUILD
int
udp_recv(void *msg_payload, uint32_t size,
			struct sockaddr_in *peer_addr);

/**
 * @brief Functino to Process IPC msgs.
 *
 * @param none
 * Return
 * 0 on success, -1 on failure
 */

int simu_cp(void);
#endif /* _UP_IO_POLL_H_ */

