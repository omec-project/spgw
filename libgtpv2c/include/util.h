// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#ifndef _LIBGTPV2C_UTIL_H_
#define _LIBGTPV2C_UTIL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GTP Message Type Values */
#define GTP_ECHO_REQ                                         (1)
#define GTP_ECHO_RSP                                         (2)
#define GTP_VERSION_NOT_SUPPORTED_IND                        (3)
#define GTP_CREATE_SESSION_REQ                               (32)
#define GTP_CREATE_SESSION_RSP                               (33)
#define GTP_MODIFY_BEARER_REQ                                (34)
#define GTP_MODIFY_BEARER_RSP                                (35)
#define GTP_DELETE_SESSION_REQ                               (36)
#define GTP_DELETE_SESSION_RSP                               (37)
#define GTP_MODIFY_BEARER_CMD                                (64)
#define GTP_MODIFY_BEARER_FAILURE_IND                        (65)
#define GTP_DELETE_BEARER_CMD                                (66)
#define GTP_DELETE_BEARER_FAILURE_IND                        (67)
#define GTP_BEARER_RESOURCE_CMD                              (68)
#define GTP_BEARER_RESOURCE_FAILURE_IND                      (69)
#define GTP_DOWNLINK_DATA_NOTIFICATION_FAILURE_IND           (70)
#define GTP_TRACE_SESSION_ACTIVATION                         (71)
#define GTP_TRACE_SESSION_DEACTIVATION                       (72)
#define GTP_STOP_PAGING_IND                                  (73)
#define GTP_CREATE_BEARER_REQ                                (95)
#define GTP_CREATE_BEARER_RSP                                (96)
#define GTP_UPDATE_BEARER_REQ                                (97)
#define GTP_UPDATE_BEARER_RSP                                (98)
#define GTP_DELETE_BEARER_REQ                                (99)
#define GTP_DELETE_BEARER_RSP                                (100)
#define GTP_DELETE_PDN_CONNECTION_SET_REQ                    (101)
#define GTP_DELETE_PDN_CONNECTION_SET_RSP                    (102)
#define GTP_IDENTIFICATION_REQ                               (128)
#define GTP_IDENTIFICATION_RSP                               (129)
#define GTP_CONTEXT_REQ                                      (130)
#define GTP_CONTEXT_RSP                                      (131)
#define GTP_CONTEXT_ACK                                      (132)
#define GTP_FORWARD_RELOCATION_REQ                           (133)
#define GTP_FORWARD_RELOCATION_RSP                           (134)
#define GTP_FORWARD_RELOCATION_COMPLETE_NTF                  (135)
#define GTP_FORWARD_RELOCATION_COMPLETE_ACK                  (136)
#define GTP_FORWARD_ACCESS_CONTEXT_NTF                       (137)
#define GTP_FORWARD_ACCESS_CONTEXT_ACK                       (138)
#define GTP_RELOCATION_CANCEL_REQ                            (139)
#define GTP_RELOCATION_CANCEL_RSP                            (140)
#define GTP_CONFIGURE_TRANSFER_TUNNEL                        (141)
#define GTP_DETACH_NTF                                       (149)
#define GTP_DETACH_ACK                                       (150)
#define GTP_CS_PAGING_INDICATION                             (151)
#define GTP_RAN_INFORMATION_RELAY                            (152)
#define GTP_ALERT_MME_NTF                                    (153)
#define GTP_ALERT_MME_ACK                                    (154)
#define GTP_UE_ACTIVITY_NTF                                  (155)
#define GTP_UE_ACTIVITY_ACK                                  (156)
#define GTP_CREATE_FORWARDING_TUNNEL_REQ                     (160)
#define GTP_CREATE_FORWARDING_TUNNEL_RSP                     (161)
#define GTP_SUSPEND_NTF                                      (162)
#define GTP_SUSPEND_ACK                                      (163)
#define GTP_RESUME_NTF                                       (164)
#define GTP_RESUME_ACK                                       (165)
#define GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ       (166)
#define GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP       (167)
#define GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ       (168)
#define GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP       (169)
#define GTP_RELEASE_ACCESS_BEARERS_REQ                       (170)
#define GTP_RELEASE_ACCESS_BEARERS_RSP                       (171)
#define GTP_DOWNLINK_DATA_NOTIFICATION                       (176)
#define GTP_DOWNLINK_DATA_NOTIFICATION_ACK                   (177)
#define GTP_RESERVED                                         (178)
#define GTP_PGW_RESTART_NOTIFICATION                         (179)
#define GTP_PGW_RESTART_NOTIFICATION_ACK                     (180)
#define GTP_UPDATE_PDN_CONNECTION_SET_REQ                    (200)
#define GTP_UPDATE_PDN_CONNECTION_SET_RSP                    (201)
#define GTP_MBMS_SESSION_START_REQ                           (231)
#define GTP_MBMS_SESSION_START_RSP                           (232)
#define GTP_MBMS_SESSION_UPDATE_REQ                          (233)
#define GTP_MBMS_SESSION_UPDATE_RSP                          (234)
#define GTP_MBMS_SESSION_STOP_REQ                            (235)
#define GTP_MBMS_SESSION_STOP_RSP                            (236)
#define GTP_MSG_END                                          (255)


