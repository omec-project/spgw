// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <rte_cfgfile.h>
#include <rte_memory.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "../../cp/cp_config.h"
#include "cp_interface.h"
#include "../../cp/ue.h"
#include "util.h"
#include "gw_adapter.h"
#include "crest.h"
#include "clogger.h"

#include "pfcp_cp_set_ie.h"

int s11logger;
int s5s8logger;
int sxlogger;
int gxlogger;
int apilogger;


MessageType ossS5s8MessageDefs[] = {
        {       3       , "Version Not Supported Indication",dNone      },
        {       32      , "Create Session Request",  dIn        	},//if SGWC then send, if PGWC then recv
        {       33      , "Create Session Response",dRespRcvd   	},//if SGWC then recv, if PGWC then send
        {       36      , "Delete Session Request",  dIn        	},//if SGWC then send, if PGWC then recv
        {       37      , "Delete Session Response",dRespRcvd   	},//if SGWC then recv, if PGWC then send
        {       34      , "Modify Bearer Request",dIn 			},	  //if SGWC then send, if PGWC then recv
        {       35      , "Modify Bearer Response",dRespRcvd    	},//if SGWC then recv, if PGWC then send
        {       40      , "Remote UE Report Notification",dNone 	},
        {       41      , "Remote UE Report Acknowledge",dNone  	},
        {       38      , "Change Notification Request",dNone   	},
        {       39      , "Change Notification Response",dNone  	},
        {       164     , "Resume Notification",dNone   		},
        {       165     , "Resume Acknowledge",dNone    		},
        {       64      , "Modify Bearer Command",dNone 		},
        {       65      , "Modify Bearer Failure Indication",dNone      },
        {       66      , "Delete Bearer Command",dNone 		},
        {       67      , "Delete Bearer Failure Indication",dNone      },
        {       68      , "Bearer Resource Command",dNone       	},
        {       69      , "Bearer Resource Failure Indication",dNone    },
        {       71      , "Trace Session Activation",dNone      	},
        {       72      , "Trace Session Deactivation",dNone    	},
        {       95      , "Create Bearer Request",dIn 		},//if SGWC then recv, if PGWC then send
        {       96      , "Create Bearer Response",dOut        	},//if SGWC then send, if PGWC then recv
        {       97      , "Update Bearer Request",dNone 		},
        {       98      , "Update Bearer Response",dNone        	},
        {       99      , "Delete Bearer Request",dNone 		},
        {       100     , "Delete Bearer Response",dNone        	},
        {       101     , "Delete PDN Connection Set Request",dNone     },
        {       102     , "Delete PDN Connection Set Response",dNone    },
        {       103     , "PGW Downlink Triggering Notification",dNone  },
        {       104     , "PGW Downlink Triggering Acknowledge",dNone   },
        {       162     , "Suspend Notification",dNone  		},
        {       163     , "Suspend Acknowledge",dNone   		},
        {       200     , "Update PDN Connection Set Request",dNone     },
        {       201     , "Update PDN Connection Set Response",dNone    },
        {       -1      , NULL,dNone  					}
};

MessageType ossS11MessageDefs[] = {
        {       3       ,"Version Not Supported Indication", dNone 	},
        {       32      ,"Create Session Request", dIn 			},
        {       33      ,"Create Session Response", dRespSend 		},
        {       36      ,"Delete Session Request", dIn 			},
        {       37      ,"Delete Session Response", dRespSend 		},
        {       34      ,"Modify Bearer Request", dIn 			},
        {       35      ,"Modify Bearer Response", dRespSend 		},
        {       40      ,"Remote UE Report Notification", dNone 	},
        {       41      ,"Remote UE Report Acknowledge", dNone 		},
        {       38      ,"Change Notification Request", dNone 		},
        {       39      ,"Change Notification Response", dNone 		},
        {       164     ,"Resume Notification", dNone 			},
        {       165     ,"Resume Acknowledge", dNone 			},
        {       64      ,"Modify Bearer Command", dNone 		},
        {       65      ,"Modify Bearer Failure Indication", dNone 	},
        {       66      ,"Delete Bearer Command", dNone 		},
        {       67      ,"Delete Bearer Failure Indication", dNone 	},
        {       68      ,"Bearer Resource Command", dNone 		},
        {       69      ,"Bearer Resource Failure Indication", dNone 	},
        {       70      ,"Downlink Data Notification Failure Indication", dNone },
        {       71      ,"Trace Session Activation", dNone 		},
        {       72      ,"Trace Session Deactivation", dNone 		},
        {       73      ,"Stop Paging Indication", dNone 		},
        {       95      ,"Create Bearer Request", dOut	 		},
        {       96      ,"Create Bearer Response", dRespRcvd	 	},
        {       97      ,"Update Bearer Request", dNone 		},
        {       98      ,"Update Bearer Response", dNone 		},
        {       99      ,"Delete Bearer Request", dNone 		},
        {       100     ,"Delete Bearer Response", dNone 		},
        {       101     ,"Delete PDN Connection Set Request", dNone 	},
        {       102     ,"Delete PDN Connection Set Response", dNone 	},
        {       103     ,"PGW Downlink Triggering Notification", dNone 	},
        {       104     ,"PGW Downlink Triggering Acknowledge", dNone 	},
        {       162     ,"Suspend Notification", dNone 			},
        {       163     ,"Suspend Acknowledge", dNone 			},
        {       160     ,"Create Forwarding Tunnel Request", dNone 	},
        {       161     ,"Create Forwarding Tunnel Response", dNone 	},
        {       166     ,"Create Indirect Data Forwarding Tunnel Request", dNone },
        {       167     ,"Create Indirect Data Forwarding Tunnel Response", dNone },
        {       168     ,"Delete Indirect Data Forwarding Tunnel Request", dNone },
        {       169     ,"Delete Indirect Data Forwarding Tunnel Response", dNone },
        {       170     ,"Release Access Bearers Request", dIn 		},
        {       171     ,"Release Access Bearers Response", dRespSend 	},
        {       176     ,"Downlink Data Notification", dOut 		},
        {       177     ,"Downlink Data Notification Acknowledge", dRespRcvd },
        {       179     ,"PGW Restart Notification", dNone 		},
        {       180     ,"PGW Restart Notification Acknowledge", dNone 	},
        {       211     ,"Modify Access Bearers Request", dNone 	},
        {       212     ,"Modify Access Bearers Response", dNone 	},
        {       -1      , NULL,dNone  					}
};

