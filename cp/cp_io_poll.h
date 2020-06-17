// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _CP_IO_POLL_H_
#define _CP_IO_POLL_H_ 
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of Interface message parsing.
 */
#include "cp_interface.h"
#include "cp_main.h"
#include "gtpv2c_msg_struct.h"

/**
 * @brief udp socket structure.
 */
typedef struct udp_sock_t {
	struct sockaddr_in my_addr;
	struct sockaddr_in other_addr;
	int sock_fd_pfcp;
	int sock_fd_s11;
	int sock_fd_s5s8;
    int select_max_fd;
} udp_sock_t;

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

		struct downlink_data_notification dl_ddn;	/** Downlink data notification info */
	} msg_union;
};
struct msgbuf sbuf;
struct msgbuf rbuf;


/**
 * @brief Function to recv the IPC message and process them.
 *
 * This function is not thread safe and should only be called once by DP.
 */
void iface_process_ipc_msgs(void);


/**
 * @brief Functino to init rte hash tables.
 *
 * @param none
 * Return
 *  None
 */

int
simu_cp(__rte_unused void *ptr);

/**
 * @brief callback to handle downlink data notification messages from the
 * data plane
 * @param msg_payload
 * message payload received by control plane from the data plane
 * @return
 * 0 inicates success, error otherwise
 */
int
cb_ddn(struct msgbuf *msg_payload);


#endif /* _CP_IO_POLL_H_ */

