// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

/***************************CP-DP-Structures**************************/

#ifndef _CP_DP_API_H_
#define _CP_DP_API_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes to describe CP DP APIs.
 */
#include <stdint.h>
#include <time.h>
/**
 * IPv6 address length
 */
#define IPV6_ADDR_LEN 16

/**
 * Maximum CDR services.
 */
#define MAX_SERVICE 1
/**
 * Maximum PCC rules per session.
 */
#define MAX_PCC_RULES 12
/**
 * Maximum PCC rules per session.
 */
#define MAX_ADC_RULES 16


/**
 * Maximum number of SDF indices that can be referred in PCC rule.
 * Max length of the sdf rules string that will be recieved as part of add
 * pcc entry from FPC. String is list of SDF indices.
 * TODO: Revisit this count
 */
#define MAX_SDF_IDX_COUNT 16
#define MAX_SDF_STR_LEN 4096

/**
 * Maximum buffer/name length
 */
#define MAX_LEN 128
/**
 * @brief  : Defines number of entries in local database.
 *
 * Recommended local table size to remain within L2 cache: 64000 entries.
 * See README for detailed calculations.
 */
#define LDB_ENTRIES_DEFAULT (1024 * 1024 * 4)

#define DEFAULT_DN_NUM 512

/**
 * Gate closed
 */
#define CLOSE 1
/**
 * Gate opened
 */
#define OPEN 0

/**
 * Maximum rating groups per bearer session.
 */
#define MAX_RATING_GRP 6

/**
 * Get pdn from context and bearer id.
 */
#define GET_PDN(x, i) (x->eps_bearers[i]->pdn)
/**
 * default bearer session.
 */
#define DEFAULT_BEARER 5

/**
 * get UE session id
 */
#define UE_SESS_ID(x) ((x & 0xffffffff) >> 4)

/**
 * get bearer id
 */
#define UE_BEAR_ID(x) (x & 0xf)
/**
 * set session id from the combination of
 * unique UE id and Bearer id
 */
#define SESS_ID(ue_id, br_id) (((uint64_t)(ue_id) << 4) | (0xf & (br_id)))

/**
 * MAX DNS Sponsor ID name lenth
 */
#define MAX_DNS_SPON_ID_LEN 16
/**
 * @brief  : Select IPv4 or IPv6.
 */
enum iptype {
	IPTYPE_IPV4 = 0,     /* IPv4. */
	IPTYPE_IPV6,        /* IPv6. */
};

/**
 * @brief  : SDF Rule type field.
 */
enum rule_type {
	RULE_STRING = 0,
	FIVE_TUPLE,
};

/**
 * @brief  : Packet action  field.
 */
enum sess_pkt_action {
	ACTION_NONE = 0,
	ACTION_DROP,
	ACTION_FORWARD,
	ACTION_BUFFER,
	ACTION_NOTIFY_CP,
	ACTION_DUPLICATE,
};

/**
 * @brief  : IPv4 or IPv6 address configuration structure.
 */
struct ip_addr {
	enum iptype iptype;			/* IP type: IPv4 or IPv6. */
	union {
		uint32_t ipv4_addr;		/* IPv4 address*/
		uint8_t  ipv6_addr[IPV6_ADDR_LEN]; /* IPv6 address*/
	} u;
} __attribute__((packed));

/**
 * @brief  : IPv4 5 tuple rule configuration structure.
 */
struct ipv4_5tuple_rule {
	uint32_t ip_src;	/* Src IP address*/
	uint32_t ip_dst;	/* Dst IP address*/
	uint32_t src_mask;	/* Src Mask*/
	uint32_t dst_mask;	/* Dst Mask*/
	uint16_t sport_s;	/* Range start Src Port */
	uint16_t sport_e;	/* Range end Src Port */
	uint16_t dport_s;	/* Range start Dst Port */
	uint16_t dport_e;	/* Range end Dst Port */
	uint8_t  proto_s;	/* Range start Protocol*/
	uint8_t  proto_e;	/* Range end Protocol*/
} __attribute__((packed));

/**
 * @brief  : IPv6 5 tuple rule configuration structure.
 */
