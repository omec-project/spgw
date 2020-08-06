/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "clogger.h"
#include "stdio.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_interface.h"


static
int validate_csrsp_msg(create_sess_rsp_t *csrsp)
{
    RTE_SET_USED(csrsp);
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
int validate_dsrsp_msg(del_sess_rsp_t *dsrsp)
{
    RTE_SET_USED(dsrsp);
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
    	case GTP_CREATE_SESSION_RSP:
        {
            ret = validate_csrsp_msg(&msg->gtpc_msg.cs_rsp);
            break;
        }        
        case GTP_MODIFY_BEARER_RSP:
        {
            ret = validate_mbrsp_msg(&msg->gtpc_msg.mb_rsp);
            break;
        }
        case GTP_DELETE_SESSION_RSP:
        {
            ret = validate_dsrsp_msg(&msg->gtpc_msg.ds_rsp);
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
