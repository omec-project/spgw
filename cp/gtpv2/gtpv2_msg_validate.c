/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config_new.h"
#include "gtpv2_common.h"
#include "clogger.h"
#include "stdio.h"

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

int validate_gtpv2_message_content(msg_info *msg)
{
    switch(msg->msg_type)
    {
    	case GTP_CREATE_SESSION_REQ:
        {
            validate_csreq_msg(&msg->gtpc_msg.csr);
            break;
        }
        default:
            break;
    }
    return 0;
}


