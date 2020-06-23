// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _MASTER_CDR_H
#define _MASTER_CDR_H
/**
 * @file
 * This file contains function prototypes for the CDR master record
 */

#include "cdr.h"


/**
 * @brief  : sets the master cdr file
 * @param  : master_cdr_file, master cdr filepath
 * @return : Returns nothing
 */
void
set_master_cdr_file(const char *master_cdr_file);

/**
 * @brief  : finalizes *.cur cdr files into *.csv and records into the master cdr file
 * @param  : cdr_path
 * @return : Returns nothing
 */
void
finalize_cur_cdrs(const char *cdr_path);

/**
 * @brief  : frees all memory allocated by master_cdr.c
 * @param  : No param
 * @return : Returns nothing
 */
void
free_master_cdr(void);

#endif /* _MASTER_H */
