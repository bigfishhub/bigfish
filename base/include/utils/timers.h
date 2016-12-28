#ifndef __LIBS_UTILS_TIMERS__H
#define __LIBS_UTILS_TIMERS__H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

# if __WORDSIZE == 64
typedef long int           int64_t;
# else
typedef long long int      int64_t;
#endif


typedef int64_t nsecs_t;       // nano-seconds
typedef int64_t usecs_t;       // subtle-seconds
typedef int64_t msecs_t;       // msec-seconds
typedef int64_t secs_t;        // seconds


enum{
	SYSTEM_TIME_REALTIME           = CLOCK_REALTIME, //system-wide realtime clock
	SYSTEM_TIME_MONOTONIC          = CLOCK_MONOTONIC, //monotonic time since unspecified starting point
	SYSTEM_TIME_PROCESS_CPUTIME_ID = CLOCK_PROCESS_CPUTIME_ID,//high-resolution per-process clock
	SYSTEM_TIME_THREAD_CPUTIME_ID  = CLOCK_THREAD_CPUTIME_ID //high-resolution per-thread clock
};


nsecs_t getSystemTimeNs(int clock = SYSTEM_TIME_REALTIME);
msecs_t getSystemTimeMs();
usecs_t getSystemTimeUs();
secs_t  getSystemTimeS();

#ifdef __cplusplus
}
#endif

#endif