// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_common.h"
#include "sm_struct.h"
#include "gtp_messages.h"
#include "cp_config.h"
#include "sm_enum.h"
#include "gtpv2_error_rsp.h"
#include "assert.h"
#include "clogger.h"
#include "cp_peer.h"
#include "gw_adapter.h"
#include "gtpv2_interface.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
#include "cp_config_apis.h"
#include "ip_pool.h"
#include "gen_utils.h"
#include "pfcp_cp_session.h"
#include "pfcp_enum.h"
#include "pfcp.h"
#include "pfcp_cp_association.h"
#include "spgw_cpp_wrapper.h"
#include "cp_transactions.h"
#include "initial_attach_proc.h"
#include "rab_proc.h"
#include "detach_proc.h"
#include "service_request_proc.h"

extern int s11logger;
extern int s5s8logger;
extern udp_sock_t my_sock;

extern const uint32_t s5s8_sgw_gtpc_base_teid; /* 0xE0FFEE */
int
check_interface_type(uint8_t iface) 
{
	switch(iface){
		case GTPV2C_IFTYPE_S1U_ENODEB_GTPU:
			if ((cp_config->cp_type == SGWC) || (cp_config->cp_type == SAEGWC)) {
				return DESTINATION_INTERFACE_VALUE_ACCESS;
			}
			break;
		case GTPV2C_IFTYPE_S5S8_SGW_GTPU:
			if (cp_config->cp_type == PGWC){
				return DESTINATION_INTERFACE_VALUE_ACCESS;
			}
			break;
		case GTPV2C_IFTYPE_S5S8_PGW_GTPU:
			if (cp_config->cp_type == SGWC){
				return DESTINATION_INTERFACE_VALUE_CORE;
			}
			break;
		case GTPV2C_IFTYPE_S1U_SGW_GTPU:
		case GTPV2C_IFTYPE_S11_MME_GTPC:
		case GTPV2C_IFTYPE_S11S4_SGW_GTPC:
		case GTPV2C_IFTYPE_S11U_SGW_GTPU:
		case GTPV2C_IFTYPE_S5S8_SGW_GTPC:
		case GTPV2C_IFTYPE_S5S8_PGW_GTPC:
		case GTPV2C_IFTYPE_S5S8_SGW_PIMPv6:
		case GTPV2C_IFTYPE_S5S8_PGW_PIMPv6:
		default:
			return -1;
			break;
	}
	return -1;
}