struct ipv6_5tuple_rule {
	uint8_t  ip_src[IPV6_ADDR_LEN];	/* Src IP address*/
	uint8_t  ip_dst[IPV6_ADDR_LEN];	/* Dst IP address*/
	uint32_t src_mask;	/* Src Mask*/
	uint32_t dst_mask;	/* Dst Mask*/
	uint16_t sport_s;	/* Range start Src Port */
	uint16_t sport_e;	/* Range end Src Port */
	uint16_t dport_s;	/* Range start Dst Port */
	uint16_t dport_e;	/* Range end Dst Port */
	uint8_t  proto_s;	/* Range start Protocol*/
	uint8_t  proto_e;	/* Range end Protocol*/
} __attribute__((packed));

/**
 * @brief  : 5 tuple rule configuration structure.
 */
struct  five_tuple_rule {
	enum iptype iptype; /* IP type: IPv4 or IPv6. */
	union {
		struct ipv4_5tuple_rule ipv4;
		struct ipv6_5tuple_rule ipv6;
	} u;
} __attribute__((packed));

/**
 * @brief  : Packet filter configuration structure.
 */
struct service_data_list {
	uint32_t	service[MAX_SERVICE];	/* list of service id*/
	/* TODO: add other members*/
} ;

/**
 * @brief  : SDF Packet filter configuration structure.
 */
struct pkt_filter {
	uint32_t pcc_rule_id;				/* PCC rule id*/
	union {
		char rule_str[MAX_LEN];		/* string of rule, please refer
						 * cp/main.c for example
						 * TODO: rule should be in struct five_tuple_rule*/
		struct five_tuple_rule rule_5tp;	/* 5 Tuple rule.
							 * This field is currently not used*/
	} u;
	enum rule_type sel_rule_type;
} __attribute__((packed));

/**
 * @brief  :  DNS selector type.
 */
enum selector_type {
	DOMAIN_NAME = 0,		/* Domain name. */
	DOMAIN_IP_ADDR,			/* Domain IP address */
	DOMAIN_IP_ADDR_PREFIX,	/* Domain IP prefix */
	DOMAIN_NONE
};

/**
 * @brief  : IPv4 or IPv6 address configuration structure.
 */
struct ip_prefix {
	struct ip_addr ip_addr;	/* IP address*/
	uint16_t prefix;		/* Prefix*/
} __attribute__((packed));

/**
 * @brief  : Redirect configuration structure.
 */
struct  redirect_info {
	uint32_t info;
};

/**
 * @brief  : QoS parameters structure for DP
 */
struct qos_info {
	uint16_t ul_mtr_profile_index; /* index 0 to skip */
	uint16_t dl_mtr_profile_index; /* index 0 to skip */
	uint16_t ul_gbr_profile_index; /* index 0 to skip */
	uint16_t dl_gbr_profile_index; /* index 0 to skip */
	uint8_t qci;    /*QoS Class Identifier*/
	uint8_t arp;    /*Allocation and Retention Priority*/
};

/**
 * @brief  : Application Detection and Control Rule Filter config structure.
 */
struct adc_rules {
	enum selector_type sel_type;	/* domain name, IP addr
					 * or IP addr prefix*/
	union {
		char domain_name[MAX_LEN];	/* Domain name. */
		struct ip_addr domain_ip;	/* Domain IP address */
		struct ip_prefix domain_prefix;	/* Domain IP prefix */
	} u;
	uint32_t rule_id;				/* Rule ID*/
} __attribute__((packed));


/**
 * @brief  : Metering Methods.
 */
enum mtr_mthds {
	SRTCM_COLOR_BLIND = 0,	/* Single Rate Three Color Marker - Color blind*/
	SRTCM_COLOR_AWARE,     /* Single Rate Three Color Marker - Color aware*/
	TRTCM_COLOR_BLIND,	/* Two Rate Three Color Marker - Color blind*/
	TRTCM_COLOR_AWARE,	/* Two Rate Three Color Marker - Color aware*/
};

/**
 * @brief  : Meter profile parameters
 */
