#ifndef LINUX_REF_BASE_H
#define LINUX_REF_BASE_H

#include <cutils/atomic.h>

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <utils/StrongPointer.h>

#define LOGD printf

namespace linuxSys{

template <class T>
class LightRefBase
{
public:
    inline LightRefBase():mCount(0) {}
    inline void incStrong(__attribute__((unused)) const void* id) const {
        LOGD("[LightRefBase] incStrong\n");
        linux_atomic_inc(&mCount);
    }
    inline void decStrong(__attribute__((unused)) const void* id) const {
        LOGD("[LightRefBase] decStrong\n");
        if (linux_atomic_dec(&mCount)==1){
            LOGD("[LightRefBase] decStrong delete object!\n");
            delete static_cast<const T*>(this);
        }
    }

    //! DEBUGGING ONLY: Get current strong ref count.
    inline int32_t getStrongCount() const{
        return mCount;
    }

    typedef LightRefBase<T> basetype;

protected:
    inline ~LightRefBase() {}

private:
    //friend class ReferenceMover;
    //inline static void renameRefs(size_t n, const ReferenceRenamer& renamer) { }
    //inline static void renameRefId(T* ref, const void* old_id, const void* new_id) { }

private:
    mutable volatile int32_t mCount;
};

class RefBase
{
public:
    void incStrong(const void* id) const;
    void decStrong(const void* id) const;

    void froceIncStrong(const void* id) const;

    //! DEBUGGING ONLY:Get current strong ref count.
    int32_t getStrongCount() const;

    class weakref_type
    {
    public:
        RefBase* refBase() const;

        void incWeak(const void* id);
        void decWeak(const void* id);

        // acquires a strong reference if there is already one.
        bool attemptIncStrong(const void* id);

        // acquires a weak reference if there is already one.
        // This is not always safe.
        bool attemptIncWeak(const void* id);

        //! DEBUGGING ONLY: Get current weak ref count.
        int32_t getWeakCount() const;

        //! DEBUGGING ONLY: Print reference held on object.
        void printRefs() const;

        //! DEBUGGING ONLY:Enable tracking for this object.
        // enable -- enable/disable tracking
        // retain -- when tracking is enable,if true,then we save a stack trace
        //      for each reference and dereference; when retain == false,we
        //      match up references and dereferences and keep only the
        //      outstanding ones. 
        void trackMe(bool enable, bool retain);
    };

    weakref_type* createWeak(const void* id) const;
    weakref_type* getWeakRefs() const;

    //! DEBUGGING ONLY:print references held on object.
    inline void printRefs() const { getWeakRefs()->printRefs(); }

    //! DEBUGGING ONLY:enable tracking of object.
    inline void trackMe(bool enable, bool retain){
        getWeakRefs()->trackMe(enable, retain);
    }

    typedef RefBase basetype;

protected:
    RefBase();
    virtual ~RefBase();

    //! Flags for extendObjectLifetime()
    enum {
        OBJECT_LIFETIME_STRONG = 0x0000,
        OBJECT_LIFETIME_WEAK   = 0x0001,
        OBJECT_LIFETIME_MASK   = 0x0001
    };

    void extendObjectLifetime(int32_t mode);

    //!Flag for onIncStrongAttempted()
    enum {
        FIRST_INC_STRONG = 0x0001
    };

    /* Strong pointer control, create object before the call */
    virtual void onFirstRef();
    /* Strong pointer control, called before the delete object */
    virtual void onLastStrongRef(const void* id);
    /* You can override this function onIncStrongAttempted(),
       weak pointer into a strong pointer is prohibited, need return flase */
    virtual bool onIncStrongAttempted(uint32_t flags, const void* id); 
    /* A weak pointer control, called before the delete object */
    virtual void onLastWeakRef(const void* id);

private:
    friend class weakref_type;
    class weakref_impl;

    RefBase(const RefBase& o);
    RefBase& operator=(const RefBase& o);

private:
    //friend class ReferenceMover;
    //static void renameRefs(size_t n, const ReferenceRenamer& renamer);
    //static void renameRefId(weakref_type* ref, const void* old_id, const void* new_id);
    //static void renameRefId(RefBase* ref, const void* old_id, const void* new_id);

    weakref_impl* const mRefs;
};

//------------------------------------------------------------------------

#define COMPARE_WEAK(_op_)                                      \
inline bool operator _op_ (const sp<T>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
inline bool operator _op_ (const T* o) const {                  \
    return m_ptr _op_ o;                                        \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const sp<U>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const U* o) const {                  \
    return m_ptr _op_ o;                                        \
}
//--------------------------------------------------------------------------
template <typename T>
class wp
{
public:
    typedef typename RefBase::weakref_type weakref_type;

    inline wp() : m_ptr(0) {}

    wp(T* other);
    wp(const wp<T>& other);
    wp(const sp<T>& other);
    template<typename U> wp(U* other);
    template<typename U> wp(const sp<U>& other);
    template<typename U> wp(const wp<U>& other);

    ~wp();

    //Assignment
    wp& operator= (T* other);
    wp& operator= (const wp<T>& other);
    wp& operator= (const sp<T>& other);

    template<typename U> wp& operator= (U* other);
    template<typename U> wp& operator= (const wp<U>& other);
    template<typename U> wp& operator= (const sp<U>& other);

    void set_object_and_refs(T* other, weakref_type* refs);

    // promotion to sp
    sp<T> promote() const;

    //Reset
    void clear();

    //Accessors
    inline weakref_type* get_refs() const { return m_refs; }
    inline T* unsafe_get() const { return m_ptr; }

    //Operator
    COMPARE_WEAK(==)
    COMPARE_WEAK(!=)
    COMPARE_WEAK(>)
    COMPARE_WEAK(<)
    COMPARE_WEAK(>=)
    COMPARE_WEAK(<=)

    inline bool operator == (const wp<T>& o) const {
        return (m_ptr == o.m_ptr) && (m_refs == o.m_refs);
    }
    template<typename U>
    inline bool operator == (const wp<U>& o) const {
        return m_ptr == o.m_ptr;
    }

    inline bool operator > (const wp<T>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs > o.m_refs) : (m_ptr > o.m_ptr);
    }
    template<typename U>
    inline bool operator > (const wp<U>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs > o.m_refs) : (m_ptr > o.m_ptr);
    }

