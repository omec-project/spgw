// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef IE_H
#define IE_H

/**
 * @file
 *
 * Information Element definitions and helper macros.
 *
 * Information Elements defined according to 3GPP TS 29.274, clause 8. IEs are
 * defined with bit-field structures for the x86_64 architecture and are not
 * cross compatible. Preprocessor definitions and enum typed values are defined
 * according to their respective 3GPP definitions.
 *
 */

#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "gtp_ies.h"
#include "gtp_messages.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GTP_VERSION_GTPV2C                                   (2)

/**
 * @brief  : GTPv2c Interface coded values for use in F-TEID IE, as defined in 3GPP
 *           TS 29.274, clause 8.22. These values are a subset of those defined in the TS,
 *           and represent only those used by the Control Plane (in addition to a couple
 *           that are not currently used).
 */
enum gtpv2c_interfaces {
	GTPV2C_IFTYPE_S1U_ENODEB_GTPU = 0,
	GTPV2C_IFTYPE_S1U_SGW_GTPU = 1,
	GTPV2C_IFTYPE_S12_RNC_GTPU = 2,
	GTPV2C_IFTYPE_S12_SGW_GTPU = 3,
	GTPV2C_IFTYPE_S5S8_SGW_GTPU = 4,
	GTPV2C_IFTYPE_S5S8_PGW_GTPU = 5,
	GTPV2C_IFTYPE_S5S8_SGW_GTPC = 6,
	GTPV2C_IFTYPE_S5S8_PGW_GTPC = 7,
	GTPV2C_IFTYPE_S5S8_SGW_PIMPv6 = 8,
	GTPV2C_IFTYPE_S5S8_PGW_PIMPv6 = 9,
	GTPV2C_IFTYPE_S11_MME_GTPC = 10,
	GTPV2C_IFTYPE_S11S4_SGW_GTPC = 11,
	GTPV2C_IFTYPE_S11U_SGW_GTPU = 39
};

#define IPV4_IE_LENGTH                                        (4)
#define IPV6_IE_LENGTH                                        (16)


#define PDN_IP_TYPE_IPV4                                      (1)
#define PDN_IP_TYPE_IPV6                                      (2)
#define PDN_IP_TYPE_IPV4V6                                    (3)

#pragma pack(1)


/*TODO: Added this enum (Nikhil) */

enum ie_instance {
        IE_INSTANCE_ZERO = 0,
        IE_INSTANCE_ONE = 1,
        IE_INSTANCE_TWO = 2,
        IE_INSTANCE_THREE = 3,
        IE_INSTANCE_FOUR = 4,
        IE_INSTANCE_FIVE = 5,
        IE_INSTANCE_SIX = 6
};

//

/**
 * Information Element structure as defined by 3GPP TS 29.274, clause 8.2.1, as
 * shown by Figure 8.2-1. IE specific data or content of grouped IE follows
 * directly in memory from this structure. IE specific data is defined by
 * structures ending with the _ie postfix in this file.
 *
 * IE Type extension is not currently supported.
 */
/**
 * @brief  : Maintains information about gtpv2c ie
 */
typedef struct gtpv2c_ie_t {
	uint8_t type;
	uint16_t length;
	uint8_t instance :4;
	uint8_t spare :4;
} gtpv2c_ie;

/**
 * IE specific data for Cause as defined by 3GPP TS 29.274, clause 8.4 for the
 * IE type value 2.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of cause ie
 */
struct cause_ie_hdr_t {
	uint8_t cause_value;
	uint8_t cause_source :1;
	uint8_t bearer_context_error :1;
	uint8_t pdn_connection_error :1;
	uint8_t spare_0 :5;
};

typedef struct cause_ie {
	struct cause_ie_hdr_t cause_ie_hdr;
	/* if gtpv2c_ie->length=2, the following fields are not active,
	 *  otherwise gtpv2c_ie->length=6
	 */
	uint8_t offending_ie_type;
	uint16_t offending_ie_length;
	uint8_t instance :4;
	uint8_t spare_1 :4;
} cause_ie;

/**
 * IE specific data for Aggregate Maximum Bit Rate (AMBR) as defined by
 * 3GPP TS 29.274, clause 8.7 for the IE type value 72.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of ambr ie
 */
typedef struct ambr_ie {
	uint32_t ambr_uplink;
	uint32_t ambr_downlink;
} ambr_ie;

/**
 * IE specific data for EPS Bearer ID (EBI) as defined by
 * 3GPP TS 29.274, clause 8.8 for the IE type value 73.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of emp bearer id ie
 */
