#include "utils/Thread.h"
#include "utils/Logger.h"

namespace linuxSys{

Thread::Thread()
    : mLock("Thread::mLock"),
      mStatus(NO_ERROR),
      mThreadTid(pthread_t(-1)),
      mExitPending(false),
      mRunning(false),
      mTid(-1)
{

}

Thread::~Thread()
{

}

status_t Thread::readyToRun()
{
    return NO_ERROR;
}

status_t Thread::run(const char* name, int priority, size_t stackSize)
{
    Mutex::Autolock _L(mLock);

    if (mRunning)
    {
        /* thread already started. */
        LOGE("[run] thread already started.");
        return INVALID_OPERATION;
    }

    mStatus = NO_ERROR;
    mExitPending = false;
    mThreadTid   = pthread_t(-1);

    mRunning = true;

    do
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        if (priority != 0 || name != NULL)
        {
            /* TODO set thread priority. */
        }
        if (stackSize)
        {
            pthread_attr_setstacksize(&attr,stackSize);
        }
        pthread_t thread;
        int result = pthread_create(&thread,&attr,_threadLoop,this);
        if (result != 0)
        {
            LOGE("[run] create thread fail!");
            goto ERROR;
        }
        mThreadTid = thread;

    }while(0);


    return NO_ERROR;
ERROR:
    mRunning = false;
    mStatus = UNKNOWN_ERROR; //something happened!
    return UNKNOWN_ERROR;
}

void* Thread::_threadLoop(void* user)
{
    Thread* const self = static_cast<Thread*>(user);

    bool first = true;
    bool result = 0;

    do{
        if (first)
        {
            first = false;
            self->mStatus = self->readyToRun();
            result = (self->mStatus == NO_ERROR);

            if (result && self->exitPending())
            {
                result = self->threadLoop();
            }
        }
        else
        {
            result = self->threadLoop();
            do{
                Mutex::Autolock _L(self->mLock);

                if (result == false || self->mExitPending)
                {
                    self->mExitPending = true;
                    self->mRunning     = false;
                    /* clear thread ID so that requestExitAndWait() does not exit if
                    called by a new thread using the same thread ID as this one. */
                    self->mThreadTid   = pthread_t(-1);
                    /* note that interested observers blocked in requestExitAndWait are
                    awoken by broadcast, but blocked on mLock until break exits scope. */
                    self->mThreadExitedCondition.broadcast();
                    break;
                }
            }while(0);
        }
        
    }while(result != false);

    return 0;
}

void Thread::requestExit()
{
    Mutex::Autolock _L(mLock);
    mExitPending = true;
}

status_t Thread::requestExitAndWait()
{
    Mutex::Autolock _L(mLock);
    if (pthread_equal(mThreadTid,pthread_self()))
    {
        LOGE("Thread (this = %p):don't call waitForExit() from this"
            "Thread object's thread. It's a guaranteed deadlock!",this);
        return WOULD_BLOCK;
    }

    mExitPending = true;
    while(mRunning == true){
        mThreadExitedCondition.wait(mLock);
    }
    /* This next line is probably not needed any more, but is being left
    for historical refetence. Note that each interested party will clear flag. */
    mExitPending = false;

    return mStatus;
}

status_t Thread::join()
{
    Mutex::Autolock _L(mLock);
    if (pthread_equal(mThreadTid,pthread_self()))
    {
        LOGE("Thread (this = %p):don't call waitForExit() from this"
            "Thread object's thread. It's a guaranteed deadlock!",this);
        return WOULD_BLOCK;
    }

    while(mRunning == true)
    {
        mThreadExitedCondition.wait(mLock);
    }
    return mStatus;
}

bool Thread::isRunning() const
{
    Mutex::Autolock _L(mLock);
    return mRunning;
}

bool Thread::exitPending() const
{
    Mutex::Autolock _L(mLock);
    return mExitPending;
}

}