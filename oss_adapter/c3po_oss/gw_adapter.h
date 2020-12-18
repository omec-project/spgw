// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdbool.h>
#ifndef GW_ADAPTER_H
#define GW_ADAPTER_H

typedef long long int _timer_t;

#define TIMER_GET_CURRENT_TP(now)                                             \
({                                                                            \
 struct timespec ts;                                                          \
 now = clock_gettime(CLOCK_REALTIME,&ts) ?                                    \
 	-1 : (((_timer_t)ts.tv_sec) * 1000000000) + ((_timer_t)ts.tv_nsec);   \
 now;                                                                         \
 })

#define TIMER_GET_ELAPSED_NS(start)                                           \
({                                                                            \
 _timer_t ns;                                                                 \
 TIMER_GET_CURRENT_TP(ns);                                                    \
 if (ns != -1){                                                               \
 	ns -= start;                                                          \
 }									      \
 ns;                                                                          \
 })

extern _timer_t st_time;


/////////////////////////////////////////////////////////////////////////////////////////

#define MAX_PEER 10
#define S11_MSG_TYPE_LEN 49
#define S5S8_MSG_TYPE_LEN 35
#define SXA_MSG_TYPE_LEN 21
#define SXB_MSG_TYPE_LEN 23
#define SXASXB_MSG_TYPE_LEN 23
#define GX_MSG_TYPE_LEN 8
#define SYSTEM_MSG_TYPE_LEN 4

#define GTP_CREATE_SESSION_REQ                               (32)
#define GTP_CREATE_SESSION_RSP                               (33)
#define GTP_MODIFY_BEARER_REQ                                (34)
#define GTP_MODIFY_BEARER_RSP                                (35)
#define GTP_DELETE_SESSION_REQ                               (36)
#define GTP_DELETE_SESSION_RSP                               (37)
#define GTP_CREATE_BEARER_REQ                                (95)
#define GTP_CREATE_BEARER_RSP                                (96)


enum GxMessageType {
    OSS_CCR_INITIAL = 120,
    OSS_CCA_INITIAL,
    OSS_CCR_UPDATE,
    OSS_CCA_UPDATE,
    OSS_CCR_TERMINATE,
    OSS_CCA_TERMINATE,
    OSS_RAR,
    OSS_RAA
};

enum oss_gw_config {
	OSS_SGWC = 01,
	OSS_PGWC = 02,
	OSS_SAEGWC = 03,
	OSS_SGWU = 04,
	OSS_PGWU = 05,
	OSS_SAEGWU = 06
};

typedef enum {
	// REQ = 0,
	ACC = 0,
	REJ = 1,
	SENT = 0,
	RCVD = 1,
	BOTH = 0
} Dir;

typedef enum {
	S11,
	S5S8,
	SX,
	GX,
	S1U,
	SGI
}CLIinterface;

typedef enum {
	itS11,
	itS5S8,
	itSxa,
	itSxb,
	itSxaSxb,
	itGx,
	itS1U,
	itSGI,
} EInterfaceType;

typedef enum {
	dIn,
	dOut,
	dRespSend,
	dRespRcvd,
	dBoth,
	dNone
} EDirection;

typedef enum {
	DECREMENT,
	INCREMENT,
} Operation;

typedef enum {
	number_of_active_session,
	number_of_users,
	number_of_bearers,
	number_of_pdn_connections,
} SystemStats;

typedef struct {
	int msgtype;
	const char *msgname;
	EDirection dir;
} MessageType;

typedef struct {
	int cnt[2];
	//time_t ts;
	char ts[80];
} Statistic;

#define FALSE 0
#define TRUE 1

typedef struct {
       struct in_addr ipaddr;
       EInterfaceType intfctype;

       bool status;
       int response_timeout;  //TRANSMIT TIMER
       uint8_t maxtimeout;    //TRANSMIT COUNT in cp.cfg
       uint8_t timeouts;

       char lastactivity[80];

       int hcrequest[2];
       int hcresponse[2];
       union {
               Statistic s11[51];
               Statistic s5s8[37];
               Statistic sxa[21];
               Statistic sxb[23];
               Statistic sxasxb[23];
               Statistic gx[23];
       } stats;
} SPeer;
/*
 * 0 --> active sessions
 * 1 --> Nbr-of-ues
 * 2 --> Nbr-of-pdn-conn
 * & so on.
 */

typedef struct {
       uint8_t gw_type;
       uint64_t *upsecs;
       uint64_t *resetsecs;
       //uint64_t upsecs;
       uint64_t stats[5];
       SPeer *peer[MAX_PEER];
}cli_node_t;

extern cli_node_t cli_node;

extern int cnt_peer;  /*last index of array*/
extern int nbr_of_peer; /*total nbr of peer count*/

extern int sxlogger;
extern int s11logger;
extern int s5s8logger;
extern int gxlogger;
extern int apilogger;

extern cli_node_t *cli_node_ptr; // OSS cli node ptr

extern MessageType ossS5s8MessageDefs[];
extern MessageType ossS11MessageDefs[];
extern MessageType ossSxaMessageDefs[];
extern MessageType ossSxbMessageDefs[];
extern MessageType ossSxaSxbMessageDefs[];
extern MessageType ossGxMessageDefs[];
extern MessageType ossSystemMessageDefs[];
extern char ossInterfaceStr[][10];
extern char ossInterfaceProtocolStr[][10];
extern char ossGatewayStr[][10];
extern uint64_t oss_reset_time;

#endif
