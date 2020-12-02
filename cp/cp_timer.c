// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include "rte_hash.h"
#include "rte_errno.h"
#include "cp_timer.h"
#include "clogger.h"
#include "errno.h"
#include "cp_log.h"


static pthread_t _gstimer_thread;
static pid_t _gstimer_tid;


const char *getPrintableTime(void)
{
	static char buf[128];
	struct timeval tv;
	struct timezone tz;
	struct tm *ptm;

	gettimeofday(&tv, &tz);
	ptm = localtime( &tv.tv_sec );

	sprintf( buf, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
			ptm->tm_year + 1900,
			ptm->tm_mon + 1,
			ptm->tm_mday,
			ptm->tm_hour,
			ptm->tm_min,
			ptm->tm_sec,
			tv.tv_usec / 1000 );

	return buf;
}

/**
 * @brief  : Start timer thread
 * @param  : arg, used to control access to thread
 * @return : Returns nothing
 */
static void *_gstimer_thread_func(void *arg)
{
	int keepgoing = 1;
	sem_t *psem = (sem_t*)arg;
	sigset_t set;
	siginfo_t si;

	_gstimer_tid = syscall(SYS_gettid);
	sem_post( psem );

	sigemptyset( &set );
	sigaddset( &set, SIGRTMIN );
	sigaddset( &set, SIGUSR1 );
	LOG_MSG(LOG_INIT, "Started timer thread ");
	while (keepgoing)
	{
		int sig = sigwaitinfo( &set, &si );

		if ( sig == SIGRTMIN )
		{
			gstimerinfo_t *ti = (gstimerinfo_t*)si.si_value.sival_ptr;

			if ( ti->ti_cb )
				(*ti->ti_cb)( ti, ti->ti_data );
		}
		else if ( sig == SIGUSR1 )
		{
			keepgoing = 0;
		}
	}

	LOG_MSG(LOG_ERROR, "Exiting timer thread ");
	return NULL;
}


//public
bool gst_init(void)
{
	int status;
	sem_t sem;

	/*
	 * start the timer thread and wait for _timer_tid to be populated
	 */
	sem_init( &sem, 0, 0 );
	status = pthread_create( &_gstimer_thread, NULL, &_gstimer_thread_func, &sem );
	if (status != 0)
		return False;

	sem_wait( &sem );
	sem_destroy( &sem );

	return true;
}

//public
void gst_deinit(void)
{
	/*
	 * stop the timer handler thread
	 */
	pthread_kill( _gstimer_thread, SIGUSR1 );
	pthread_join( _gstimer_thread, NULL );
}

/**
 * @brief  : Initialize timer
 * @param  : timer_id, timer if
 * @param  : data, holds timer related information
 * @return : Returns true in case of success , false otherwise
 */
static bool _create_timer(timer_t *timer_id, const void *data)
{
	int status;
	struct sigevent se;

	/*
	 * Set the sigevent structure to cause the signal to be
	 * delivered by creating a new thread.
	 */
	memset(&se, 0, sizeof(se));
	se.sigev_notify = SIGEV_THREAD_ID;
	se._sigev_un._tid = _gstimer_tid;
	se.sigev_signo = SIGRTMIN;
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
	se.sigev_value.sival_ptr = (void*)data;
#pragma GCC diagnostic pop   /* require GCC 4.6 */
	/*
	 * create the timer
	 */
	status = timer_create(CLOCK_REALTIME, &se, timer_id);

	return status == 0 ? true : false;
}

// pub;ic 
bool gst_timer_init( gstimerinfo_t *ti, gstimertype_t tt,
				gstimercallback cb, int milliseconds, const void *data )
{
	ti->ti_type = tt;
	ti->ti_cb = cb;
	ti->ti_ms = milliseconds;
	ti->ti_data = data;

	return _create_timer( &ti->ti_id, ti );
}

void gst_timer_deinit(gstimerinfo_t *ti)
{
	timer_delete( ti->ti_id );
}

bool gst_timer_setduration(gstimerinfo_t *ti, int milliseconds)
{
	ti->ti_ms = milliseconds;
	return gst_timer_start( ti );
}

bool gst_timer_start(gstimerinfo_t *ti)
{
	int status;
	struct itimerspec ts;

	/*
	 * set the timer expiration
	 */
	ts.it_value.tv_sec = ti->ti_ms / 1000;
	ts.it_value.tv_nsec = (ti->ti_ms % 1000) * 1000000;
	if ( ti->ti_type == ttInterval )
	{
		ts.it_interval.tv_sec = ts.it_value.tv_sec;
		ts.it_interval.tv_nsec = ts.it_value.tv_nsec;
	}
	else
	{
		ts.it_interval.tv_sec = 0;
		ts.it_interval.tv_nsec = 0;
	}

	status = timer_settime( ti->ti_id, 0, &ts, NULL );
	return status == -1 ? false : true;
}

void gst_timer_stop(gstimerinfo_t *ti)
{
	struct itimerspec ts;

	/*
	 * set the timer expiration, setting it_value and it_interval to 0 disables the timer
	 */
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;

	timer_settime( ti->ti_id, 0, &ts, NULL );

}



bool startTimer( gstimerinfo_t *ti )
{
	return gst_timer_start( ti );
}

void stopTimer( gstimerinfo_t *ti )
{
	gst_timer_stop( ti );
}

void deinitTimer( gstimerinfo_t *ti )
{
	gst_timer_deinit( ti );
}

bool init_timer(gstimerinfo_t *pt, int ptms, gstimercallback cb, void *data)
{
	return gst_timer_init(pt, ttInterval, cb, ptms, data);
}

bool starttimer(gstimerinfo_t *ti)
{
	return gst_timer_start( ti );
}

void stoptimer(timer_t *tid)
{
	struct itimerspec ts;

	/*
	 * set the timer expiration, setting it_value and it_interval to 0 disables the timer
	 */
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;

	timer_settime(*tid, 0, &ts, NULL);

}

void deinittimer(timer_t *tid)
{
	timer_delete(*tid);
}


