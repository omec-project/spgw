// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include <sys/time.h>
#include "pfcp_cp_util.h"

uint32_t
current_ntp_timestamp(void)
{

       struct timeval tim;
       uint8_t ntp_time[8] = {0};
       uint32_t timestamp = 0;

       gettimeofday(&tim, NULL);
       time_to_ntp(&tim, ntp_time);

       timestamp |= ntp_time[0] << 24 | ntp_time[1] << 16
                                                               | ntp_time[2] << 8 | ntp_time[3];

       return timestamp;
}
void
time_to_ntp(struct timeval *tv, uint8_t *ntp)
{
	uint64_t ntp_tim = 0;
	uint8_t len = (uint8_t)sizeof(ntp)/sizeof(ntp[0]);
	uint8_t *p = ntp + len;

	int i = 0;

	ntp_tim = tv->tv_usec;
	ntp_tim <<= 32;
	ntp_tim /= 1000000;

	/* Setting the ntp in network byte order */

	for (i = 0; i < len/2; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}

	ntp_tim = tv->tv_sec;
	ntp_tim += OFFSET;

	/* Settting  the fraction of second */

	for (; i < len; i++) {
		*--p = ntp_tim & 0xff;
		ntp_tim >>= 8;
	}

}
