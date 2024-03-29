// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <string.h>

#include "util.h"

const char *
cause_str(enum cause_value cause)
{
	switch (cause) {
	case GTPV2C_CAUSE_REQUEST_ACCEPTED:
		return "GTPV2C_CAUSE_REQUEST_ACCEPTED";
	case GTPV2C_CAUSE_REQUEST_ACCEPTED_PARTIALLY:
		return "GTPV2C_CAUSE_REQUEST_ACCEPTED_PARTIALLY";
	case GTPV2C_CAUSE_NEW_PDN_TYPE_NETWORK_PREFERENCE:
		return "GTPV2C_CAUSE_NEW_PDN_TYPE_NETWORK_PREFERENCE";
	case GTPV2C_CAUSE_NEW_PDN_TYPE_SINGLE_ADDR_BEARER:
		return "GTPV2C_CAUSE_NEW_PDN_TYPE_SINGLE_ADDR_BEARER";
	case GTPV2C_CAUSE_CONTEXT_NOT_FOUND:
		return "GTPV2C_CAUSE_CONTEXT_NOT_FOUND";
	case GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT:
		return "GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT";
	case GTPV2C_CAUSE_INVALID_LENGTH:
		return "GTPV2C_CAUSE_INVALID_LENGTH";
	case GTPV2C_CAUSE_SERVICE_NOT_SUPPORTED:
		return "GTPV2C_CAUSE_SERVICE_NOT_SUPPORTED";
	case GTPV2C_CAUSE_MANDATORY_IE_INCORRECT:
		return "GTPV2C_CAUSE_MANDATORY_IE_INCORRECT";
	case GTPV2C_CAUSE_MANDATORY_IE_MISSING:
		return "GTPV2C_CAUSE_MANDATORY_IE_MISSING";
	case GTPV2C_CAUSE_SYSTEM_FAILURE:
		return "GTPV2C_CAUSE_SYSTEM_FAILURE";
	case GTPV2C_CAUSE_NO_RESOURCES_AVAILABLE:
		return "GTPV2C_CAUSE_NO_RESOURCES_AVAILABLE";
	case GTPV2C_CAUSE_MISSING_UNKNOWN_APN:
		return "GTPV2C_CAUSE_MISSING_UNKNOWN_APN";
	case GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED:
		return "GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED";
	case GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED:
		return "GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED";
	case GTPV2C_CAUSE_REQUEST_REJECTED:
		return "GTPV2C_CAUSE_REQUEST_REJECTED";
	case GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING:
		return "GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING";
	case GTPV2C_CAUSE_CONDITIONAL_IE_MISSING:
		return "GTPV2C_CAUSE_CONDITIONAL_IE_MISSING";
		/* TODO: populate entire 8.4.1 table */
	default:
		return "Unknown... Look at 29.274 Table 8.4.1";
	}
}


