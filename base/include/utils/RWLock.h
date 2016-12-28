#ifndef __LIBS_UTILS_RWLOCK__H
#define __LIBS_UTILS_RWLOCK__H

#include "errors.h"
#include <pthread.h>

namespace linuxSys{

class RWLock{

public:
    enum{
        PRIVATE = 0,
        SHARED  = 1
    };

    RWLock();
    RWLock(const char* name);
    RWLock(int type, const char* name = NULL);
    ~RWLock();

    status_t readLock();
    status_t tryReadLock();
    status_t timedReadLock(const unsigned int millisecond);
    status_t writeLock();
    status_t tryWriteLock();
    status_t timedWriteLock(const unsigned int millisecond);
    status_t unlock();

    class AutoRLock{
    public:
        inline AutoRLock(RWLock& rwlock):mLock(rwlock) { mLock.readLock(); }
        inline ~AutoRLock() { mLock.unlock(); }
    private:
        RWLock& mLock;
    };

    class AutoWLock{
    public:
        inline AutoWLock(RWLock& rwlock):mLock(rwlock) { mLock.writeLock(); }
        inline ~AutoWLock() { mLock.unlock(); }
    private:
        RWLock& mLock;
    };

private:
    /* A RWLock cannot be copied */
    RWLock(const RWLock&);
    RWLock& operator=(const RWLock&);

    pthread_rwlock_t mRWLock;
};

inline RWLock::RWLock()
{
    pthread_rwlock_init(&mRWLock,NULL);
}

inline RWLock::RWLock(__attribute__((unused)) const char* name)
{
    pthread_rwlock_init(&mRWLock,NULL);
}

inline RWLock::RWLock(int type, __attribute__((unused)) const char* name)
{
    if (type == SHARED)
    {
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
        pthread_rwlock_init(&mRWLock,&attr);
        pthread_rwlockattr_destroy(&attr);
    }
    else
    {
        pthread_rwlock_init(&mRWLock,NULL);
    }
}

inline RWLock::~RWLock()
{
    pthread_rwlock_destroy(&mRWLock);
}

inline status_t RWLock::readLock()
{
    return pthread_rwlock_rdlock(&mRWLock);
}

inline status_t RWLock::tryReadLock()
{
    return pthread_rwlock_tryrdlock(&mRWLock);
}

inline status_t RWLock::writeLock()
{
    return pthread_rwlock_wrlock(&mRWLock);
}

inline status_t RWLock::tryWriteLock()
{
    return pthread_rwlock_trywrlock(&mRWLock);
}

inline status_t RWLock::unlock()
{
    return pthread_rwlock_unlock(&mRWLock);
}

inline status_t RWLock::timedReadLock(const unsigned int millisecond)
{
    struct timespec localtime;

    if (!clock_gettime(CLOCK_REALTIME,&localtime))
    {
        return -1;
    }
    localtime.tv_sec  += (millisecond/1000);
    localtime.tv_nsec += (millisecond%1000*1000);
    return pthread_rwlock_timedrdlock(&mRWLock,&localtime);
}

inline status_t RWLock::timedWriteLock(const unsigned int millisecond)
{
    struct timespec localtime;

    if (!clock_gettime(CLOCK_REALTIME,&localtime))
    {
        return -1;
    }
    localtime.tv_sec  += (millisecond/1000);
    localtime.tv_nsec += (millisecond%1000*1000);
    if (localtime.tv_nsec >= 1000000000)
    {
        localtime.tv_nsec -= 1000000000;
        localtime.tv_sec  += 1;
    } 
    return pthread_rwlock_timedwrlock(&mRWLock,&localtime);
}

}
#endif