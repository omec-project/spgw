// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __GX_ERROR_RSP__
#define __GX_ERROR_RSP__
#include "./gx_app/include/gx.h"
#include "sm_struct.h"

#ifdef __cplusplus
extern "C" {
#endif
void gen_reauth_error_response(pdn_connection_t *pdn, int16_t error, uint16_t seq);
/**
 * @brief  : Preocess sending of ccr-t message if there is any error while procesing gx message
 * @param  : msg, information related to message which caused error
 * @param  : ebi, bearer id
 * @param  : teid, teid value
 * @return : Returns nothing
 */
void send_ccr_t_req(msg_info_t *msg, uint8_t ebi, uint32_t teid);

#ifdef __cplusplus
}
#endif
#endif
