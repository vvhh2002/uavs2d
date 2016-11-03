#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#ifdef   __cplusplus
extern "C"{
#endif

typedef void*(*avs2_tfunc_a_t)(void *);
typedef volatile long atom_t;   // 32 bits, signed

#if defined(_WIN32)
#include "win32thread.h"
#else

#pragma comment(lib, "pthreadVC2.lib")

#include <pthread.h>
#define avs2_pthread_t                pthread_t
#define avs2_pthread_create           pthread_create
#define avs2_pthread_join             pthread_join
#define avs2_pthread_mutex_t          pthread_mutex_t
#define avs2_pthread_mutex_init       pthread_mutex_init
#define avs2_pthread_mutex_destroy    pthread_mutex_destroy
#define avs2_pthread_mutex_lock       pthread_mutex_lock
#define avs2_pthread_mutex_unlock     pthread_mutex_unlock
#define avs2_pthread_cond_t           pthread_cond_t
#define avs2_pthread_cond_init        pthread_cond_init
#define avs2_pthread_cond_destroy     pthread_cond_destroy
#define avs2_pthread_cond_broadcast   pthread_cond_broadcast
#define avs2_pthread_cond_wait        pthread_cond_wait
#define avs2_pthread_attr_t           pthread_attr_t
#define avs2_pthread_attr_init        pthread_attr_init
#define avs2_pthread_attr_destroy     pthread_attr_destroy
#define avs2_pthread_num_processors_np pthread_num_processors_np
#define AVS2_PTHREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#endif


/**
 * ===========================================================================
 * type defines
 * ===========================================================================
 */
typedef struct 
{
    void *(*func)(void *);
    void *arg;
    void *ret;
    int wait;
    avs2_pthread_mutex_t mutex;
    avs2_pthread_cond_t  cv;
} avs2_threadpool_job_t;

typedef struct 
{
    avs2_threadpool_job_t   **list;
    int                     i_max_size;
    int                     i_size;
    avs2_pthread_mutex_t    mutex;
    avs2_pthread_cond_t     cv_fill;  /* event signaling that the list became fuller */
    avs2_pthread_cond_t     cv_empty; /* event signaling that the list became emptier */
} avs2_threadpool_job_list_t;

typedef struct avs2_threadpool_t
{
    int            exit;
    int            threads;
    avs2_pthread_t *thread_handle;
    void (*init_func)(void *);
    void           *init_arg;

    avs2_threadpool_job_list_t uninit; /* list of jobs that are awaiting use */
    avs2_threadpool_job_list_t run;    /* list of jobs that are queued for processing by the pool */
    avs2_threadpool_job_list_t done;   /* list of jobs that have finished processing */
} avs2_threadpool_t;

int   avs2_threadpool_init(avs2_threadpool_t **p_pool, int threads, void (*init_func)(void *), void *init_arg);
void  avs2_threadpool_run(avs2_threadpool_t *pool, void *(*func)(void *), void *arg, int wait_sign);
void *avs2_threadpool_wait(avs2_threadpool_t *pool, void *arg);
void  avs2_threadpool_delete(avs2_threadpool_t *pool);


#ifdef   __cplusplus
}
#endif

#endif /* __AVS2_THREADPOOL_H__ */