typedef struct eps_bearer_id_ie {
	uint8_t ebi :4;
	uint8_t spare :4;
} eps_bearer_id_ie;


/**
 * IE specific data for PDN Address Allocation (PAA) as defined by
 * 3GPP TS 29.274, clause 8.14 for the IE type value 79.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of paa ie
 */
struct paa_ie_hdr_t {
	uint8_t pdn_type :3;
	uint8_t spare :5;
};

struct ipv6 {
	uint8_t prefix_length;
	struct in6_addr ipv6;
};

struct paa_ipv4v6 {
	uint8_t prefix_length;
	struct in6_addr ipv6;
	struct in_addr ipv4;
};

typedef struct paa_ie {
	struct paa_ie_hdr_t paa_ie_hdr;
	union ip_type_union_t {
		struct in_addr ipv4;
		struct ipv6 ipv6;
		struct paa_ipv4v6 paa_ipv4v6;
	} ip_type_union;
} paa_ie;

/**
 * IE specific data segment for Quality of Service (QoS).
 *
 * Definition used by bearer_qos_ie and flow_qos_ie.
 */
/**
 * @brief  : Maintains information  about Quality of Service data segment
 */
typedef struct qos_segment_t {
	/** QoS class identifier - defined by 3GPP TS 23.203 */
	uint8_t qci;

	/** Uplink Maximum Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t ul_mbr;
	/** Downlink Maximum Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t dl_mbr;
	/** Uplink Guaranteed Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t ul_gbr;
	/** Downlink Guaranteed Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t dl_gbr;
} qos_segment;

/**
 * IE specific data for Bearer Quality of Service (QoS) as defined by
 * 3GPP TS 29.274, clause 8.15 for the IE type value 80.
 */
/**
 * @brief  : Maintains information of ar priority ie
 */
typedef struct ar_priority_ie_t {
	uint8_t preemption_vulnerability :1;
	uint8_t spare1 :1;
	uint8_t priority_level :4;
	uint8_t preemption_capability :1;
	uint8_t spare2 :1;
} ar_priority_ie;

/**
 * IE specific data for Bearer Quality of Service (QoS) as defined by
 * 3GPP TS 29.274, clause 8.15 for the IE type value 80.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of bearer qos ie
 */
typedef struct bearer_qos_ie {
	/** QoS class identifier - defined by 3GPP TS 23.203 */
	uint8_t qci;

	/** Uplink Maximum Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t ul_mbr;

	/** Downlink Maximum Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t dl_mbr;

	/** Uplink Guaranteed Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t ul_gbr;

	/** Downlink Guaranteed Bit Rate in kilobits (1000bps) - for non-GBR
	 * Bearers this field to be set to zero*/
	uint64_t dl_gbr;

	/* First Byte: Allocation/Retention Priority (ARP) */
	ar_priority_ie arp;

	/*VS: TODO: Remove this declaration of segment onces get code stable with updated structure */
	/* qos_segment qos; */
} bearer_qos_ie;

#define BEARER_QOS_IE_PREMPTION_DISABLED (1)
#define BEARER_QOS_IE_PREMPTION_ENABLED  (0)

/* IEI = IE_FLOW_QOS = 81 */
/**
 * IE specific data for Flow Quality of Service (QoS) as defined by
 * 3GPP TS 29.274, clause 8.16 for the IE type value 81.
 */
/**
 * @brief  : Maintains information of flow qos ie
 */
typedef struct flow_qos_ie_t {
	qos_segment qos;
} flow_qos_ie;

/**
 * IE specific data for Bearer Traffic Flow Template (TFT) as defined by
 * 3GPP TS 24.008, clause 10.5.6.12 for the IE type value 84.
 */
/**
 * @brief  : Maintains information of bearer tft ie
 */
typedef struct bearer_tft_ie_t {
	/* For the TFT_OP_DELETE_EXISTING operation and TFT_OP_NO_OP,
	 * num_pkt_filters shall be 0'' */
	uint8_t num_pkt_filters :4;
	uint8_t parameter_list :1; /* Refereed to e-bit in spec */
	uint8_t tft_op_code :3;
} bearer_tft_ie;

/* for use in bearer_tft.tft_op_code */
#define TFT_OP_CREATE_NEW                (1)
#define TFT_OP_DELETE_EXISTING           (2)
#define TFT_OP_ADD_FILTER_EXISTING       (3)
#define TFT_OP_REPLACE_FILTER_EXISTING   (4)
#define TFT_OP_DELETE_FILTER_EXISTING    (5)
#define TFT_OP_NO_OP                     (6)

#define TFT_DOWNLINK							(16)
#define TFT_UPLINK								(32)
#define TFT_BIDIRECTIONAL						(48)

#define TFT_CREATE_NEW							(32)
#define TFT_REPLACE_FILTER_EXISTING				(128)
#define TFT_PROTO_IDENTIFIER_NEXT_HEADER_TYPE	(48)
#define TFT_SINGLE_SRC_PORT_TYPE				(64)
#define TFT_SINGLE_REMOTE_PORT_TYPE				(80)
#define TFT_IPV4_REMOTE_ADDR_TYPE				(16)
#define TFT_IPV4_SRC_ADDR_TYPE					(17)
//#define TFT_SINGLE_DEST_PORT_TYPE				(64)
#define TFT_DEST_PORT_RANGE_TYPE				(65)
//#define TFT_SINGLE_SRC_PORT_TYPE				(80)
#define TFT_SRC_PORT_RANGE_TYPE					(81)


/**
 * Packet filter list when the TFT operation is TFT_OP_DELETE_EXISTING
 * From Figure 10.5.144a/3GPP TS 24.008
 */
/**
 * @brief  : Maintains information of packet filter for delete operation
 */
typedef struct delete_pkt_filter_t {
	uint8_t pkt_filter_id :4;
	uint8_t spare :4;
} delete_pkt_filter;

struct ipv4_t {
	struct in_addr ipv4;
	struct in_addr mask;
	uint8_t next_component;
};

struct port_t {
	uint16_t port;
	uint8_t next_component;
};

struct port_range_t {
	uint16_t port_low;
	uint16_t port_high;
	uint8_t next_component;
};

struct proto_t {
	uint8_t proto;
	uint8_t next_component;
};

/**
 * Packet filter component from Table 10.5.162/3GPP TS 24.008
 */
/**
 * @brief  : Maintains information packet filter component
 */
typedef struct packet_filter_component_t {
	uint8_t type;
	union type_union_u {
		struct ipv4_t ipv4;
		struct port_t port;
		struct port_range_t port_range;
		struct proto_t proto;
	} type_union;
} packet_filter_component;

/**
 * Packet filter list from Figure 10.5.144b/3GPP TS 24.008 for use when TFT
 * operation is TFT_OP_CREATE_NEW
 *
 * For future use with operations TFT_OP_ADD_FILTER_EXISTING and
 * TFT_OP_REPLACE_FILTER_EXISTING - not currently supported by Control Plane
 */
/**
 * @brief  : Maintains information packet filter to be created
 */
typedef struct create_pkt_filter_t {
	uint8_t pkt_filter_id :4;
	uint8_t direction :2;
	uint8_t spare :2;
	uint8_t precedence;
	uint8_t pkt_filter_length;
} create_pkt_filter;

/* for use in create_pkt_filter.direction */
#define TFT_DIRECTION_NONE      	 (0)
#define TFT_DIRECTION_DOWNLINK_ONLY      (1)
#define TFT_DIRECTION_UPLINK_ONLY        (2)
#define TFT_DIRECTION_BIDIRECTIONAL      (3)

/**
 * @brief  : Flow Status AVP describes whether the IP flow(s) are enabled or disabled.
 */
enum flow_status {
        FL_ENABLED_UPLINK = 0,
        FL_ENABLED_DOWNLINK = 1,
        FL_ENABLED = 2,
        FL_DISABLED = 3,
        FL_REMOVED = 4
};

/* Packet filter component type identifiers. Following create_pkt_filter,
 * num_pkt_filters packet filter contents consist of a pair consisting of
 * (component type id, value) where value length is dependent upon id
 *
 * Note: The term local refers to the MS (UE)
 * and the term remote refers to an external network entity */
#define IPV4_REMOTE_ADDRESS         0x10             /* 0b00010000 */
#define IPV4_LOCAL_ADDRESS          0x11             /* 0b00010001 */
#define PROTOCOL_ID_NEXT_HEADER     0x30             /* 0b00110000 */
#define SINGLE_LOCAL_PORT           0x40             /* 0b01000000 */
#define LOCAL_PORT_RANGE            0x41             /* 0b01000001 */
#define SINGLE_REMOTE_PORT          0x50             /* 0b01010000 */
#define REMOTE_PORT_RANGE           0x51             /* 0b01010001 */
/* Unsupported packet filter components
 * #define IPV6_REMOTE_ADDRESS        0b00100000
 * #define IPV6_REMOTE_ADDRESS_PREFIX 0b00100001
 * #define IPV6_LOCAL_ADDRESS_PREFIX  0b00100011
 * #define SECURITY_PARAMETER_INDEX   0b01100000
 * #define TRAFFIC_CLASS_TOS          0b01110000
 * #define FLOW_LABEL_TYPE            0b10000000
 */

static const uint8_t PACKET_FILTER_COMPONENT_SIZE[REMOTE_PORT_RANGE + 1] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x0 to 0xf */
    sizeof(struct ipv4_t),sizeof(struct ipv4_t),0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10 to 0x1f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20 to 0x2f */
    sizeof(struct proto_t),0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x30 to 0x3f */
    sizeof(struct port_t), sizeof(struct port_range_t),0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x40 to 0x4f */
    sizeof(struct port_t), sizeof(struct port_range_t)/* 0x50 0x51 */
};