    inline bool operator < (const wp<T>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs < o.m_refs) : (m_ptr < o.m_ptr);
    }
    template<typename U>
    inline bool operator < (const wp<U>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs < o.m_refs) : (m_ptr < o.m_ptr);
    }
                         inline bool operator != (const wp<T>& o) const { return m_refs != o.m_refs; }
    template<typename U> inline bool operator != (const wp<U>& o) const { return !operator == (o); }
                         inline bool operator <= (const wp<T>& o) const { return !operator > (o); }
    template<typename U> inline bool operator <= (const wp<U>& o) const { return !operator > (o); }
                         inline bool operator >= (const wp<T>& o) const { return !operator < (o); }
    template<typename U> inline bool operator >= (const wp<U>& o) const { return !operator < (o); }

private:
    template<typename Y> friend class sp;
    template<typename Y> friend class wp;

    T* m_ptr;
    weakref_type* m_refs;
};

#undef COMPARE_WEAK
//------------------------------------------------------------------------

template<typename T>
wp<T>::wp(T* other)
    : m_ptr(other)
{
    if (other)
        m_refs = other->createWeak(this);
}

template<typename T>
wp<T>::wp(const wp<T>& other)
    : m_ptr(other.m_ptr),
      m_refs(other.m_refs)
{
    if (m_ptr) 
        m_refs->incWeak(this);
}

template<typename T>
wp<T>::wp(const sp<T>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr)
        m_refs = m_ptr->createWeak(this);
}

template<typename T> template<typename U>
wp<T>::wp(U* other)
    : m_ptr(other)
{
    if (other)
        m_refs = other->createWeak(this);
}

template<typename T> template<typename U>
wp<T>::wp(const wp<U>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr) {
        m_refs = other->m_refs;
        m_refs->incWeak(this);
    }
}

template<typename T> template<typename U>
wp<T>::wp(const sp<U>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr)
        m_refs = m_ptr->createWeak(this);
}

template<typename T> 
wp<T>::~wp()
{
    if (m_ptr)
        m_refs->decWeak(this);
}

template<typename T>
wp<T>& wp<T>::operator= (T*other)
{
    weakref_type* newRefs = 
        other ? other->createWeak(this):0;
    if (m_ptr)
        m_refs->decWeak(this);
    m_ptr = other;
    m_refs= newRefs;
    return *this;
}

template<typename T>
wp<T>& wp<T>::operator= (const wp<T>& other)
{
    weakref_type* otherRefs(other.m_refs);
    T* otherPtr(other.m_ptr);
    if (m_ptr)
        m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs= otherRefs;
    return *this; 
}

template<typename T>
wp<T>& wp<T>::operator= (const sp<T>& other)
{
    weakref_type* newRefs = 
        other != NULL ? other->createWeak(this):0;
    T* otherPtr(other.m_ptr);
    if (m_ptr)
        m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs= newRefs;
    return *this;
}

template<typename T> template<typename U>
wp<T>& wp<T>::operator = (U* other)
{
    weakref_type* newRefs =
        other ? other->createWeak(this) : 0;
    if (m_ptr) 
        m_refs->decWeak(this);
    m_ptr = other;
    m_refs = newRefs;
    return *this;
}

template<typename T> template<typename U>
wp<T>& wp<T>::operator = (const wp<U>& other)
{
    weakref_type* otherRefs(other.m_refs);
    U* otherPtr(other.m_ptr);
    if (otherPtr) otherRefs->incWeak(this);
    if (m_ptr) 
        m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = otherRefs;
    return *this;
}

template<typename T> template<typename U>
wp<T>& wp<T>::operator = (const sp<U>& other)
{
    weakref_type* newRefs =
        other != NULL ? other->createWeak(this) : 0;
    U* otherPtr(other.m_ptr);
    if (m_ptr) 
        m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = newRefs;
    return *this;
}

template<typename T>
void wp<T>::set_object_and_refs(T* other, weakref_type* refs)
{
    if (other)
        refs->incWeak(this);
    if (m_ptr)
        m_refs->decWeak(this);
    m_ptr = other;
    m_refs= refs;
}

template<typename T>
sp<T> wp<T>::promote() const
{
    sp<T> result;
    if (m_ptr && m_refs->attemptIncStrong(&result)) {
        result.set_pointer(m_ptr);
    }
    return result;
}

template<typename T>
void wp<T>::clear()
{
    if (m_ptr) {
        m_refs->decWeak(this);
        m_ptr = 0;
    }
}

}
#endif