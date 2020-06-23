// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _GTPV2C_ERROR_RSP_H_
#define _GTPV2C_ERROR_RSP_H_

#include "ue.h"
#include "sm_struct.h"
#include "gtpv2c_ie.h"

/**
 * @brief  : Maintains data to be filled in error response
 */
typedef struct err_rsp_info_t
{
	uint32_t sender_teid;
	uint32_t teid;
	uint32_t seq;
	uint8_t ebi_index;
	uint8_t offending;
	uint8_t bearer_count;
	uint8_t bearer_id[MAX_BEARERS];
}err_rsp_info;

/**
 * @brief  : Performs clean up task
 * @param  : ebi, bearer id
 * @param  : teid, teid value
 * @param  : imsi_val, imsi value
 * @param  : imsi_len, imsi length
 * @param  : seq, sequence
 * @return : Returns nothing
 */
int8_t clean_up_while_error(uint8_t ebi, uint32_t teid, uint64_t *imsi_val, uint16_t imsi_len, uint32_t seq );

/**
 * @brief  : Set and send error response in case of processing create session request
 * @param  : msg, holds information related to message caused error
 * @param  : cause_value, cause type of error
 * @param  : iface, interface on which response to be sent
 * @return : Returns nothing
 */
void cs_error_response(msg_info *msg, uint8_t cause_value, int iface);

/**
 * @brief  : Set and send error response in case of processing modify bearer request
 * @param  : msg, holds information related to message caused error
 * @param  : cause_value, cause type of error
 * @param  : iface, interface on which response to be sent
 * @return : Returns nothing
 */
void mbr_error_response(msg_info *msg, uint8_t cause_value, int iface);

/**
 * @brief  : Set and send error response in case of processing delete session request
 * @param  : msg, holds information related to message caused error
 * @param  : cause_value, cause type of error
 * @param  : iface, interface on which response to be sent
 * @return : Returns nothing
 */
void ds_error_response(msg_info *msg, uint8_t cause_value, int iface);

/**
 * @brief  : Gets information related to error and fills error response structure
 * @param  : msg, information related to message which caused error
 * @param  : err_rsp_info, structure to be filled
 * @param  : index, index of csr message in pending_csr array if parant message is csr
 * @return : Returns nothing
 */
void get_error_rsp_info(msg_info *msg, err_rsp_info *err_rsp_info);

/**
 * @brief  : Gets information related to error and fills error response structure
 *           similar to get_error_rsp_info, but only gets called from error handler function
 * @param  : msg, information related to message which caused error
 * @param  : t2, structure to be filled
 * @param  : index, index of csr message in pending_csr array if parant message is csr
 * @return : Returns nothing
 */
void get_info_filled(msg_info *msg, err_rsp_info *t2);

void ubr_error_response(msg_info *msg, uint8_t cause_value, int iface);


/**
 * @brief  : Send Version not supported response to peer node.
 * @param  : iface, interface.
 * @param  : seq, sequesnce number.
 * @return : Returns nothing
 */
void send_version_not_supported(int iface, uint32_t seq);

#endif
