// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef SM_ENUM_H
#define SM_ENUM_H

#include <stdio.h>
#include <stdint.h>
typedef enum 
{
	INTF_NONE,
	PGW_S5_S8,
    SGW_S11_S4,
    SGW_S5_S8,
    PGW_GX,
    PGW_SXB,
    SGW_SXA,
}PKT_RX_INTERFACE;

typedef enum 
{
	NONE_PROC,
	INITIAL_PDN_ATTACH_PROC,
	SERVICE_REQUEST_PROC,
	SGW_RELOCATION_PROC,
	CONN_SUSPEND_PROC,
	DETACH_PROC,
	DED_BER_ACTIVATION_PROC,
	PDN_GW_INIT_BEARER_DEACTIVATION,
	MME_INI_DEDICATED_BEARER_DEACTIVATION_PROC,
	UPDATE_BEARER_PROC,
	RESTORATION_RECOVERY_PROC,
	END_PROC
}sm_proc;


/* VS: Defined different states of the STATE Machine */
typedef enum 
{
	SGWC_NONE_STATE,
	PFCP_ASSOC_REQ_SNT_STATE,
	PFCP_ASSOC_RESP_RCVD_STATE,
	PFCP_SESS_EST_REQ_SNT_STATE,
	PFCP_SESS_EST_RESP_RCVD_STATE,
	CONNECTED_STATE = 5,
	IDEL_STATE,
	CS_REQ_SNT_STATE,
	CS_RESP_RCVD_STATE,
	PFCP_SESS_MOD_REQ_SNT_STATE=9,
	PFCP_SESS_MOD_RESP_RCVD_STATE,
	PFCP_SESS_DEL_REQ_SNT_STATE=11,
	PFCP_SESS_DEL_RESP_RCVD_STATE,
	DS_REQ_SNT_STATE=13,
	DS_RESP_RCVD_STATE,
	DDN_REQ_SNT_STATE,
	DDN_ACK_RCVD_STATE = 16,
	MBR_REQ_SNT_STATE,
	MBR_RESP_RCVD_STATE,
	CREATE_BER_REQ_SNT_STATE=19,
	RE_AUTH_ANS_SNT_STATE=20,
	PGWC_NONE_STATE=21,
	CCR_SNT_STATE,
	CREATE_BER_RESP_SNT_STATE,
	PFCP_PFD_MGMT_RESP_RCVD_STATE=24,
	ERROR_OCCURED_STATE,
	UPDATE_BEARER_REQ_SNT_STATE=26,
	UPDATE_BEARER_RESP_SNT_STATE,
	DELETE_BER_REQ_SNT_STATE=28,
	CCRU_SNT_STATE,
	PGW_RSTRT_NOTIF_REQ_SNT_STATE=30,
	UPD_PDN_CONN_SET_REQ_SNT_STATE,
	DEL_PDN_CONN_SET_REQ_SNT_STATE,
	DEL_PDN_CONN_SET_REQ_RCVD_STATE=33,
	PFCP_SESS_SET_DEL_REQ_SNT_STATE,
	PFCP_SESS_SET_DEL_REQ_RCVD_STATE,
	END_STATE
}sm_state;

/* VS: Register different types of events */
typedef enum 
{
	NONE_EVNT,
	CS_REQ_RCVD_EVNT = 1,
	PFCP_ASSOC_SETUP_SNT_EVNT,
	PFCP_ASSOC_SETUP_RESP_RCVD_EVNT=3,
	PFCP_SESS_EST_REQ_RCVD_EVNT,
	PFCP_SESS_EST_RESP_RCVD_EVNT=5,
	CS_RESP_RCVD_EVNT,
	MB_REQ_RCVD_EVNT = 7,
	PFCP_SESS_MOD_REQ_RCVD_EVNT=8,
	PFCP_SESS_MOD_RESP_RCVD_EVNT=9,
	MB_RESP_RCVD_EVNT,
	REL_ACC_BER_REQ_RCVD_EVNT,
	DS_REQ_RCVD_EVNT = 12 ,
	PFCP_SESS_DEL_REQ_RCVD_EVNT,
	PFCP_SESS_DEL_RESP_RCVD_EVNT = 14,
	DS_RESP_RCVD_EVNT=15,
	ECHO_REQ_RCVD_EVNT,
	ECHO_RESP_RCVD_EVNT,
	DDN_ACK_RESP_RCVD_EVNT,
	PFCP_SESS_RPT_REQ_RCVD_EVNT,
	RE_AUTH_REQ_RCVD_EVNT=20,
	CREATE_BER_RESP_RCVD_EVNT,
	CCA_RCVD_EVNT = 22,
	CREATE_BER_REQ_RCVD_EVNT,
	PFCP_PFD_MGMT_RESP_RCVD_EVNT=24,
	ERROR_OCCURED_EVNT,
	UPDATE_BEARER_REQ_RCVD_EVNT,
	UPDATE_BEARER_RSP_RCVD_EVNT = 27,
	DELETE_BER_REQ_RCVD_EVNT,
	DELETE_BER_RESP_RCVD_EVNT = 29,
	DELETE_BER_CMD_RCVD_EVNT,
	CCAU_RCVD_EVNT,
	PGW_RSTRT_NOTIF_ACK_RCVD_EVNT,
	UPD_PDN_CONN_SET_REQ_RCVD_EVNT = 33,
	UPD_PDN_CONN_SET_RESP_RCVD_EVNT,
	DEL_PDN_CONN_SET_REQ_RCVD_EVNT,
	DEL_PDN_CONN_SET_RESP_RCVD_EVNT = 36,
	PFCP_SESS_SET_DEL_REQ_RCVD_EVNT,
	PFCP_SESS_SET_DEL_RESP_RCVD_EVNT,
	END_EVNT
}sm_event;

/*
typedef enum {
	NONE_EVNT,
	CS_REQ_RCVD_EVNT,
	CS_RESP_RCVD_EVNT,
	MB_REQ_RCVD_EVNT,
	MB_RESP_RCVD_EVNT,
	REL_ACC_BER_REQ_RCVD_EVNT,
	DS_REQ_RCVD_EVNT,
	DS_RESP_RCVD_EVNT,
	ECHO_REQ_RCVD_EVNT,
	ECHO_RESP_RCVD_EVNT,
	DDN_ACK_RESP_RCVD_EVNT,
	CREATE_BER_RESP_RCVD_EVNT,
	CREATE_BER_REQ_RCVD_EVNT,
	ERROR_OCCURED_EVNT,
	UPDATE_BEARER_REQ_RCVD_EVNT,
	UPDATE_BEARER_RSP_RCVD_EVNT,
	DELETE_BER_REQ_RCVD_EVNT,
	DELETE_BER_RESP_RCVD_EVNT,
	DELETE_BER_CMD_RCVD_EVNT,
	CCAU_RCVD_EVNT,
	PGW_RSTRT_NOTIF_ACK_RCVD_EVNT,
	UPD_PDN_CONN_SET_REQ_RCVD_EVNT,
	UPD_PDN_CONN_SET_RESP_RCVD_EVNT,
	DEL_PDN_CONN_SET_REQ_RCVD_EVNT,
	DEL_PDN_CONN_SET_RESP_RCVD_EVNT,
	END_EVNT
}sm_event;
*/



#endif