MessageType ossSxaMessageDefs[] = {
        {	   1	  ,"PFCP Heartbeat Request",dNone		},
        {	   2	  ,"PFCP Heartbeat Response",dNone		},
        {	   5	  ,"PFCP Association Setup Request",dOut	},
        {	   6	  ,"PFCP Association Setup Response",dRespRcvd	},
        {	   7	  ,"PFCP Association Update Request",dNone	},
        {	   8	  ,"PFCP Association Update Response",dNone	},
        {	   9	  ,"PFCP Association Release Request",dNone	},
        {	   10	  ,"PFCP Association Release Response",dNone	},
        {	   11	  ,"PFCP Version Not Supported Response",dNone	},
        {	   12	  ,"PFCP Node Report Request",dNone		},
        {	   13	  ,"PFCP Node Report Response",dNone		},
        {	   14	  ,"PFCP Session Set Deletion Request",dNone	},
        {	   15	  ,"PFCP Session Set Deletion Response",dNone	},
        {	   50	  ,"PFCP Session Establishment Request",dOut	},
        {	   51	  ,"PFCP Session Establishment Response",dRespRcvd},
        {	   52	  ,"PFCP Session Modification Request",dOut	},
        {	   53	  ,"PFCP Session Modification Response",dRespRcvd},
        {	   54	  ,"PFCP Session Deletion Request",dOut		},
        {	   55	  ,"PFCP Session Deletion Response",dRespRcvd	},
        {	   56	  ,"PFCP Session Report Request",dIn		},
        {	   57	  ,"PFCP Session Report Response",dRespSend	},
        {          -1     , NULL,dNone					}
};

MessageType ossSxbMessageDefs[] = {
        {	  1	  ,"PFCP Heartbeat Request",dNone		},
        {	  2	  ,"PFCP Heartbeat Response",dNone		},
	{  	  3       ,"PFCP PFD Management Request",dOut           },
	{         4       ,"PFCP PFD Management Response",dRespRcvd     },
        {	  5	  ,"PFCP Association Setup Request",dOut	},
        {	  6	  ,"PFCP Association Setup Response",dRespRcvd	},
        {	  7	  ,"PFCP Association Update Request",dNone	},
        {	  8	  ,"PFCP Association Update Response",dNone	},
        {	  9	  ,"PFCP Association Release Request",dNone	},
        {	  10	  ,"PFCP Association Release Response",dNone	},
        {	  11	  ,"PFCP Version Not Supported Response",dNone	},
        {	  12	  ,"PFCP Node Report Request",dNone		},
        {	  13	  ,"PFCP Node Report Response",dNone		},
        {	  14	  ,"PFCP Session Set Deletion Request",dNone	},
        {	  15	  ,"PFCP Session Set Deletion Response",dNone	},
        {	  50	  ,"PFCP Session Establishment Request",dOut	},
        {	  51	  ,"PFCP Session Establishment Response",dRespRcvd},
        {	  52	  ,"PFCP Session Modification Request",dOut	},
        {	  53	  ,"PFCP Session Modification Response",dRespRcvd},
        {	  54	  ,"PFCP Session Deletion Request",dOut		},
        {	  55	  ,"PFCP Session Deletion Response",dRespRcvd	},
        {	  56	  ,"PFCP Session Report Request",dIn		},
        {	  57	  ,"PFCP Session Report Response",dRespSend	},
        {         -1      , NULL,dNone  				}
};

