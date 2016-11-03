#ifndef _WIN32THREAD_H__
#define _WIN32THREAD_H__

#include <windows.h>

/* the following macro is used within cavs */
#undef ERROR

typedef struct {
    void *handle;
    void *(*func)(void *arg);
    void *arg;
    void *ret;
} avs2_pthread_t;
#define avs2_pthread_attr_t int

/* the conditional variable api for windows 6.0+ uses critical sections and not mutexes */
typedef CRITICAL_SECTION avs2_pthread_mutex_t;
#define AVS2_PTHREAD_MUTEX_INITIALIZER {0}
#define avs2_pthread_mutexattr_t int
#define pthread_exit(a)
/* This is the CONDITIONAL_VARIABLE typedef for using Window's native conditional variables on kernels 6.0+.
 * MinGW does not currently have this typedef. */
typedef struct {
    void *ptr;
} avs2_pthread_cond_t;
#define avs2_pthread_condattr_t int

int avs2_pthread_create(avs2_pthread_t *thread, const avs2_pthread_attr_t *attr,
                        void *(*start_routine)(void *), void *arg);
int avs2_pthread_join(avs2_pthread_t thread, void **value_ptr);

int avs2_pthread_mutex_init(avs2_pthread_mutex_t *mutex, const avs2_pthread_mutexattr_t *attr);
int avs2_pthread_mutex_destroy(avs2_pthread_mutex_t *mutex);
int avs2_pthread_mutex_lock(avs2_pthread_mutex_t *mutex);
int avs2_pthread_mutex_unlock(avs2_pthread_mutex_t *mutex);

int avs2_pthread_cond_init(avs2_pthread_cond_t *cond, const avs2_pthread_condattr_t *attr);
int avs2_pthread_cond_destroy(avs2_pthread_cond_t *cond);
int avs2_pthread_cond_broadcast(avs2_pthread_cond_t *cond);
int avs2_pthread_cond_wait(avs2_pthread_cond_t *cond, avs2_pthread_mutex_t *mutex);
int avs2_pthread_cond_signal(avs2_pthread_cond_t *cond);

#define avs2_pthread_attr_init(a) 0
#define avs2_pthread_attr_destroy(a) 0

int  avs2_win32_threading_init(void);
void avs2_win32_threading_destroy(void);

int avs2_pthread_num_processors_np(void);

#endif
