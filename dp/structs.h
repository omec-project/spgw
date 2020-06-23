// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

/**
 * @brief  : Maintains pcc rule information
 */
struct pcc_id_precedence {
	uint32_t pcc_id;		/* pcc rule id */
	uint8_t precedence;		/* precedence */
	uint8_t gate_status;	/* gate status */
} __attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));

/**
 * @brief  : Maintains pcc rule information and entries
 */
struct filter_pcc_data {
	uint32_t entries;			/* number of elements in pcc_info */
	struct pcc_id_precedence *pcc_info;	/* pcc information */
} __attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));

//struct pdr_id_precedence {
//	uint8_t pdr_id;		/* pdr id */
//	uint8_t precedence;	/* precedence */
//	uint8_t gate_status;	/* gate status */
//} __attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));
//
//struct filter_sdf_data {
//	uint32_t entries;			/* number of elements in pcc_info */
//	struct pdr_id_precedence *pdr_info;	/* PDR information */
//} __attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));

enum filter_pcc_type {
	FILTER_SDF,		/* SDF filter type */
	FILTER_ADC,		/* ADC filter type */
};

#endif /*_STRUCTS_H_ */
