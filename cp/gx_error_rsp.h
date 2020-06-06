#ifndef __GX_ERROR_RSP__
#define __GX_ERROR_RSP__
#include "./gx_app/include/gx.h"
#ifdef GX_BUILD
void gen_reauth_error_response(pdn_connection_t *pdn, int16_t error);
/**
 * @brief  : Preocess sending of ccr-t message if there is any error while procesing gx message
 * @param  : msg, information related to message which caused error
 * @param  : ebi, bearer id
 * @param  : teid, teid value
 * @return : Returns nothing
 */
void send_ccr_t_req(msg_info *msg, uint8_t ebi, uint32_t teid);
#endif /* GX_BUILD */
#endif
