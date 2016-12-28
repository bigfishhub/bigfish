#ifndef __LIBS_UTILS_MUTEX__H
#define __LIBS_UTILS_MUTEX__H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "errors.h"

namespace linuxSys
{

class Mutex{

public:
    enum{
        PRIVATE = 0,
        SHARED  = 1
    };

    Mutex();
    Mutex(const char* name);
    Mutex(int type, const char* name = NULL);
    ~Mutex();

    /*lock and unlock the mutex*/
    status_t lock();
    status_t unlock();

    /*lock if possible,return 0 on success, error otherwise*/
    status_t trylock();
    status_t timedlock(const unsigned int millisecond);

    /*Manages the mutex automatically. It'll be locked when Autolock is
     constructed and released when Autolock goes out of scope*/
    class Autolock{
    public:
        inline Autolock(Mutex& mutex):mLock(mutex) { mLock.lock(); }
        inline Autolock(Mutex* mutex):mLock(*mutex) { mLock.lock(); }
        inline ~Autolock() { mLock.unlock(); }
    private:
        Mutex& mLock;
    };

private:
    friend class Condition;
    /* A mutex cannot be copied*/
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

    pthread_mutex_t mMutex;
};

inline Mutex::Mutex()
{
    pthread_mutex_init(&mMutex,NULL);
}

inline Mutex::Mutex(__attribute__((unused)) const char* name)
{
    pthread_mutex_init(&mMutex,NULL);
}

inline Mutex::Mutex(int type, __attribute__((unused)) const char* name)
{
    if (SHARED == type)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mMutex,&attr);
        pthread_mutexattr_destroy(&attr);
    }
    else
    {
        pthread_mutex_init(&mMutex,NULL);
    }
}

inline Mutex::~Mutex()
{
    pthread_mutex_destroy(&mMutex);
}

inline status_t Mutex::lock()
{
    return pthread_mutex_lock(&mMutex);
}

inline status_t Mutex::unlock()
{
    return pthread_mutex_unlock(&mMutex);
}

inline status_t Mutex::trylock()
{
    return pthread_mutex_trylock(&mMutex);
}

inline status_t Mutex::timedlock(const unsigned int millisecond)
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
    return pthread_mutex_timedlock(&mMutex,&localtime);
}

}
#endif