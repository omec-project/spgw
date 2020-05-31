/*
 * Copyright (c) 2019 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PFCP_CP_ASSOC_H
#define PFCP_CP_ASSOC_H

#include "pfcp_messages.h"
#include "sm_struct.h"
/**
 * @brief  : This function processes pfcp associatiuon response
 * @param  : msg hold the data from pfcp associatiuon response
 * @param  : peer_addr denotes address of peer node
 * @return : Returns 0 in case of success else negative value
 */
uint8_t
process_pfcp_ass_resp(msg_info *msg, struct sockaddr_in *peer_addr);

/**
 * @brief  : This function adds csr to list of buffrered csrs
 * @param  : context hold information about ue context
 * @param  : upf_context hold information about upf context
 * @param  : ebi indicates eps bearer id
 * @return : Returns 0 in case of success else negative value
 */
int
buffer_csr_request(ue_context *context,
		upf_context_t *upf_context, uint8_t ebi);


/**
 * @brief  : fills default rule and qos values
 * @param  : pdn
 * @return : Returns nothing
 */
void
fill_rule_and_qos_inform_in_pdn(pdn_connection *pdn);


/**
 * @brief  : This function processes incoming create session request
 * @param  : teid
 * @param  : eps_bearer_id indicates eps bearer id
 * @return : Returns 0 in case of success else negative value
 */
int
process_create_sess_request(uint32_t teid, uint8_t eps_bearer_id);

#endif /* PFCP_ASSOC_H */