#define AUTHORIZATION_TOKEN              (1)
#define FLOW_IDENTIFIER                  (2)
#define PACKET_FILTER_IDENTIFIER         (3)

/**
 * IE specific data for Traffic Aggregation Description (TAD) for IE type 85.
 *
 * TFT is reused for TAD, where use of parameters such as packet filter
 * identifiers may differ. See NOTE 3 in 3GPP TS 24.008, clause 10.5.6.12, as
 * well as 3GPP TS 24.301.
 */
typedef struct bearer_tft_ie_t traffic_aggregation_description;


/**
 * IE specific data for Fully qualified Tunnel Endpoint ID (F-TEID) as defined
 * by 3GPP TS 29.274, clause 8.22 for the IE type value 87.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of fteid ie
 */
struct fteid_ie_hdr_t {
	uint8_t interface_type :6;
	uint8_t v6 :1;
	uint8_t v4 :1;
	uint32_t teid_or_gre;
};

struct ipv4v6 {
	struct in_addr ipv4;
	struct in6_addr ipv6;
};

typedef struct fteid_ie {
	struct fteid_ie_hdr_t fteid_ie_hdr;
	union ip_t {
		struct in_addr ipv4;
		struct in6_addr ipv6;
		struct ipv4v6 ipv4v6;
	} ip_u;
} fteid_ie;

