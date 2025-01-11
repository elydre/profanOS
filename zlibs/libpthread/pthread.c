/*****************************************************************************\
|   === pthread.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of pthread functions from libpthread           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <pthread.h>
#include <profan.h>
#include <unistd.h>

#undef PROFAN_FNI
#define PROFAN_FNI 0

pthread_t pthread_self(void) {
    return NULL; // monothreaded ok
}

int pthread_atfork(void (*)(void), void (*)(void), void (*)(void)) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_destroy(pthread_attr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getstack(const pthread_attr_t *, void **, size_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getstacksize(const pthread_attr_t *, size_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getstackaddr(const pthread_attr_t *, void **) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getguardsize(const pthread_attr_t *, size_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getdetachstate(const pthread_attr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_init(pthread_attr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setstacksize(pthread_attr_t *, size_t) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setstack(pthread_attr_t *, void *, size_t) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setstackaddr(pthread_attr_t *, void *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setguardsize(pthread_attr_t *, size_t) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setdetachstate(pthread_attr_t *, int) {
    return (PROFAN_FNI, 0);
}

void pthread_cleanup_pop(int) {
    PROFAN_FNI;
}

void pthread_cleanup_push(void (*) (void *), void *routine_arg) {
    PROFAN_FNI;
}

int pthread_condattr_destroy(pthread_condattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_condattr_init(pthread_condattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_broadcast(pthread_cond_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_destroy(pthread_cond_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_signal(pthread_cond_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *, const struct timespec *) {
    return (PROFAN_FNI, 0);
}

int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *) {
    return (PROFAN_FNI, 0);
}

int pthread_detach(pthread_t) {
    return (PROFAN_FNI, 0);
}

int pthread_equal(pthread_t, pthread_t) {
    return (PROFAN_FNI, 0);
}

void pthread_exit(void *) {
    PROFAN_FNI;
    _exit(0); // avoid warning
}

void *pthread_getspecific(pthread_key_t) {
    return (PROFAN_FNI, NULL);
}

int pthread_join(pthread_t, void **) {
    return (PROFAN_FNI, 0);
}

int pthread_key_create(pthread_key_t *, void (*) (void *)) {
    return (PROFAN_FNI, 0);
}

int pthread_key_delete(pthread_key_t) {
    return (PROFAN_FNI, 0);
}

int pthread_kill(pthread_t, int) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_init(pthread_mutexattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_gettype(pthread_mutexattr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_settype(pthread_mutexattr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_destroy(pthread_mutex_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *) {
    return 0; // monothreaded ok
}


int pthread_mutex_lock(pthread_mutex_t *) {
    return 0; // monothreaded ok
}

int pthread_mutex_timedlock(pthread_mutex_t *, const struct timespec *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_trylock(pthread_mutex_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_unlock(pthread_mutex_t *) {
    return 0; // monothreaded ok
}

int pthread_once(pthread_once_t *, void (*) (void)) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_destroy(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_rdlock(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *, const struct timespec *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *, const struct timespec *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_unlock(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlock_wrlock(pthread_rwlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_setspecific(pthread_key_t, const void *) {
    return (PROFAN_FNI, 0);
}

int pthread_cancel(pthread_t) {
    return (PROFAN_FNI, 0);
}

int pthread_setcancelstate(int, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_setcanceltype(int, int *) {
    return (PROFAN_FNI, 0);
}

void pthread_testcancel(void) {
    PROFAN_FNI;
}

int pthread_getprio(pthread_t) {
    return (PROFAN_FNI, 0);
}

int pthread_setprio(pthread_t, int) {
    return (PROFAN_FNI, 0);
}

void pthread_yield(void) {
    PROFAN_FNI;
}

int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_getprioceiling(pthread_mutex_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutex_setprioceiling(pthread_mutex_t *, int, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_getprotocol(pthread_mutexattr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_condattr_getclock(const pthread_condattr_t *, clockid_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_condattr_setclock(pthread_condattr_t *, clockid_t) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getinheritsched(const pthread_attr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getschedparam(const pthread_attr_t *, struct sched_param *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getschedpolicy(const pthread_attr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_getscope(const pthread_attr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setinheritsched(pthread_attr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setschedparam(pthread_attr_t *, const struct sched_param *) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setschedpolicy(pthread_attr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_attr_setscope(pthread_attr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_getschedparam(pthread_t pthread, int *, struct sched_param *) {
    return (PROFAN_FNI, 0);
}

int pthread_setschedparam(pthread_t, int, const struct sched_param *) {
    return (PROFAN_FNI, 0);
}

int pthread_getconcurrency(void) {
    return (PROFAN_FNI, 0);
}

int pthread_setconcurrency(int) {
    return (PROFAN_FNI, 0);
}

int pthread_barrier_init(pthread_barrier_t *, pthread_barrierattr_t *, unsigned int) {
    return (PROFAN_FNI, 0);
}

int pthread_barrier_destroy(pthread_barrier_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_barrier_wait(pthread_barrier_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_barrierattr_init(pthread_barrierattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_barrierattr_getpshared(pthread_barrierattr_t *, int *) {
    return (PROFAN_FNI, 0);
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_spin_init(pthread_spinlock_t *, int) {
    return (PROFAN_FNI, 0);
}

int pthread_spin_destroy(pthread_spinlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_spin_trylock(pthread_spinlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_spin_lock(pthread_spinlock_t *) {
    return (PROFAN_FNI, 0);
}

int pthread_spin_unlock(pthread_spinlock_t *) {
    return (PROFAN_FNI, 0);
}
