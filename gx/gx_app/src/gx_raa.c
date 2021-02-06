// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdint.h>
#include <sys/queue.h>

#include "gx.h"
#include "cp_log.h"

extern void hexDump(char *desc, void *address, int len);
extern gx_trans_data_t *gx_trans_list;
/*
*
*       Fun:    gx_send_raa
*
*       Desc:
*
*       Ret:
*
*       Notes:  None
*
*       File:   gx_raa.c
*
*/
int gx_send_raa(void *buf)
{
    gx_msg *req = (gx_msg*)buf;
    void *data = (void *)(&req->data.cp_raa);
    LOG_MSG(LOG_DEBUG2, "Message type(%d). Received RAR with sequence number = %d ", 
                req->msg_type, req->seq_num);
	int ret = FD_REASON_OK;
	struct msg *ans = NULL;
	//uint32_t buflen ;
#ifdef GX_DEBUG
	LOG_MSG(LOG_DEBUG, "length is %d", *(uint32_t*)data );
	hexDump("gx_raa", data, *(uint32_t*)data);
#endif
	GxRAA *gx_raa = (GxRAA*)malloc(sizeof(*gx_raa));    /* allocate the RAA structure */
	memset((void*)gx_raa, 0, sizeof(*gx_raa));

#if 0
    uint16_t rar_seq_num = *(uint16_t *)(data);
    data = (unsigned char *)data + sizeof(uint16_t);
#endif

	gx_raa_unpack((unsigned char *)data, gx_raa);
	//buflen = gx_raa_calc_length (&gx_raa);

	//memcpy(&rqst_ptr, ((unsigned char *)data + buflen -1), sizeof(unsigned long));
	//memcpy(&ans, ((unsigned char *)data + *(uint32_t*)data), sizeof(ans));

    pending_gx_trans_t *key = NULL; 
    key = LIST_FIRST(&gx_trans_list->pending_gx_trans);
    while(key != NULL) {
        if(key->seq_num == req->seq_num) {
            ans = (struct msg *)key->msg;
            LIST_REMOVE(key, next_trans_entry);
            break;
        }
        key = LIST_NEXT(key, next_trans_entry);
    }
    assert(ans != NULL);
	//	ans = (struct msg*)rqst_ptr;

	/* construct the message */
	FDCHECK_MSG_NEW_ANSWER_FROM_REQ(fd_g_config->cnf_dict, ans, ret, goto err);
	//FDCHECK_MSG_NEW_APPL( gxDict.cmdRAA, gxDict.appGX, ans, ret, goto err);
	FDCHECK_MSG_ADD_ORIGIN(ans, ret, goto err);

	//if (gx_raa->presence.session_id)
	//	FDCHECK_MSG_ADD_AVP_OSTR(gxDict.avp_session_id, ans, MSG_BRW_LAST_CHILD,
	//			gx_raa->session_id.val, gx_raa->session_id.len, ret, goto err);

	//FDCHECK_MSG_ADD_AVP_OSTR(gxDict.avp_destination_host, ans, MSG_BRW_LAST_CHILD, fd_g_config->cnf_diamid, fd_g_config->cnf_diamid_len, ret, goto err );
	//FDCHECK_MSG_ADD_AVP_OSTR( gxDict.avp_destination_host, ans, MSG_BRW_LAST_CHILD,
	//		"dstest4.test3gpp.net", strlen("dstest4.test3gpp.net"), ret, goto   err );
	//FDCHECK_MSG_ADD_AVP_OSTR( gxDict.avp_destination_realm, ans, MSG_BRW_LAST_CHILD, fd_g_config->cnf_diamrlm, fd_g_config->cnf_diamrlm_len, ret, goto err );

    if(gx_raa->presence.result_code) {
	    FDCHECK_MSG_ADD_AVP_U32(gxDict.avp_result_code, ans, MSG_BRW_LAST_CHILD,
				gx_raa->result_code, ret, goto err );
    }
    if(gx_raa->presence.experimental_result) {
        struct avp *experimental_result = NULL;
	    FDCHECK_MSG_ADD_AVP_GROUPED_2(gxDict.avp_experimental_result, ans, MSG_BRW_LAST_CHILD,
				experimental_result, ret, goto err );

        FDCHECK_MSG_ADD_AVP_U32(gxDict.avp_vendor_id, experimental_result,
                                MSG_BRW_LAST_CHILD, gx_raa->experimental_result.vendor_id,
                                ret, goto err);
        FDCHECK_MSG_ADD_AVP_U32(gxDict.avp_experimental_result_code, experimental_result,
                                MSG_BRW_LAST_CHILD, gx_raa->experimental_result.experimental_result_code,
                                ret, goto err);

    }


	//FDCHECK_MSG_ADD_AVP_OSTR( gxDict.avp_auth_application_id, ans, MSG_BRW_LAST_CHILD,
	//		gxDict.appGX, sizeof(gxDict.appGX), ret, goto err );

	//TODO - FILL IN HERE
#if GX_DEBUG
FD_DUMP_MESSAGE(ans);
#endif

   /* send the message */
   FDCHECK_MSG_SEND( ans, NULL, NULL, ret, goto err );
   goto fini;

err:
   /* free the message since an error occurred */
   FDCHECK_MSG_FREE(ans);

fini:
   return ret;
}