const char *
gtp_type_str(uint8_t type)
{
	/* GTP Message Type Values */
	switch (type) {
	case GTP_ECHO_REQ:
		return "GTP_ECHO_REQ";
	case GTP_ECHO_RSP:
		return "GTP_ECHO_RSP";
	case GTP_VERSION_NOT_SUPPORTED_IND:
		return "GTP_VERSION_NOT_SUPPORTED_IND";
	case GTP_CREATE_SESSION_REQ:
		return "GTP_CREATE_SESSION_REQ";
	case GTP_CREATE_SESSION_RSP:
		return "GTP_CREATE_SESSION_RSP";
	case GTP_MODIFY_BEARER_REQ:
		return "GTP_MODIFY_BEARER_REQ";
	case GTP_MODIFY_BEARER_RSP:
		return "GTP_MODIFY_BEARER_RSP";
	case GTP_DELETE_SESSION_REQ:
		return "GTP_DELETE_SESSION_REQ";
	case GTP_DELETE_SESSION_RSP:
		return "GTP_DELETE_SESSION_RSP";
	case GTP_MODIFY_BEARER_CMD:
		return "GTP_MODIFY_BEARER_CMD";
	case GTP_MODIFY_BEARER_FAILURE_IND:
		return "GTP_MODIFY_BEARER_FAILURE_IND";
	case GTP_DELETE_BEARER_CMD:
		return "GTP_DELETE_BEARER_CMD";
	case GTP_DELETE_BEARER_FAILURE_IND:
		return "GTP_DELETE_BEARER_FAILURE_IND";
	case GTP_BEARER_RESOURCE_CMD:
		return "GTP_BEARER_RESOURCE_CMD";
	case GTP_BEARER_RESOURCE_FAILURE_IND:
		return "GTP_BEARER_RESOURCE_FAILURE_IND";
	case GTP_DOWNLINK_DATA_NOTIFICATION_FAILURE_IND:
		return "GTP_DOWNLINK_DATA_NOTIFICATION_FAILURE_IND";
	case GTP_TRACE_SESSION_ACTIVATION:
		return "GTP_TRACE_SESSION_ACTIVATION";
	case GTP_TRACE_SESSION_DEACTIVATION:
		return "GTP_TRACE_SESSION_DEACTIVATION";
	case GTP_STOP_PAGING_IND:
		return "GTP_STOP_PAGING_IND";
	case GTP_CREATE_BEARER_REQ:
		return "GTP_CREATE_BEARER_REQ";
	case GTP_CREATE_BEARER_RSP:
		return "GTP_CREATE_BEARER_RSP";
	case GTP_UPDATE_BEARER_REQ:
		return "GTP_UPDATE_BEARER_REQ";
	case GTP_UPDATE_BEARER_RSP:
		return "GTP_UPDATE_BEARER_RSP";
	case GTP_DELETE_BEARER_REQ:
		return "GTP_DELETE_BEARER_REQ";
	case GTP_DELETE_BEARER_RSP:
		return "GTP_DELETE_BEARER_RSP";
	case GTP_DELETE_PDN_CONNECTION_SET_REQ:
		return "GTP_DELETE_PDN_CONNECTION_SET_REQ";
	case GTP_DELETE_PDN_CONNECTION_SET_RSP:
		return "GTP_DELETE_PDN_CONNECTION_SET_RSP";
	case GTP_IDENTIFICATION_REQ:
		return "GTP_IDENTIFICATION_REQ";
	case GTP_IDENTIFICATION_RSP:
		return "GTP_IDENTIFICATION_RSP";
	case GTP_CONTEXT_REQ:
		return "GTP_CONTEXT_REQ";
	case GTP_CONTEXT_RSP:
		return "GTP_CONTEXT_RSP";
	case GTP_CONTEXT_ACK:
		return "GTP_CONTEXT_ACK";
	case GTP_FORWARD_RELOCATION_REQ:
		return "GTP_FORWARD_RELOCATION_REQ";
	case GTP_FORWARD_RELOCATION_RSP:
		return "GTP_FORWARD_RELOCATION_RSP";
	case GTP_FORWARD_RELOCATION_COMPLETE_NTF:
		return "GTP_FORWARD_RELOCATION_COMPLETE_NTF";
	case GTP_FORWARD_RELOCATION_COMPLETE_ACK:
		return "GTP_FORWARD_RELOCATION_COMPLETE_ACK";
	case GTP_FORWARD_ACCESS_CONTEXT_NTF:
		return "GTP_FORWARD_ACCESS_CONTEXT_NTF";
	case GTP_FORWARD_ACCESS_CONTEXT_ACK:
		return "GTP_FORWARD_ACCESS_CONTEXT_ACK";
	case GTP_RELOCATION_CANCEL_REQ:
		return "GTP_RELOCATION_CANCEL_REQ";
	case GTP_RELOCATION_CANCEL_RSP:
		return "GTP_RELOCATION_CANCEL_RSP";
	case GTP_CONFIGURE_TRANSFER_TUNNEL:
		return "GTP_CONFIGURE_TRANSFER_TUNNEL";
	case GTP_DETACH_NTF:
		return "GTP_DETACH_NTF";
	case GTP_DETACH_ACK:
		return "GTP_DETACH_ACK";
	case GTP_CS_PAGING_INDICATION:
		return "GTP_CS_PAGING_INDICATION";
	case GTP_RAN_INFORMATION_RELAY:
		return "GTP_RAN_INFORMATION_RELAY";
	case GTP_ALERT_MME_NTF:
		return "GTP_ALERT_MME_NTF";
	case GTP_ALERT_MME_ACK:
		return "GTP_ALERT_MME_ACK";
	case GTP_UE_ACTIVITY_NTF:
		return "GTP_UE_ACTIVITY_NTF";
	case GTP_UE_ACTIVITY_ACK:
		return "GTP_UE_ACTIVITY_ACK";
	case GTP_CREATE_FORWARDING_TUNNEL_REQ:
		return "GTP_CREATE_FORWARDING_TUNNEL_REQ";
	case GTP_CREATE_FORWARDING_TUNNEL_RSP:
		return "GTP_CREATE_FORWARDING_TUNNEL_RSP";
	case GTP_SUSPEND_NTF:
		return "GTP_SUSPEND_NTF";
	case GTP_SUSPEND_ACK:
		return "GTP_SUSPEND_ACK";
	case GTP_RESUME_NTF:
		return "GTP_RESUME_NTF";
	case GTP_RESUME_ACK:
		return "GTP_RESUME_ACK";
	case GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ:
		return "GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ";
	case GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP:
		return "GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP";
	case GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ:
		return "GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ";
	case GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP:
		return "GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP";
	case GTP_RELEASE_ACCESS_BEARERS_REQ:
		return "GTP_RELEASE_ACCESS_BEARERS_REQ";
	case GTP_RELEASE_ACCESS_BEARERS_RSP:
		return "GTP_RELEASE_ACCESS_BEARERS_RSP";
	case GTP_DOWNLINK_DATA_NOTIFICATION:
		return "GTP_DOWNLINK_DATA_NOTIFICATION";
	case GTP_DOWNLINK_DATA_NOTIFICATION_ACK:
		return "GTP_DOWNLINK_DATA_NOTIFICATION_ACK";
	case GTP_RESERVED:
		return "GTP_RESERVED";
	case GTP_PGW_RESTART_NOTIFICATION:
		return "GTP_PGW_RESTART_NOTIFICATION";
	case GTP_PGW_RESTART_NOTIFICATION_ACK:
		return "GTP_PGW_RESTART_NOTIFICATION_ACK";
	case GTP_UPDATE_PDN_CONNECTION_SET_REQ:
		return "GTP_UPDATE_PDN_CONNECTION_SET_REQ";
	case GTP_UPDATE_PDN_CONNECTION_SET_RSP:
		return "GTP_UPDATE_PDN_CONNECTION_SET_RSP";
	case GTP_MBMS_SESSION_START_REQ:
		return "GTP_MBMS_SESSION_START_REQ";
	case GTP_MBMS_SESSION_START_RSP:
		return "GTP_MBMS_SESSION_START_RSP";
	case GTP_MBMS_SESSION_UPDATE_REQ:
		return "GTP_MBMS_SESSION_UPDATE_REQ";
	case GTP_MBMS_SESSION_UPDATE_RSP:
		return "GTP_MBMS_SESSION_UPDATE_RSP";
	case GTP_MBMS_SESSION_STOP_REQ:
		return "GTP_MBMS_SESSION_STOP_REQ";
	case GTP_MBMS_SESSION_STOP_RSP:
		return "GTP_MBMS_SESSION_STOP_RSP";
	case GTP_MSG_END:
		return "GTP_MSG_END";
	default:
		return "UNKNOWN";
	}
}
