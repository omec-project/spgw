// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _PFCP_ERROR_RSP_H_
#define _PFCP_ERROR_RSP_H_

#include "ue.h"
#include "sm_struct.h"


typedef struct pfcp_err_rsp_info
{
	uint64_t dp_seid;
    uint32_t seq;
    uint8_t offending;
    struct sockaddr_in peer_addr; 
}pfcp_err_rsp_info_t;

void session_report_error_response(msg_info_t *msg, uint8_t cause_value, int iface);
void get_error_session_report_info(msg_info_t *msg, pfcp_err_rsp_info_t *err_rsp_info);
#endif
