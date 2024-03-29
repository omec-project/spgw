// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef _CP_IO_POLL_H_
#define _CP_IO_POLL_H_ 
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of Interface message parsing.
 */
#include "cp_interface.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief udp socket structure.
 */
typedef struct udp_sock_t {
    struct sockaddr_in s5s8_recv_sockaddr;
    struct sockaddr_in pfcp_sockaddr;
    struct sockaddr_in s11_sockaddr;
    struct sockaddr_in loopback_sockaddr;
    int sock_fd_local;
	int sock_fd_pfcp;
	int sock_fd_s11;
	int sock_fd_s5s8;
    int gx_app_sock;
    int select_max_fd;
} udp_sock_t;
extern udp_sock_t my_sock;

/* message types */
enum dp_msg_type {
	/* Session Bearer Map Hash Table*/
	MSG_SESS_TBL_CRE,
	MSG_SESS_TBL_DES,
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
struct msgbuf1 {
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
	} msg_union;
};


/**
 * @brief Function to recv the IPC message and process them.
 *
 * This function is not thread safe and should only be called once by DP.
 */
void iface_process_events(void);
void *msg_handler_local(void*);
void process_local_msg(void *data, uint16_t event);
void *incoming_event_handler(void*);

#ifdef __cplusplus
}
#endif
#endif /* _CP_IO_POLL_H_ */
