// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _MAIN_H_
#define _MAIN_H_

/**
 * max length of name string.
 */
#define MAX_LEN 128
#ifdef USE_REST

/* VS: Number of connection can maitain in the hash */
#define NUM_CONN	500

#ifdef USE_CSID
/* Configure the local csid */
extern uint16_t local_csid;
#endif /* USE_CSID */

extern int32_t conn_cnt;

/**
 * @brief  : Initiatizes echo table and starts the timer thread
 * @param  : No param
 * @return : Returns nothing
 */
void rest_thread_init(void);


#endif  /* USE_REST */

/**
 * @brief  : Functino to handle signals.
 * @param  : signo
 *           signal number signal to be handled
 * @return : Returns nothing
 */
void sig_handler(int signo);

#endif /* _MAIN_H_ */

