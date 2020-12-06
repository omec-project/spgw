// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#ifndef _CP_LOG_H
#define _CP_LOG_H
#include <stdio.h>
#include "gen_utils.h"
#include <stdint.h>

enum {
	LOG_NEVER=0,
	LOG_INIT,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};

static const char *log_level_name[] = { "NEVER", "INIT", "ERROR", "WARN", "INFO", "DEBUG"};

extern uint8_t logging_level;

#define LOG_MSG(prio, msg, ...) do {\
	if(prio<=logging_level) { \
		fprintf(stdout, "[%s] : %s : %d : "msg" \n", log_level_name[prio], __file__, __LINE__, ##__VA_ARGS__);\
	}\
} while (0) 

inline void set_logging_level(const char *level) {
	if(!strcmp(level, "LOG_NEVER")) {
		logging_level = LOG_NEVER;
	} else if (!strcmp(level, "LOG_ERROR")) {
		logging_level = LOG_ERROR;
	} else if (!strcmp(level, "LOG_WARN")) {
		logging_level = LOG_WARN;
	} else if (!strcmp(level, "LOG_INFO")) {
		logging_level = LOG_INFO;
	} else if (!strcmp(level, "LOG_DEBUG")) {
		logging_level = LOG_DEBUG;
	} else {
		// default level
		logging_level = LOG_ERROR;
	}
	return;
}

#endif