/**
 * IE specific data for Delay Value as definedby 3GPP TS 29.274, clause 8.27
 * for the IE type value 92.
 */
/**
 * @brief  : Maintains information delay ie
 */
typedef struct delay_ie_t {
	uint8_t delay_value;
} delay_ie;

/**
 * IE specific data for Charging Characteristics as defined by
 * 3GPP TS 29.274, clause 8.30 for the IE type value 95.
 *
 * Charging characteristics information element is defined in 3GPP TS 32.251
 *
 * For the encoding of this information element see 3GPP TS 32.298
 */
/**
 * @brief  : Maintains information of charging characteristics ie
 */
typedef struct charging_characteristics_ie_t {
	uint8_t b0 :1;
	uint8_t b1 :1;
	uint8_t b2 :1;
	uint8_t b3 :1;
	uint8_t b4 :1;
	uint8_t b5 :1;
	uint8_t b6 :1;
	uint8_t b7 :1;
	uint8_t b8 :1;
	uint8_t b9 :1;
	uint8_t b10 :1;
	uint8_t b11 :1;
	uint8_t b12 :1;
	uint8_t b13 :1;
	uint8_t b14 :1;
	uint8_t b15 :1;
} charging_characteristics_ie;


/**
 * IE specific data for Packet Data Network (PDN) Type as defined by
 * 3GPP TS 29.274, clause 8.34 for the IE type value 99.
 * TODO: REMOVE_DUPLICATE_USE_LIBGTPV2C
 * Remove following structure and use structure defined in
 * libgtpv2c header file.
 * Following structure has dependency on functionality
 * which can not to be tested now.
 */
/**
 * @brief  : Maintains information of pdn type ie
 */
typedef struct pdn_type_ie {
	uint8_t ipv4 :1;
	uint8_t ipv6 :1;
	uint8_t spare :6;
} pdn_type_ie;

/**
 * @brief  : Maintains information of fqdn type ie
 */
typedef struct fqdn_type_ie {
	uint8_t fqdn[256];
} fqdn_type_ie;

#pragma pack()

#define IE_TYPE_PTR_FROM_GTPV2C_IE(ptr_type, gtpv2c_ie_ptr) \
		((ptr_type *)((gtpv2c_ie_ptr) + 1))

/* returns offset of APN encoding according to 3gpp 23.003 9.1*/
#define APN_PTR_FROM_APN_IE(gtpv2c_ie_ptr)    \
		IE_TYPE_PTR_FROM_GTPV2C_IE(char, gtpv2c_ie_ptr)

#ifdef __cplusplus
}
#endif
#endif /* IE_H */

