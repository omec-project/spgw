/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __APN_APIS_H
#define __APN_APIS_H
#include <stdint.h>
#include "apn_struct.h"
/**
 * @brief  : This function takes the c-string argstr describing a apn by url, for example
 *           label1.label2.label3 and populates the apn structure according 3gpp 23.003
 *           clause 9.1
 * @param  : an_apn
 *           apn to be initialized
 * @param  : argstr
 *           c-string containing the apn label
 * @return : Returns nothing
 */
void
set_apn_name(apn_t *an_apn, char *argstr);

/**
 * @brief  : returns the apn strucutre of the apn referenced by create session message
 * @param  : apn_label
 *           apn_label within a create session message
 * @param  : apn_length
 *           the length as recorded by the apn information element
 * @return : the apn label configured for the CP
 */
apn_t *
get_apn(char *apn_label, uint16_t apn_length);

#endif
