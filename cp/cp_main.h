// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * max length of name string.
 */
#define MAX_LEN 128

#ifdef USE_CSID
/* Configure the local csid */
extern uint16_t local_csid;
#endif /* USE_CSID */

extern int32_t conn_cnt;


#ifdef __cplusplus
}
#endif
#endif /* _MAIN_H_ */

