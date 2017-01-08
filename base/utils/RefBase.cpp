#define LOG_TAG "RefBase"

#include <fcntl.h>

#include <utils/RefBase.h>
#include <cutils/atomic.h>

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

#ifndef ALOG_ASSERT
#define ALOG_ASSERT(cond, ...) ((void)0)//LOG_FATAL_IF(!(cond), ## __VA_ARGS__)
#endif

// compile with refcounting debugging enabled
#define DEBUG_REFS     0

namespace linuxSys {

#define INITIAL_STRONG_VALUE (1<<28)

//------------------------------------------------------------

class RefBase::weakref_impl : public RefBase::weakref_type
{
public:
    volatile int32_t mStrong;
    volatile int32_t mWeak;
    RefBase* const   mBase;
    volatile int32_t mFlags;

#if !DEBUG_REFS

    weakref_impl(RefBase* base)
        :mStrong(INITIAL_STRONG_VALUE)
        ,mWeak(0)
        ,mBase(base)
        ,mFlags(0)
    {
    }

    void addStrongRef(const void* /*id*/) { }
    void removeStrongRef(const void* /*id*/) { }
    void renameStrongRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void addWeakRef(const void* /*id*/) { }
    void removeWeakRef(const void* /*id*/) { }
    void renameWeakRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void printRefs() const { }
    void trackMe(bool, bool) { }

#else

#endif

};

//------------------------------------------------------------

void RefBase::incStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);
    refs->addStrongRef(id); //if release,it is an empty function
    const int32_t c = linux_atomic_inc(&refs->mStrong);
    ALOG_ASSERT(c > 0, "incStrong() called on %p after last strong ref", refs);

    if (c != INITIAL_STRONG_VALUE){
        return ;
    }
    linux_atomic_add(-INITIAL_STRONG_VALUE,&refs->mStrong);
    refs->mBase->onFirstRef();
}

void RefBase::decStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->removeStrongRef(id); //if release,it is an empty function
    const int32_t c = linux_atomic_dec(&refs->mStrong);
    ALOG_ASSERT(c >= 1, "decStrong() called on %p too many times", refs);
    if (c == 1){
        refs->mBase->onLastStrongRef(id);
        if ((refs->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG){
            delete this;
        }
    }
    refs->decWeak(id);
}

void RefBase::froceIncStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);
    refs->addStrongRef(id); // if release,it is an empty function
    const int32_t c = linux_atomic_inc(&refs->mStrong);
    ALOG_ASSERT(c >= 0, "forceIncStrong called on %p after ref count underflow",refs);
    switch (c){
        case INITIAL_STRONG_VALUE:
            linux_atomic_add(-INITIAL_STRONG_VALUE,&refs->mStrong);
            //break;
        case 0:
            refs->mBase->onFirstRef();
    }
}

int32_t RefBase::getStrongCount() const
{
    return mRefs->mStrong;
}

//-------------------------------------------------------------

RefBase* RefBase::weakref_type::refBase() const
{
    return static_cast<const weakref_impl*>(this)->mBase;
}

void RefBase::weakref_type::incWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->addWeakRef(id);  //if release,it is an empty function
    const int32_t c __unused = linux_atomic_inc(&impl->mWeak);
    ALOG_ASSERT(c >= 0, "incWeak called on %p after last weak ref", this);
}