struct mtr_params {
	/* Committed Information Rate (CIR). Measured in bytes per second.*/
	uint64_t cir;
	/* Committed Burst Size (CBS).  Measured in bytes.*/
	uint64_t cbs;
	/* Excess Burst Size (EBS).  Measured in bytes.*/
	uint64_t ebs;
	/* TODO: add TRTCM params */
} __attribute__((packed));

/**
 * @brief  : Meter Profile entry config structure.
 */
struct mtr_entry {
	uint16_t mtr_profile_index;	/* Meter profile index*/
	struct mtr_params mtr_param;	/* Meter params*/
	uint8_t  metering_method;	/* Metering Methods
								 * -fwd, srtcm, trtcm*/
} __attribute__((packed));

/**
 * @brief  : Direction on which the session is applicable.
 */
enum sess_direction {
	SESS_UPLINK = 0,/* rule applicable for Uplink. */
	SESS_DOWNLINK,	/* rule applicable for Downlink*/
};

/**
 * @brief  : UpLink S1u interface config structure.
 */
struct ul_s1_info {
	uint32_t sgw_teid;		/* SGW teid*/
	uint32_t s5s8_pgw_teid; 	/* PGW teid */
	struct ip_addr enb_addr;	/* eNodeB address*/
	struct ip_addr sgw_addr;	/* Serving Gateway address*/
	struct ip_addr s5s8_pgwu_addr;	/* S5S8_PGWU address*/
} __attribute__((packed));

/**
 * @brief  : DownLink S1u interface config structure.
 */
struct dl_s1_info {
	uint32_t enb_teid;		/* eNodeB teid*/
	struct ip_addr enb_addr;	/* eNodeB address*/
	struct ip_addr sgw_addr;	/* Serving Gateway address*/
	struct ip_addr s5s8_sgwu_addr;	/* S5S8_SGWU address*/
} __attribute__((packed));

/**
 * @brief  : Policy and Charging Control structure for DP
 */
struct pcc_rules {
	uint32_t rule_id;			/* Rule ID*/
	char rule_name[MAX_LEN];		/* Rule Name*/
	uint32_t rating_group;			/* Group rating*/
	uint32_t service_id;			/* identifier for the service or the service component
						 * the service data flow relates to.*/
	uint8_t rule_status;			/* Rule Status*/
	uint8_t  gate_status;			/* gate status indicates whether the service data flow,
						 * detected by the service data flow filter(s),
						 * may pass or shall be discarded*/
	uint8_t  session_cont;			/* Total Session Count*/
	uint8_t  report_level;			/* Level of report*/
	uint32_t  monitoring_key;		/* key to identify monitor control instance that shall
						 * be used for usage monitoring control of the service
						 * data flows controlled*/
	char sponsor_id[MAX_LEN];		/* to identify the 3rd party organization (the
						 * sponsor) willing to pay for the operator's charge*/
	struct  redirect_info redirect_info;	/* Redirect  info*/
	uint32_t precedence;			/* Precedence*/
	uint64_t drop_pkt_count;		/* Drop count*/
	uint32_t adc_idx; //GCC_Security flag
	uint32_t sdf_idx_cnt;
	uint32_t sdf_idx[MAX_SDF_IDX_COUNT];
	struct qos_info qos;			/* QoS Parameters*/
	uint8_t  charging_mode;			/* online and offline charging*/
	uint8_t  metering_method;		/* Metering Methods
						 * -fwd, srtcm, trtcm*/
	uint8_t  mute_notify;			/* Mute on/off*/
} __attribute__((packed));

/**
 * @brief  : Maintains cdr details
 */
struct cdr {
	uint64_t bytes;
	uint64_t pkt_count;
};
/**
 * @brief  : Volume based Charging
 */
struct chrg_data_vol {
	struct cdr ul_cdr;		/* Uplink cdr*/
	struct cdr dl_cdr;		/* Downlink cdr*/
	struct cdr ul_drop;		/* Uplink dropped cdr*/
	struct cdr dl_drop;		/* Downlink dropped cdr*/
};

/**
 * @brief  : Rating group index mapping Data structure.
 */
struct rating_group_index_map {
	uint32_t rg_val;				/* Rating group*/
	uint8_t rg_idx;					/* Rating group index*/
};

