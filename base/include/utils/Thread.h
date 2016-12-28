#ifndef _LIBS_UTILS_THREAD_H
#define _LIBS_UTILS_THREAD_H

#include <utils/Condition.h>
#include <utils/mutex.h>
#include <utils/timers.h>
#include <utils/errors.h>

namespace linuxSys{

class Thread
{
public:
    /* create a thread object, but doesn't create or start the associated
    thread. see the run() method */
    Thread();
    virtual ~Thread();

    virtual status_t run(const char* name = 0,
                        int priority = 0,
                        size_t stackSize = 0);

    /* ask this obiect's thread to exit. This function is asyncchronous,
    when the function  returns the thread might still be runing. of course,
    this function can be called from a different thread. */
    virtual void requestExit();

    /* good place to do one-time initializations. */
    virtual status_t readyToRun();

    /* call requestExit() and wait until this object's thread exits.
    BE VERY CAREFUL of deadlocks. In particular, it would be silly to call
    this function from this object's thread. Will return WOULD_BLOCK in
    that case. */
    status_t requestExitAndWait();

    /* wait until this object's thread exits. Returns immediately if not yet running
     Do not call from this object's thread; will return WOULD_BLOCK in that case. */
    status_t join();

    /* Indicates whether this thread is running or not. */
    bool isRunning() const;

protected:
    /* exitPending() returns true if requestExit() has been called. */
    bool exitPending() const;

private:
    /* Derived class must implement threadLoop(). The thread starts its life 
    here. There are two ways of using the Thread object:
    1)loop:if threadLoop() returns true, it will be called again if
       requestExit() wasn't called.
    2)once:if threadLoop() returns false, the thread will exit upon return */
    virtual bool threadLoop() = 0;

private:
    Thread& operator=(const Thread&);
    static void*  _threadLoop(void* user);
    
    /* always hold mLock when reading or writing */
    mutable Mutex mLock;
    Condition     mThreadExitedCondition;
    status_t      mStatus;
    pthread_t     mThreadTid;

    /* note that all accesses of mExitPending and mRunning need to hold mLock. */
    volatile bool mExitPending;
    volatile bool mRunning;
    pthread_t     mTid;
};

}

#endif