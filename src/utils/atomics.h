/*
 * Androusb
 * Copyright (c) 2014-2017 saki t_saki@serenegiant.com
 * Distributed under the terms of the GNU Lesser General Public License (LGPL v3.0) License.
 * License details are in the file license.txt, distributed as part of this software.
 */

#ifndef FLIGHTDEMO_ATOMICS_H
#define FLIGHTDEMO_ATOMICS_H

#include <sys/cdefs.h>
#include <sys/time.h>

__BEGIN_DECLS

#define __ATOMIC_INLINE__ static __inline__ __attribute__((always_inline))

__ATOMIC_INLINE__ int
__atomic_cmpxchg(int old, int _new, volatile int *ptr)
{
    /* We must return 0 on success */
    return __sync_val_compare_and_swap(ptr, old, _new) != old;
}

__ATOMIC_INLINE__ int
__atomic_swap(int _new, volatile int *ptr)
{
    int prev;
    do {
        prev = *ptr;
    } while (__sync_val_compare_and_swap(ptr, prev, _new) != prev);
    return prev;
}

__ATOMIC_INLINE__ int
__atomic_dec(volatile int *ptr)
{
  return __sync_fetch_and_sub (ptr, 1);
}

__ATOMIC_INLINE__ int
__atomic_inc(volatile int *ptr)
{
  return __sync_fetch_and_add (ptr, 1);
}

__ATOMIC_INLINE__ int
__atomic_add(int add, volatile int *ptr) {
	return __sync_fetch_and_add(ptr, add);
}

__ATOMIC_INLINE__ int
__atomic_sub(int sub, volatile int *ptr) {
	return __sync_fetch_and_sub(ptr, sub);
}

__ATOMIC_INLINE__ int
__atomic_or(int value, volatile int *ptr) {
/*	int32_t prev, tmp, status;
//	android_memory_barrier();
	do {
		__asm__ __volatile__ ("ldrex %0, [%4]\n"
							"orr %1, %0, %5\n"
							"strex %2, %1, [%4]"
							: "=&r" (prev), "=&r" (tmp),
							"=&r" (status), "+m" (*ptr)
							: "r" (ptr), "Ir" (value)
							: "cc");
	} while (__builtin_expect(status != 0, 0));
	return prev; */
	return __sync_fetch_and_or(ptr, value);
}

__END_DECLS

#endif //FLIGHTDEMO_ATOMICS_H