/**
 * @brief  : IP-CAN Bearer Charging Data Records
 */
struct ipcan_dp_bearer_cdr {
	uint32_t charging_id;			/* Bearer Charging id*/
	uint32_t pdn_conn_charging_id;		/* PDN connection charging id*/
	struct tm record_open_time;		/* Record time*/
	uint64_t duration_time;			/* duration (sec)*/
	uint8_t	record_closure_cause;		/* Record closure cause*/
	uint64_t record_seq_number;		/* Sequence no.*/
	uint8_t charging_behavior_index; 	/* Charging index*/
	uint32_t service_id;			/* to identify the service
						 * or the service component
						 * the bearer relates to*/
	char sponsor_id[MAX_DNS_SPON_ID_LEN];	/* to identify the 3rd party organization (the
						 * sponsor) willing to pay for the operator's charge*/
	struct service_data_list service_data_list; /* List of service*/
	uint32_t rating_group;			/* rating group of this bearer*/
	uint64_t vol_threshold;			/* volume threshold in MBytes*/
	struct chrg_data_vol data_vol;		/* charing per UE by volume*/
	uint32_t charging_rule_id;			/* Charging Rule ID*/
} __attribute__((packed));


/**
 * @brief  : Bearer Session information structure
 */
struct session_info {
	struct ip_addr ue_addr;						/* UE ip address*/
	struct ul_s1_info ul_s1_info;					/* UpLink S1u info*/
	struct dl_s1_info dl_s1_info;					/* DownLink S1u info*/
	uint8_t bearer_id;						/* Bearer ID*/

	/* PCC rules related params*/
	uint32_t num_ul_pcc_rules;					/* No. of UL PCC rule*/
	uint32_t ul_pcc_rule_id[MAX_PCC_RULES]; 			/* PCC rule id supported in UL*/
	uint32_t num_dl_pcc_rules;					/* No. of PCC rule*/
	uint32_t dl_pcc_rule_id[MAX_PCC_RULES];				/* PCC rule id*/

	/* ADC rules related params*/
	uint32_t num_adc_rules;					/* No. of ADC rule*/
	uint32_t adc_rule_id[MAX_ADC_RULES]; 			/* List of ADC rule id*/

	/* Charging Data Records*/
	struct ipcan_dp_bearer_cdr ipcan_dp_bearer_cdr;			/* Charging Data Records*/
	uint32_t client_id;

	uint64_t sess_id;						/* session id of this bearer
									 * last 4 bits of sess_id
									 * maps to bearer id*/
	uint64_t cp_sess_id;
	uint32_t service_id;						/* Type of service given
									 * given to this session like
									 * Internet, Management, CIPA etc
									 */
	uint32_t ul_apn_mtr_idx;		/* UL APN meter profile index*/
	uint32_t dl_apn_mtr_idx;		/* DL APN meter profile index*/
	enum sess_pkt_action action;
} __attribute__((packed));


/**
 * @brief  : DataPlane identifier information structure.
 */
struct dp_id {
	uint64_t id;			/* table identifier.*/
	char name[MAX_LEN];		/* name string of identifier*/
} __attribute__((packed));

/**
 * @brief  : Type of CDR record to be flushed.
 */
enum cdr_type {
    CDR_TYPE_BEARER,
    CDR_TYPE_ADC,
    CDR_TYPE_FLOW,
    CDR_TYPE_RG,
    CDR_TYPE_ALL
};
/**
 * @brief  : Structure to flush different types of UE CDRs into file.
 */
struct msg_ue_cdr {
    uint64_t session_id;    /* session id of the bearer, this field
							 * should have same value as set in sess_id
							 * in struct session_info during session create.*/
    enum cdr_type type;     /* type of cdrs to flush. It can be
							 * either Bearer, ADC, FLOW, Rating group
							 * or all. Please refer enum cdr_type for values*/
    uint8_t action;         /* 0 to append and 1 to clear old logs and
							 * write new logs into cdr log file.*/
} __attribute__((packed));


#define MAX_NB_DPN	8  /* Note: MAX_NB_DPN <= 8 */

#endif /* _CP_DP_API_H_ */
