// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __GSTIMER_H
#define __GSTIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
#define S11_SGW_PORT_ID   0
#define S5S8_SGWC_PORT_ID 1
#define SX_PORT_ID        2
#define S5S8_PGWC_PORT_ID 3

#define OFFSET      2208988800ULL

/**
 * @brief  : Numeric value for true and false
 */
typedef enum { False = 0, True } boolean;

/**
 * @brief  : Maintains timer related information
 */
typedef struct _gstimerinfo_t gstimerinfo_t;

/**
 * @brief  : function pointer to timer callback
 */
typedef void (*gstimercallback)(gstimerinfo_t *ti, const void *data);

/**
 * @brief  : Maintains timer type
 */
typedef enum {
	ttSingleShot,
	ttInterval
} gstimertype_t;

/**
 * @brief  : Maintains timer related information
 */
struct _gstimerinfo_t {
	timer_t           ti_id;
	gstimertype_t     ti_type;
	gstimercallback   ti_cb;
	int               ti_ms;
	const void       *ti_data;
};

/**
 * @brief  : start the timer thread and wait for _timer_tid to be populated
 * @param  : No param
 * @return : Returns true in case of success , false otherwise
 */
bool gst_init(void);

/**
 * @brief  : Stop the timer handler thread
 * @param  : No param
 * @return : Returns nothing
 */
void gst_deinit(void);

/**
 * @brief  : Initialize timer with provided information
 * @param  : ti, timer structure to be initialized
 * @param  : cb, timer callback function
 * @param  : milliseconds, timeout in milliseconds
 * @param  : data, timer data
 * @return : Returns true in case of success , false otherwise
 */
bool gst_timer_init( gstimerinfo_t *ti, gstimertype_t tt,
			gstimercallback cb, int milliseconds, const void *data );

/**
 * @brief  : Delete timer
 * @param  : ti, holds information about timer to be deleted
 * @return : Returns nothing
 */
void gst_timer_deinit( gstimerinfo_t *ti );

/**
 * @brief  : Set timeout in timer
 * @param  : ti, holds information about timer
 * @param  : milliseconds, timeout in milliseconds
 * @return : Returns true in case of success , false otherwise
 */
bool gst_timer_setduration( gstimerinfo_t *ti, int milliseconds );

/**
 * @brief  : Start timer
 * @param  : ti, holds information about timer
 * @return : Returns true in case of success , false otherwise
 */
bool gst_timer_start( gstimerinfo_t *ti );

/**
 * @brief  : Stop timer
 * @param  : ti, holds information about timer
 * @return : Returns nothing
 */
void gst_timer_stop( gstimerinfo_t *ti );

/**
 * @brief  : Start timer
 * @param  : ti, holds information about timer
 * @return : Returns true in case of success , false otherwise
 */
bool startTimer( gstimerinfo_t *ti );

/**
 * @brief  : Stop timer
 * @param  : ti, holds information about timer
 * @return : Returns nothing
 */
void stopTimer( gstimerinfo_t *ti );

/**
 * @brief  : Delete timer
 * @param  : ti, holds information about timer
 * @return : Returns nothing
 */
void deinitTimer( gstimerinfo_t *ti );


/**
 * @brief  : Initialize timer
 * @param  : md, Peer node information
 * @param  : t1ms, periodic timer interval
 * @param  : cb, timer callback function
 * @return : Returns true in case of success , false otherwise
 */
bool init_timer(gstimerinfo_t *pt, int ptms, gstimercallback cb, void *data);

/**
 * @brief  : Start timer
 * @param  : ti, holds information about timer
 * @return : Returns true in case of success , false otherwise
 */
bool starttimer( gstimerinfo_t *ti );

/**
 * @brief  : Stop timer
 * @param  : tid, timer id
 * @return : Returns nothing
 */
void stoptimer(timer_t *tid);

/**
 * @brief  : Delete timer
 * @param  : tid, timer id
 * @return : Returns nothing
 */
void deinittimer(timer_t *tid);

#ifdef __cplusplus
}
#endif
#endif
