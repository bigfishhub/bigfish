#ifndef CUTILS_ATOMIC_H
#define CUTILS_ATOMIC_H

#include <stdint.h>

#ifndef LINUX_ARM
#include <pthread.h>

static pthread_mutex_t atomicMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifndef LINUX_ATOMIC_INLINE  //强制内联
#define LINUX_ATOMIC_INLINE inline __attribute__((always_inline))
#endif

#define linux_atomic_cmpxchg linux_atomic_release_cas

#ifndef LINUX_ARM

extern LINUX_ATOMIC_INLINE void linux_memory_barrier()
{

}

extern LINUX_ATOMIC_INLINE int linux_atomic_cas(int32_t old_value, 
                            int32_t new_value, volatile int32_t *ptr)
{
    pthread_mutex_lock(&atomicMutex);
    if (old_value == *ptr)
    {
       *ptr = new_value;
       pthread_mutex_unlock(&atomicMutex);
       return 0;
    }
    pthread_mutex_unlock(&atomicMutex);
    return 1;
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_release_cas(int32_t old_value, 
                              int32_t new_value, volatile int32_t *ptr)
{
    return linux_atomic_cas(old_value,new_value,ptr);
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_add(int32_t increment, volatile int32_t *ptr)
{
    //TODO atomic operation
    pthread_mutex_lock(&atomicMutex);
    int32_t mid = *ptr;
    *ptr += increment;
    pthread_mutex_unlock(&atomicMutex);
    return mid;
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_inc(volatile int32_t *addr)
{
    return linux_atomic_add(1,addr);
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_dec(volatile int32_t *addr)
{
    return linux_atomic_add(-1,addr);
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_or(int32_t value, volatile int32_t *ptr)
{
  pthread_mutex_lock(&atomicMutex);
  int32_t mid = *ptr;
  *ptr = *ptr | value;
  pthread_mutex_unlock(&atomicMutex);
  return mid;
}

#else
extern LINUX_ATOMIC_INLINE void linux_memory_barrier()
{
/*#if ANDROID_SMP == 0
    android_compiler_barrier();
#else*/
    __asm__ __volatile__ ("dmb" : : : "memory");
//#endif
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_add(int32_t increment, volatile int32_t *ptr)
{
    int32_t prev, tmp, status;
    linux_memory_barrier();
    do {
        __asm__ __volatile__ ("ldrex %0, [%4]\n"
                              "add %1, %0, %5\n"
                              "strex %2, %1, [%4]"
                              : "=&r" (prev), "=&r" (tmp),
                                "=&r" (status), "+m" (*ptr)
                              : "r" (ptr), "Ir" (increment)
                              : "cc");
    } while (__builtin_expect(status != 0, 0));
    return prev;
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_inc(volatile int32_t *addr)
{
    return linux_atomic_add(1,addr);
}

extern LINUX_ATOMIC_INLINE int32_t linux_atomic_dec(volatile int32_t *addr)
{
    return linux_atomic_add(-1,addr);
}
#endif

#endif