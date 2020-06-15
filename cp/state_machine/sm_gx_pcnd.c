// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include "sm_pcnd.h"
#include "cp_stats.h"
#include "pfcp_cp_util.h"
#include "pfcp.h"
#include "gtp_messages_decoder.h"
#include "cp_config.h"
#include "gw_adapter.h"

extern struct cp_stats_t cp_stats;

uint8_t
gx_pcnd_check(gx_msg *gx_rx, msg_info *msg)
{
	int ret = 0;
	uint32_t call_id = 0;
	gx_context_t *gx_context = NULL;
	pdn_connection_t *pdn_cntxt = NULL;

	struct sockaddr_in saddr_in;
	saddr_in.sin_family = AF_INET;
	inet_aton("127.0.0.1", &(saddr_in.sin_addr));

	msg->msg_type = gx_rx->msg_type;

	switch(msg->msg_type) {
		case GX_CCA_MSG: {
			if (gx_cca_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type),
						&msg->gx_msg.cca) <= 0) {
			    return -1;
			}

			switch(msg->gx_msg.cca.cc_request_type) {
				case INITIAL_REQUEST:
					update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCA_INITIAL, RCVD, GX);
					break;
				case UPDATE_REQUEST:
					update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCA_UPDATE, RCVD, GX);
					break;
				case TERMINATION_REQUEST:
					update_cli_stats(saddr_in.sin_addr.s_addr, OSS_CCA_TERMINATE, RCVD, GX);
					break;
			}

			break;
		}
		case GX_RAR_MSG: {

			uint32_t call_id = 0;
			uint32_t buflen ;
			update_cli_stats(saddr_in.sin_addr.s_addr, OSS_RAR, RCVD, GX);


			if (gx_rar_unpack((unsigned char *)gx_rx + sizeof(gx_rx->msg_type),
						&msg->gx_msg.rar) <= 0) {
			    return -1;
			}

			break;
		}
	default:
				clLog(clSystemLog, eCLSeverityCritical, "%s::process_msgs-"
					"\n\tcase: SAEGWC::spgw_cfg= %d;"
					"\n\tReceived Gx Message : "
					"%d not supported... Discarding\n", __func__,
					cp_config->cp_type, gx_rx->msg_type);
			return -1;
	}

	return 0;
}
