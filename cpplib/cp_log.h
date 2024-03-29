// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0


#ifndef _CP_LOG_H
#define _CP_LOG_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "gen_utils.h"
#include <time.h>

#ifdef __cplusplus  
extern "C" { 
#endif 
enum {
	LOG_ALL=0,
	LOG_INIT,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
    LOG_DEBUG0,
    LOG_DEBUG1, // MME/UPF/PCRF node related operations
    LOG_DEBUG2, // Message Traciing
    LOG_DEBUG3, // Session/Pdn/Bearer Creation/deletion
    LOG_DEBUG4, // FSM transitions for subscriber 
    LOG_DEBUG5, // table related operations
	LOG_DEBUG,
    LOG_NEVER // Dont print this log 
};

static const char *log_level_name[] = { "ALL", "INIT", "ERROR", "WARN", "INFO", "DEBUG0", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4", "DEBUG5", "DEBUG", "NEVER"};

extern uint8_t logging_level;

#define LOG_MSG(prio, msg, ...) do {\
	if(prio<=logging_level) { \
        char _s[30]; \
        time_t _t = time(NULL); \
        struct tm * _p = localtime(&_t);\
        strftime(_s, 30, "%Y-%m-%d %H:%M:%S", _p);\
		fprintf(stdout, "[%s] %s : %s : %s : %d : " msg " \n", log_level_name[prio], _s, __file__ , __func__, __LINE__, ##__VA_ARGS__);\
	}\
} while (0) 

static inline void set_logging_level(const char *level) {
	if(!strcmp(level, "LOG_NEVER")) {
		logging_level = LOG_NEVER;
	} else if (!strcmp(level, "LOG_ERROR")) {
		logging_level = LOG_ERROR;
	} else if (!strcmp(level, "LOG_WARN")) {
		logging_level = LOG_WARN;
	} else if (!strcmp(level, "LOG_INFO")) {
		logging_level = LOG_INFO;
	} else if (!strcmp(level, "LOG_DEBUG1")) {
		logging_level = LOG_DEBUG1;
	} else if (!strcmp(level, "LOG_DEBUG2")) {
		logging_level = LOG_DEBUG2;
	} else if (!strcmp(level, "LOG_DEBUG3")) {
		logging_level = LOG_DEBUG3;
	} else if (!strcmp(level, "LOG_DEBUG4")) {
		logging_level = LOG_DEBUG4;
	} else if (!strcmp(level, "LOG_DEBUG5")) {
		logging_level = LOG_DEBUG5;
	} else if (!strcmp(level, "LOG_DEBUG")) {
		logging_level = LOG_DEBUG;
	} else {
		// default level
		logging_level = LOG_ERROR;
	}
    LOG_MSG(LOG_INIT, "Logging level set to %s", log_level_name[logging_level]);
	return;
}

#ifdef __cplusplus  
}
#endif 
#endif