void RefBase::weakref_type::decWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->removeWeakRef(id); //if release,it is an empty function
    const int32_t c = linux_atomic_dec(&impl->mWeak);
    ALOG_ASSERT(c >= 1, "decWeak called on %p too many times", this);
    if (c != 1) return;

    if ((impl->mFlags & OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG)
    {
        /* This is the regular lifetime case. The object is destroyed
           when the last strong reference goes away. Since weakref_impl
           outlive the object, it is not destroyed in the dtor, and
           we'll have to do it here. */
        if (impl->mStrong == INITIAL_STRONG_VALUE){
            /* Special case: we never had a strong reference, so we need 
               destroy the object now. */
            delete impl->mBase;
        }else{
            delete impl;
        }
    }else{
        /* less common case : lifetime is OBJECT_LIFETIME_{WEAK|FOREVER} */
        impl->mBase->onLastWeakRef(id);
        if ((impl->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK){
            /* this is the OBJECT_LIFETIME_WEAK case. The last weak-reference
               is gone, we can destroy the object. */
            delete impl->mBase;
        }
    }
}

bool RefBase::weakref_type::attemptIncStrong(const void* id)
{
    incWeak(id);

    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    int32_t curCount = impl->mStrong;

    ALOG_ASSERT(curCount >= 0,
            "attemptIncStrong called on %p after underflow", this);

    while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
        // we're in the easy/common case of promoting a weak-reference
        // from an existing strong reference.
        if (linux_atomic_cmpxchg(curCount, curCount+1, &impl->mStrong) == 0) {
            break;
        }
        // the strong count has changed on us, we need to re-assert our
        // situation.
        curCount = impl->mStrong;
    }

    if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
        // we're now in the harder case of either:
        // - there never was a strong reference on us
        // - or, all strong references have been released
        if ((impl->mFlags&OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG) {
            // this object has a "normal" life-time, i.e.: it gets destroyed
            // when the last strong reference goes away
            if (curCount <= 0) {
                // the last strong-reference got released, the object cannot
                // be revived.
                decWeak(id);
                return false;
            }

            // here, curCount == INITIAL_STRONG_VALUE, which means
            // there never was a strong-reference, so we can try to
            // promote this object; we need to do that atomically.
            while (curCount > 0) {
                if (linux_atomic_cmpxchg(curCount, curCount + 1,
                        &impl->mStrong) == 0) {
                    break;
                }
                // the strong count has changed on us, we need to re-assert our
                // situation (e.g.: another thread has inc/decStrong'ed us)
                curCount = impl->mStrong;
            }

            if (curCount <= 0) {
                // promote() failed, some other thread destroyed us in the
                // meantime (i.e.: strong count reached zero).
                decWeak(id);
                return false;
            }
        } else {
            // this object has an "extended" life-time, i.e.: it can be
            // revived from a weak-reference only.
            // Ask the object's implementation if it agrees to be revived
            if (!impl->mBase->onIncStrongAttempted(FIRST_INC_STRONG, id)) {
                // it didn't so give-up.
                decWeak(id);
                return false;
            }
            // grab a strong-reference, which is always safe due to the
            // extended life-time.
            curCount = linux_atomic_inc(&impl->mStrong);
        }

        // If the strong reference count has already been incremented by
        // someone else, the implementor of onIncStrongAttempted() is holding
        // an unneeded reference.  So call onLastStrongRef() here to remove it.
        // (No, this is not pretty.)  Note that we MUST NOT do this if we
        // are in fact acquiring the first reference.
        if (curCount > 0 && curCount < INITIAL_STRONG_VALUE) {
            impl->mBase->onLastStrongRef(id);
        }
    }

    impl->addStrongRef(id);

    // now we need to fix-up the count if it was INITIAL_STRONG_VALUE
    // this must be done safely, i.e.: handle the case where several threads
    // were here in attemptIncStrong().
    curCount = impl->mStrong;
    while (curCount >= INITIAL_STRONG_VALUE) {
        ALOG_ASSERT(curCount > INITIAL_STRONG_VALUE,
                "attemptIncStrong in %p underflowed to INITIAL_STRONG_VALUE",
                this);
        if (linux_atomic_cmpxchg(curCount, curCount-INITIAL_STRONG_VALUE,
                &impl->mStrong) == 0) {
            break;
        }
        // the strong-count changed on us, we need to re-assert the situation,
        // for e.g.: it's possible the fix-up happened in another thread.
        curCount = impl->mStrong;
    }

    return true;
}

bool RefBase::weakref_type::attemptIncWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);

    int32_t curCount = impl->mWeak;
    ALOG_ASSERT(curCount >= 0, "attemptIncWeak called on %p after underflow",
               this);
    while (curCount > 0) {
        if (linux_atomic_cmpxchg(curCount, curCount+1, &impl->mWeak) == 0) {
            break;
        }
        curCount = impl->mWeak;
    }

    if (curCount > 0) {
        impl->addWeakRef(id);
    }

    return curCount > 0;
}

int32_t RefBase::weakref_type::getWeakCount() const
{
    return static_cast<const weakref_impl*>(this)->mWeak;
}

void RefBase::weakref_type::printRefs() const
{
    static_cast<const weakref_impl*>(this)->printRefs();
}

void RefBase::weakref_type::trackMe(bool enable, bool retain)
{
    static_cast<weakref_impl*>(this)->trackMe(enable,retain);
}

RefBase::weakref_type* RefBase::createWeak(const void* id) const
{
    mRefs->incWeak(id);
    return mRefs;
}

RefBase::weakref_type* RefBase::getWeakRefs() const
{
    return mRefs;
}

RefBase::RefBase()
    :mRefs(new weakref_impl(this))
{
}

RefBase::~RefBase()
{
    if (mRefs->mStrong == INITIAL_STRONG_VALUE){
        /* we need acquired a strong (and/or weak) reference on this object. */
        delete mRefs;
    }else{
        /* life-time of this object is extended to WEAK or FOREVER, in
           which case weakref_impl doesn't out-live the object and we
           can free it now. */
        if ((mRefs->mFlags & OBJECT_LIFETIME_MASK) != OBJECT_LIFETIME_STRONG){
            /* it is possible that the weak count is not 0 if the object 
               re-acquired a weak reference in its destructor. */
            if (mRefs->mWeak == 0){
                delete mRefs;
            }
        }
    }
    // for debugging purposes, clear this.
    const_cast<weakref_impl*&>(mRefs) = NULL;
}

void RefBase::extendObjectLifetime(int32_t mode)
{
    linux_atomic_or(mode,&mRefs->mFlags);
}

void RefBase::onFirstRef()
{
}

void RefBase::onLastStrongRef(const void* /*id*/)
{
}

bool RefBase::onIncStrongAttempted(uint32_t flags, const void* /*id*/)
{
    return (flags & FIRST_INC_STRONG) ? true : false;
}

void RefBase::onLastWeakRef(const void* /*id*/)
{
}

} 
