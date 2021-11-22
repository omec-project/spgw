// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef CP_APP_H_
#define CP_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "gx_app/include/gx_struct.h"
#include "gx_app/include/gx.h"
#include "ue.h"
#include "sm_struct.h"
#include "pdn.h"
#include "bearer.h"

#ifdef __cplusplus
extern "C" {
#endif
/* VG1 Temp inlude remove this after handling of CSR on gx*/
#include "../libgtpv2c/include/gtp_messages.h"


#define SERVER_PATH "/tmp/sock_server"


/* IMSI length on gx */
#define STR_IMSI_LEN 16

/* MSISDN length on gx */
#define STR_MSISDN_LEN 12

extern int g_cp_sock;
extern int g_app_sock;

#pragma pack(1)


/**
 * @brief  : Returns Gx message type string from type code value as defined by 3gpp TS
 *           29.212. Messages supported by this function may be incomplete.
 * @param  : type
 *           Gx message type value as specified by seaction 5.6 in 3gpp TS 29.212.
 * @return : String describing Gx message type.
 */

const char *
gx_type_str(uint8_t type);

#pragma pack()

/**
 * @brief  : Handles processing of gx cca message
 * @param  : recv_buf, Received data from incoming message
 * @return : Returns nothing
 */
void
handle_gx_cca( unsigned char *recv_buf);

/**
 * @brief  : Handles incoming gx messages
 * @param  : No param
 * @return : Returns 0 in case of success , -1 otherwise
 */
void*
msg_handler_gx( void *);

/**
 * @brief  : Activate  interface for listening gx messages
 * @param  : No param
 * @return : Returns nothing
 */
void
start_cp_app( void );

/**
 * @brief  : Fill ccr request
 * @param  : ccr, structure to be filled
 * @param  : context, ue context data
 * @param  : ebi_index, array index of bearer
 * @param  : sess_id, session id
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
fill_ccr_request(GxCCR *ccr, ue_context_t *context,
		uint8_t ebi_index, char *sess_id);


/**
 * @brief  : Fill rat type ie
 * @param  : ccr_rat_type, parameter to be filled
 * @param  : csr_rat_type, input rat type
 * @return : Returns nothing
 */
void
fill_rat_type_ie( int32_t *ccr_rat_type, uint8_t csr_rat_type );

/**
 * @brief  : Fill user equipment information
 * @param  : ccr_user_eq_info, structure to be filled
 * @param  : csr_imei, imei value
 * @return : Returns nothing
 */
void
fill_user_equipment_info( GxUserEquipmentInfo *ccr_user_eq_info, uint64_t csr_imei );

/**
 * @brief  : Fill timezone information
 * @param  : ccr_tgpp_ms_timezone, structure to be filled
 * @param  : csr_ue_timezone, input data
 * @return : Returns nothing
 */
void
fill_3gpp_ue_timezone( Gx3gppMsTimezoneOctetString *ccr_tgpp_ms_timezone,
		gtp_ue_time_zone_ie_t csr_ue_timezone );

/**
 * @brief  : Fill subscription id information
 * @param  : subs_id, structure to be filled
 * @param  : imsi, imsi value
 * @param  : msisdn, msisdn value
 * @return : Returns nothing
 */
void
fill_subscription_id( GxSubscriptionIdList *subs_id, uint64_t imsi, uint64_t msisdn );

/**
 * @brief  : Process create bearer response and send raa message
 * @param  : sock, interface id to send raa
 * @return : Returns nothing
 */
void
process_create_bearer_resp_and_send_raa(proc_context_t *proc );

/**
 * @brief  : Convert binary data to string value
 * @param  : b_val, input binary data
 * @param  : s_val, parameter to store converted string
 * @param  : b_len, length of binary data
 * @param  : s_len, length of string
 * @return : Returns nothing
 */
void
bin_to_str(unsigned char *b_val, char *s_val, int b_len, int s_len);

int
gen_reauth_response(ue_context_t *context, uint8_t ebi_index);

int
gen_ccr_request(proc_context_t *proc_context, uint8_t ebi_index, create_sess_req_t *csr);

int
gen_ccru_request(pdn_connection_t *pdn, eps_bearer_t *bearer , mod_bearer_req_t *mb_req, uint8_t flag_check);

int
ccru_req_for_bear_termination(pdn_connection_t *pdn, eps_bearer_t *bearer);

int handle_cca_update_msg(msg_info_t **msg);
int handle_cca_initial_msg(msg_info_t **msg);
int handle_ccr_terminate_msg(msg_info_t **msg);
int handle_rar_msg(msg_info_t **msg);
void gx_msg_proc_failure(proc_context_t *proc_ctxt);

void
process_gx_msg(void *data, uint16_t event);

void*
out_handler_gx(void *data);

uint32_t
gx_send(int fd, char *buf, uint16_t len);

void init_gx(void);

/**
 * @brief  : Generate the CCR Session ID with combination of timestamp and call id.
 * @param  : sess id
 * @param  : call id
 * @return : Returns 0 on success
 */
int8_t
gen_sess_id_for_ccr(char *sess_id, uint32_t call_id);

/**
 * @brief  : Generate the CALL ID
 * @param  : void
 * @return : Returns call id  on success , 0 otherwise
 */
uint32_t
generate_call_id(void);

/**
 * @brief  : Retrieve Call ID from CCR Session ID
 * @param  : str represents CCR session ID
 * @param  : call_id , variable to store retrived call id
 * @return : Returns 0  on success , 0 otherwise
 */
int
retrieve_call_id(char *str, uint32_t *call_id);

/**
 * @brief  : Parse GX CCA message and fill ue context
 * @param  : cca holds data from gx cca message
 * @param  : _context , ue context to be filled
 * @return : Returns 0 on success, -1 otherwise
 */
int8_t
parse_gx_cca_msg(GxCCA *cca, pdn_connection_t **_pdn);

#ifdef __cplusplus
}
#endif
#endif /* CP_APP_H_ */
