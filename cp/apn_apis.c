// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <assert.h>
#include <string.h>
#include <rte_malloc.h>
#include <rte_debug.h>
#include <rte_lcore.h>
#include <rte_errno.h>
#include "apn_apis.h"

#define MAX_NB_DPN 8
apn_t apn_list[MAX_NB_DPN];

void
set_apn_name(apn_t *an_apn, char *argstr)
{
    static uint8_t configured_apn;
    assert(configured_apn != MAX_NB_DPN); 

	if (argstr == NULL)
		rte_panic("APN Name argument not set\n");
	an_apn->apn_name_length = strlen(argstr) + 1;
	an_apn->apn_name_label = rte_zmalloc_socket(NULL, an_apn->apn_name_length,
	    RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (an_apn->apn_name_label == NULL)
		rte_panic("Failure to allocate apn_name_label buffer: "
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__,
				__LINE__);
	/* Don't copy NULL termination */
	strncpy(an_apn->apn_name_label + 1, argstr, strlen(argstr));
	char *ptr, *size;
	size = an_apn->apn_name_label;
	*size = 1;
	ptr = an_apn->apn_name_label + strlen(argstr) - 1;
	do {
		if (ptr == size)
			break;
		if (*ptr == '.') {
			*ptr = *size;
			*size = 0;
		} else {
			(*size)++;
		}
		--ptr;
	} while (ptr != an_apn->apn_name_label);
    memcpy(&apn_list[configured_apn], an_apn, sizeof(*an_apn));  
    configured_apn++;
}

apn_t *
get_apn(char *apn_label, uint16_t apn_length)
{
	int i;
	printf("%s %d - APN %s length %d \n",__FUNCTION__,__LINE__, apn_label, apn_length);
	for (i = 0; i < MAX_NB_DPN; i++)   {
	    printf("%s %d - APN %s length %lu \n",__FUNCTION__,__LINE__, apn_list[i].apn_name_label, apn_list[i].apn_name_length);
		if ((apn_length == apn_list[i].apn_name_length)
			&& !memcmp(apn_label, apn_list[i].apn_name_label,
			apn_length)) {
            return &apn_list[i];
	        }
	}
	printf("%s %d - APN Not found index = %d \n",__FUNCTION__,__LINE__, i);

    return NULL;
}