/*
*
*       Fun:    gx_raa_cb
*
*       Desc:   CMDNAME call back
*
*       Ret:    0
*
*       File:   gx_raa.c
*
    The Re-Auth-Answer (RAA) command, indicated by
    the Command-Code field set to 258 and the 'R'
    bit cleared in the Command Flags field, is sent to/from MME or SGSN.
*
    Re-Auth-Answer ::= <Diameter Header: 258, PXY, 16777238>
          < Session-Id >
          [ DRMP ]
          { Origin-Host }
          { Origin-Realm }
          [ Result-Code ]
          [ Experimental-Result ]
          [ Origin-State-Id ]
          [ OC-Supported-Features ]
          [ OC-OLR ]
          [ IP-CAN-Type ]
          [ RAT-Type ]
          [ AN-Trusted ]
      * 2 [ AN-GW-Address ]
          [ 3GPP-SGSN-MCC-MNC ]
          [ 3GPP-SGSN-Address ]
          [ 3GPP-SGSN-Ipv6-Address ]
          [ RAI ]
          [ 3GPP-User-Location-Info ]
          [ User-Location-Info-Time ]
          [ NetLoc-Access-Support ]
          [ User-CSG-Information ]
          [ 3GPP-MS-TimeZone ]
          [ Default-QoS-Information ]
      *   [ Charging-Rule-Report ]
          [ Error-Message ]
          [ Error-Reporting-Host ]
          [ Failed-AVP ]
      *   [ Proxy-Info ]
      *   [ AVP ]

*/
int gx_raa_cb
(
   struct msg ** msg,
   struct avp * pavp,
   struct session * sess,
   void * data,
   enum disp_action * act
)
{
   int ret = FD_REASON_OK;
   struct msg *ans = *msg;
   struct msg *qry = NULL;
   GxRAA *raa = NULL;

   LOG_MSG(LOG_DEBUG, "===== RAA RECEIVED FOM PCEF======= ");
//#if 1
//FD_DUMP_MESSAGE(ans);
//#endif
//
//   /* retrieve the original query associated with the answer */
//   CHECK_FCT(fd_msg_answ_getq (ans, &qry));
//
//   /* allocate the raa message */
//   raa = (GxRAA*)malloc(sizeof(*raa));
//
//   memset((void*)raa, 0, sizeof(*raa));
//
//   ret = gx_raa_parse(*msg, raa);
//   if (ret != FD_REASON_OK)
//      goto err;
//
//   /*
//    *  TODO - Add request processing code
//    */
//
//   //gx_raa_free(raa);
//   goto fini2;
//
//err:
//   //gx_raa_free(raa);
//   free(raa);
//   goto fini2;
//
////fini1:
//
//fini2:
//   FDCHECK_MSG_FREE(*msg);
//   *msg = NULL;
//   return 0;
}