/**
 * @brief  : Partial list of cause values from 3GPP TS 29.274, Table 8.4-1 containing
 *           values currently used by Control Plane.
 */
enum cause_value {
	GTPV2C_CAUSE_PGW_NOT_RESPONDING = 12,
	GTPV2C_CAUSE_REQUEST_ACCEPTED = 16,
	GTPV2C_CAUSE_REQUEST_ACCEPTED_PARTIALLY = 17,
	GTPV2C_CAUSE_NEW_PDN_TYPE_NETWORK_PREFERENCE = 18,
	GTPV2C_CAUSE_NEW_PDN_TYPE_SINGLE_ADDR_BEARER = 19,
	GTPV2C_CAUSE_CONTEXT_NOT_FOUND = 64,
	GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT = 65,
	GTPV2C_CAUSE_VERSION_NOT_SUPPORTED = 66,
	GTPV2C_CAUSE_INVALID_LENGTH = 67,
	GTPV2C_CAUSE_SERVICE_NOT_SUPPORTED = 68,
	GTPV2C_CAUSE_MANDATORY_IE_INCORRECT = 69,
	GTPV2C_CAUSE_MANDATORY_IE_MISSING = 70,
	GTPV2C_CAUSE_SYSTEM_FAILURE = 72,
	GTPV2C_CAUSE_NO_RESOURCES_AVAILABLE = 73,
	GTPV2C_CAUSE_MISSING_UNKNOWN_APN = 78,
	GTPV2C_CAUSE_PREFERRED_PDN_TYPE_UNSUPPORTED = 83,
	GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_OCCUPIED = 84,
	GTPV2C_CAUSE_NO_MEMORY_AVAILABLE = 91,
	GTPV2C_CAUSE_REQUEST_REJECTED = 94,
	GTPV2C_CAUSE_IMSI_NOT_KNOWN = 96,
	GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING = 100,
	GTPV2C_CAUSE_CONDITIONAL_IE_MISSING = 103,
	GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER = 107,
	GTPV2C_CAUSE_INVALID_PEER = 109
};

/**
 * @brief  : Returns cause string from code value as defined by 3gpp TS 29.274.
 * @param  : cause
 *           The cause coded value as specified by Table 8.4-1, TS 29.274.
 * @return : String describing cause code value.
 */
const char *
cause_str(enum cause_value cause);

/**
 * @brief  : Returns gtp message type string from type code value as defined by 3gpp TS
 *           29.274. Messages supported by this function may be incomplete.
 * @param  : type
 *           GTPv2 message type value as specified by table 6.1-1 in 3gpp TS 29.274.
 * @return : String describing GTPv2 message type.
 */
const char *
gtp_type_str(uint8_t type);


#ifdef __cplusplus
}
#endif
#endif /* _LIBGTPV2C_UTIL_H_ */