MessageType ossSxaSxbMessageDefs[] = {
        {	  1	 ,"PFCP Heartbeat Request",dNone		},
        {	  2	 ,"PFCP Heartbeat Response",dNone		},
        {	  3	 ,"PFCP PFD Management Request",dOut		},
        {	  4	 ,"PFCP PFD Management Response",dRespRcvd	},
        {	  5	 ,"PFCP Association Setup Request",dOut		},
        {	  6	 ,"PFCP Association Setup Response",dRespRcvd	},
        {	  7	 ,"PFCP Association Update Request",dNone	},
        {	  8	 ,"PFCP Association Update Response",dNone	},
        {	  9	 ,"PFCP Association Release Request",dNone	},
        {	  10	 ,"PFCP Association Release Response",dNone	},
        {	  11	 ,"PFCP Version Not Supported Response",dNone	},
        {	  12	 ,"PFCP Node Report Request",dNone		},
        {	  13	 ,"PFCP Node Report Response",dNone		},
        {	  14	 ,"PFCP Session Set Deletion Request",dNone	},
        {	  15	 ,"PFCP Session Set Deletion Response",dNone	},
        {	  50	 ,"PFCP Session Establishment Request",dOut	},
        {	  51	 ,"PFCP Session Establishment Response",dRespRcvd},
        {	  52	 ,"PFCP Session Modification Request",dOut	},
        {	  53	 ,"PFCP Session Modification Response",dRespRcvd},
        {	  54	 ,"PFCP Session Deletion Request",dOut		},
        {	  55	 ,"PFCP Session Deletion Response",dRespRcvd	},
        {	  56	 ,"PFCP Session Report Request",dIn		},
        {	  57	 ,"PFCP Session Report Response",dRespSend	},
        {         -1     , NULL,dNone  					}
};


MessageType ossGxMessageDefs[] = {
    {     120    ,"Credit Control Request Initial",dOut             },
    {     121    ,"Credit Control Answer Initial",dIn               },
    {     122    ,"Credit Control Request Update",dOut              },
    {     123    ,"Credit Control Answer Update",dIn                },
    {     124    ,"Credit Control Request Terminate",dOut         },
    {     125    ,"Credit Control Answer Terminate",dIn           },
    {     126    ,"Re-Auth-Request",dIn                            },
    {     127    ,"Re-Auth Answer",dOut                             },
    {     -1     , NULL,dNone                                       }
};


MessageType ossSystemMessageDefs[] = {
    {  0    ,"Number of active session",dNone	},
    {  1    ,"Number of ues",dNone  		},
    {  2    ,"Number of bearers",dNone          },
    {  3    ,"Number of pdn connections",dNone  },
    {  -1   , NULL,dNone  			}
};

char ossInterfaceStr[][10] = {
    "s11" ,
    "s5s8",
    "sxa",
    "sxb",
    "sxasxb",
    "gx",
	"s1u",
	"sgi",
    "none"
};

char ossInterfaceProtocolStr[][10] = {
    "gtpv2" ,
    "gtpv2",
    "pfcp",
    "pfcp",
    "pfcp",
    "diameter",
	"gtp",
    "none"
};

char ossGatewayStr[][10] = {
    "none",
    "SGWC",
    "PGWC",
    "SAEGWC",
    "SGWU",
    "PGWU",
    "SAEGWU"
};




int s11MessageTypes [] = {
    -1,-1,-1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,2,5,6,3,4,9,10,
    7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,13,14,15,16,17,18,19,20,21,22,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,23,24,25,26,27,
    28,29,30,31,32,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    35,36,33,34,11,12,37,38,39,40,41,42,-1,-1,-1,-1,43,44,-1,45,
    46,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,47,48
};

int s5s8MessageTypes [] = {
    -1,-1,-1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,2,5,6,3,4,9,10,
    7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,13,14,15,16,17,18,-1,19,20,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,21,22,23,24,25,
    26,27,28,29,30,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,31,32,11,12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    33,34
};

int sxaMessageTypes [] = {
    -1,0,1,-1,-1,2,3,4,5,6,7,8,9,10,11,12,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,13,14,15,16,17,18,19,20
};

int sxbMessageTypes [] = {
    -1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,15,16,17,18,19,20,21,22
};

int sxasxbMessageTypes [] = {
    -1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,15,16,17,18,19,20,21,22
};

int gxMessageTypes [] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,
  1,2,3,4,5,6,7
};


void init_cli_module(uint8_t gw_logger)
{
    clSetOption(eCLOptLogFileName, "logs/cp.log");
    clSetOption(eCLOptStatFileName, "logs/cp_stat.log");
    clSetOption(eCLOptAuditFileName, "logs/cp_sys.log");

    clInit("saegw", gw_logger);
    s11logger = clAddLogger("s11", gw_logger);
    s5s8logger = clAddLogger("s5s8", gw_logger);
    gxlogger = clAddLogger("Gx", gw_logger);
    sxlogger = clAddLogger("sx", gw_logger);
    apilogger = clAddLogger("api", gw_logger);
    clAddRecentLogger("sgwc-001","cp",5);
    clStart();
}

