/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "gtpv2_common.h"
#include "clogger.h"
#include "stdio.h"
#include "gtpv2c_error_rsp.h"

static 
int validate_csreq_msg(create_sess_req_t *csr) 
{
	if (csr->indctn_flgs.header.len &&
			csr->indctn_flgs.indication_uimsi) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d Unauthenticated IMSI Not Yet Implemented - "
				"Dropping packet\n", __FILE__, __func__, __LINE__);
	
		return GTPV2C_CAUSE_IMSI_NOT_KNOWN;
	}

	if ((cp_config->cp_type == SGWC) &&
			(!csr->pgw_s5s8_addr_ctl_plane_or_pmip.header.len)) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d Mandatory IE missing. Dropping packet len:%u\n",
				__FILE__, __func__, __LINE__,
				csr->pgw_s5s8_addr_ctl_plane_or_pmip.header.len);
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;
	}

    printf("bc = %d fteid = %d imsi %d apn_ambr = %d pdn_type = %d bc qos = %d rat %d pdn type = %d \n", csr->bearer_contexts_to_be_created.header.len, csr->sender_fteid_ctl_plane.header.len, csr->imsi.header.len, csr->apn_ambr.header.len, csr->pdn_type.header.len, csr->bearer_contexts_to_be_created.bearer_lvl_qos.header.len, csr->rat_type.header.len, (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4));

	if (/*!csr->max_apn_rstrct.header.len
			||*/ !csr->bearer_contexts_to_be_created.header.len
			|| !csr->sender_fteid_ctl_plane.header.len
			|| !csr->imsi.header.len
			|| !csr->apn_ambr.header.len
			|| !csr->pdn_type.header.len
			|| !csr->bearer_contexts_to_be_created.bearer_lvl_qos.header.len
			|| !csr->rat_type.header.len
			|| !(csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4) ) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d Mandatory IE missing. Dropping packet\n",
				__FILE__, __func__, __LINE__);
		return GTPV2C_CAUSE_MANDATORY_IE_MISSING;
	}

	if (csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV6 ||
			csr->pdn_type.pdn_type_pdn_type == PDN_IP_TYPE_IPV4V6) {
		clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%d IPv6 Not Yet Implemented - Dropping packet\n",
				__FILE__, __func__, __LINE__);
		return GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED;
	}    
    return 0;
}

static
int validate_csrsp_msg(create_sess_rsp_t *csrsp)
{
    RTE_SET_USED(csrsp);
    return 0;
}

static
int validate_mbreq_msg(mod_bearer_req_t *mbreq)
{
    RTE_SET_USED(mbreq);
    return 0;
}

static
int validate_mbrsp_msg(mod_bearer_rsp_t *mbrsp)
{
    RTE_SET_USED(mbrsp);
    return 0;
}

/* Requirement : validate presence of EBI in DSReq 
*/
static
int validate_dsreq_msg(del_sess_req_t *dsreq)
{
    RTE_SET_USED(dsreq);
    return 0;
}

static
int validate_dsrsp_msg(del_sess_rsp_t *dsrsp)
{
    RTE_SET_USED(dsrsp);
    return 0;
}

static
int validate_rab_req_msg(rel_acc_bearer_req_t *rab)
{
    RTE_SET_USED(rab);
    return 0;
}

static
int validate_ddnack_msg(downlink_data_notification_t *ddnack)
{
    RTE_SET_USED(ddnack);
    return 0;
}

static
int validate_cbreq_msg(create_bearer_req_t *cbreq)
{
    RTE_SET_USED(cbreq);
    return 0;
}

static
int validate_cbrsp_msg(create_bearer_rsp_t *cbrsp)
{
    RTE_SET_USED(cbrsp);
    return 0;
}

static
int validate_dbreq_msg(del_bearer_req_t *dbreq)
{
    RTE_SET_USED(dbreq);
    return 0;
}

static
int validate_dbrsp_msg(del_bearer_rsp_t *dbrsp)
{
    RTE_SET_USED(dbrsp);
    return 0;
}

static
int validate_ubreq_msg(upd_bearer_req_t *ubreq)
{
    RTE_SET_USED(ubreq);
    return 0;
}

static
int validate_ubrsp_msg(upd_bearer_rsp_t *ubrsp)
{
    RTE_SET_USED(ubrsp);
    return 0;
}

int validate_gtpv2_message_content(msg_info_t *msg)
{
    int ret=0;
    printf("Validate gtpv2 message\n");
    switch(msg->msg_type)
    {
    	case GTP_CREATE_SESSION_REQ:
        {
            printf("Validate CSReq message\n");
            ret = validate_csreq_msg(&msg->gtpc_msg.csr);
		    if(ret != 0 ) {
                cs_error_response(msg, ret,
				cp_config->cp_type != PGWC ? S11_IFACE : S5S8_IFACE);
            }
            break;
        }
    	case GTP_CREATE_SESSION_RSP:
        {
            ret = validate_csrsp_msg(&msg->gtpc_msg.cs_rsp);
            break;
        }        
        case GTP_MODIFY_BEARER_REQ:
        {
            ret = validate_mbreq_msg(&msg->gtpc_msg.mbr);
            break;
        }
        case GTP_MODIFY_BEARER_RSP:
        {
            ret = validate_mbrsp_msg(&msg->gtpc_msg.mb_rsp);
            break;
        }
        case GTP_DELETE_SESSION_REQ:
        {
            ret = validate_dsreq_msg(&msg->gtpc_msg.dsr);
            break;
        } 
        case GTP_DELETE_SESSION_RSP:
        {
            ret = validate_dsrsp_msg(&msg->gtpc_msg.ds_rsp);
            break;
        }        
        case GTP_RELEASE_ACCESS_BEARERS_REQ:
        {
            ret = validate_rab_req_msg(&msg->gtpc_msg.rab);
            break;
        } 
        case GTP_DOWNLINK_DATA_NOTIFICATION_ACK:
        {
            ret = validate_ddnack_msg(&msg->gtpc_msg.ddn_ack);
            break;
        }        
        case GTP_CREATE_BEARER_REQ:
        {
            ret = validate_cbreq_msg(&msg->gtpc_msg.cb_req);
            break;
        } 
        case GTP_CREATE_BEARER_RSP:
        {
            ret = validate_cbrsp_msg(&msg->gtpc_msg.cb_rsp);
            break;
        }        
        case GTP_DELETE_BEARER_REQ:
        {
            ret = validate_dbreq_msg(&msg->gtpc_msg.db_req);
            break;
        } 
        case GTP_DELETE_BEARER_RSP:
        {
            ret = validate_dbrsp_msg(&msg->gtpc_msg.db_rsp);
            break;
        }        
        case GTP_UPDATE_BEARER_REQ:
        {
            ret = validate_ubreq_msg(&msg->gtpc_msg.ub_req);
            break;
        } 
        case GTP_UPDATE_BEARER_RSP:
        {
            ret = validate_ubrsp_msg(&msg->gtpc_msg.ub_rsp);
            break;
        }        
        default:
        {
            assert(0); // unhandled message reception
            break;
        }
    }
    return ret;
}
