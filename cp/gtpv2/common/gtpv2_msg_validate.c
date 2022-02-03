// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
//
// SPDX-License-Identifier: Apache-2.0

#include "sm_struct.h"
#include "gtp_messages.h"
#include "spgw_config_struct.h"
#include "stdio.h"
#include "gtpv2_error_rsp.h"
#include "gtpv2_interface.h"
#include "util.h"
#include "cp_log.h"


static
int validate_csrsp_msg(create_sess_rsp_t *csrsp)
{
    LOG_MSG(LOG_NEVER, "csrsp = %p ", csrsp);
    return 0;
}

static
int validate_mbrsp_msg(mod_bearer_rsp_t *mbrsp)
{
    LOG_MSG(LOG_NEVER, "mbrsp = %p ", mbrsp);
    return 0;
}

/* Requirement : validate presence of EBI in DSReq 
*/

static
int validate_dsrsp_msg(del_sess_rsp_t *dsrsp)
{
    LOG_MSG(LOG_NEVER, "dsrsp = %p ", dsrsp);
    return 0;
}

static
int validate_cbreq_msg(create_bearer_req_t *cbreq)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", cbreq);
    return 0;
}

static
int validate_cbrsp_msg(create_bearer_rsp_t *cbrsp)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", cbrsp);
    return 0;
}

static
int validate_dbreq_msg(del_bearer_req_t *dbreq)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", dbreq);
    return 0;
}

static
int validate_dbrsp_msg(del_bearer_rsp_t *dbrsp)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", dbrsp);
    return 0;
}

static
int validate_ubreq_msg(upd_bearer_req_t *ubreq)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", ubreq);
    return 0;
}

static
int validate_ubrsp_msg(upd_bearer_rsp_t *ubrsp)
{
    LOG_MSG(LOG_NEVER, "gtp message = %p ", ubrsp);
    return 0;
}

int validate_gtpv2_message_content(msg_info_t *msg)
{
    int ret=0;
    switch(msg->msg_type)
    {
    	case GTP_CREATE_SESSION_RSP:
        {
            ret = validate_csrsp_msg(&msg->rx_msg.cs_rsp);
            break;
        }        
        case GTP_MODIFY_BEARER_RSP:
        {
            ret = validate_mbrsp_msg(&msg->rx_msg.mb_rsp);
            break;
        }
        case GTP_DELETE_SESSION_RSP:
        {
            ret = validate_dsrsp_msg(&msg->rx_msg.ds_rsp);
            break;
        }        
        case GTP_CREATE_BEARER_REQ:
        {
            ret = validate_cbreq_msg(&msg->rx_msg.cb_req);
            break;
        } 
        case GTP_CREATE_BEARER_RSP:
        {
            ret = validate_cbrsp_msg(&msg->rx_msg.cb_rsp);
            break;
        }        
        case GTP_DELETE_BEARER_REQ:
        {
            ret = validate_dbreq_msg(&msg->rx_msg.db_req);
            break;
        } 
        case GTP_DELETE_BEARER_RSP:
        {
            ret = validate_dbrsp_msg(&msg->rx_msg.db_rsp);
            break;
        }        
        case GTP_UPDATE_BEARER_REQ:
        {
            ret = validate_ubreq_msg(&msg->rx_msg.ub_req);
            break;
        } 
        case GTP_UPDATE_BEARER_RSP:
        {
            ret = validate_ubrsp_msg(&msg->rx_msg.ub_rsp);
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
