#include "utils/timers.h"
#include <sys/time.h>

nsecs_t getSystemTimeNs(int clock)
{
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	clock_gettime(clock,&t);

	return nsecs_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}

msecs_t getSystemTimeMs()
{
	struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return msecs_t(t.tv_sec)*1000L + msecs_t(t.tv_usec)/1000;
}

usecs_t getSystemTimeUs()
{
	struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return msecs_t(t.tv_sec)*1000000L + msecs_t(t.tv_usec);
}

secs_t  getSystemTimeS()
{
	return time(NULL);
}
