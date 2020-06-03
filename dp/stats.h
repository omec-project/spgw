/*
 * Copyright 2020-present Open Networking Foundation
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _STATS_H_
#define _STATS_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of dataplane nic and pipeline stats.
 */
#include <rte_pipeline.h>

/**
 * @brief  : Function to display IN and OUT stats for all pipelines from struct.
 * @param  : No param
 * @return : Returns nothing
 */
void display_stats(void);

/**
 * @brief  : Function to get IN stats of a pipeline.
 * @param  : p, rte pipeline.
 * @param  : port_id, port id.
 * @param  : istats, struct rte_pipeline_port_in_stats param.
 * @return : Returns nothing
 */
void pip_istats(struct rte_pipeline *p, char *name, uint8_t port_id,
		struct rte_pipeline_port_in_stats *istats);

/**
 * @brief  : Function to fill IN stats for all pipelines in struct.
 * @param  : No param
 * @return : Returns nothing
 */
void pipeline_in_stats(void);

/**
 * @brief  : Function to get OUT stats of a pipeline.
 * @param  : p, rte pipeline.
 * @param  : name, pipeline name
 * @param  : port_id, port id.
 * @param  : ostats, struct rte_pipeline_port_out_stats param.
 * @return : Returns nothing
 */
void pip_ostats(struct rte_pipeline *p, char *name, uint8_t port_id,
		struct rte_pipeline_port_out_stats *ostats);

/**
 * @brief  : Function to fill OUT stats for all pipelines in struct.
 * @param  : No param
 * @return : Returns nothing
 */
void pipeline_out_stats(void);

/**
 * @brief  : Function to display NIC stats.
 * @param  : No param
 * @return : Returns nothing
 */
void nic_in_stats(void);

/**
 * @brief  : Function to display stats header parameters.
 * @param  : No param
 * @return : Returns nothing
 */
void print_headers(void);

/**
 * @brief  : Core to print the pipeline stats.
 * @param  : No param
 * @return : Returns nothing
 */
void epc_stats_core(void);

#endif /*_STATS_H_ */
