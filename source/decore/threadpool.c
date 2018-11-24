#include <stdlib.h>
#include <assert.h>
#include "threadpool.h"

/* ---------------------------------------------------------------------------
 * multi line macros
 */
#if !defined(_WIN32)
#define __pragma(a)
#endif
#include <string.h>
#define MULTI_LINE_MACRO_BEGIN  do {
#define MULTI_LINE_MACRO_END \
    __pragma(warning(push))\
    __pragma(warning(disable:4127))\
    } while(0)\
    __pragma(warning(pop))

#define CHECKED_MALLOC(var, type, size)\
    MULTI_LINE_MACRO_BEGIN\
    (var) = (type)malloc(size);\
    if ((var) == NULL) {\
        goto fail;\
    }\
    MULTI_LINE_MACRO_END
#define CHECKED_MALLOCZERO(var, type, size)\
    MULTI_LINE_MACRO_BEGIN\
    CHECKED_MALLOC(var, type, size);\
    memset(var, 0, size);\
    MULTI_LINE_MACRO_END

#define XCHG(type, a, b)\
    MULTI_LINE_MACRO_BEGIN\
    type __tmp = (a); (a) = (b); (b) = __tmp;\
    MULTI_LINE_MACRO_END


/**
 * ===========================================================================
 * function defines
 * ===========================================================================
 */

/* ---------------------------------------------------------------------------
 */
avs2_threadpool_job_t *avs2_frame_shift(avs2_threadpool_job_t **list)
{
    avs2_threadpool_job_t *job = list[0];
    int i;
    for (i = 0; list[i]; i++) {
        list[i] = list[i+1];
    }
    assert(job);
    return job;
}


/* ---------------------------------------------------------------------------
 */
void avs2_frame_delete(avs2_threadpool_job_t *job)
{
    avs2_pthread_mutex_destroy(&job->mutex);
    avs2_pthread_cond_destroy(&job->cv);
    free(job);
}
/* ---------------------------------------------------------------------------
 */
void avs2_frame_delete_list(avs2_threadpool_job_t **list)
{
    int i = 0;
    if (!list) {
        return;
    }
    while (list[i]) {
        avs2_frame_delete(list[i++]);
    }
    free(list);
}

/* ---------------------------------------------------------------------------
 */
void avs2_sync_frame_list_delete(avs2_threadpool_job_list_t *slist)
{
    avs2_pthread_mutex_destroy(&slist->mutex);
    avs2_pthread_cond_destroy(&slist->cv_fill);
    avs2_pthread_cond_destroy(&slist->cv_empty);
    avs2_frame_delete_list(slist->list);
}

/* ---------------------------------------------------------------------------
 */
void avs2_sync_frame_list_push(avs2_threadpool_job_list_t *slist, avs2_threadpool_job_t *job)
{
    avs2_pthread_mutex_lock(&slist->mutex);
    while (slist->i_size == slist->i_max_size) {
        avs2_pthread_cond_wait(&slist->cv_empty, &slist->mutex);
    }
    slist->list[slist->i_size++] = job;
    avs2_pthread_cond_broadcast(&slist->cv_fill);
    avs2_pthread_mutex_unlock(&slist->mutex);
}

/* ---------------------------------------------------------------------------
 */
avs2_threadpool_job_t *avs2_sync_frame_list_pop(avs2_threadpool_job_list_t *slist)
{
    avs2_threadpool_job_t *job;
    avs2_pthread_mutex_lock(&slist->mutex);
    while (!slist->i_size) {
        avs2_pthread_cond_wait(&slist->cv_fill, &slist->mutex);
    }
    job = slist->list[--slist->i_size];
    slist->list[slist->i_size] = NULL;
    avs2_pthread_cond_broadcast(&slist->cv_empty);
    avs2_pthread_mutex_unlock(&slist->mutex);
    return job;
}
/* ---------------------------------------------------------------------------
 */
int avs2_sync_frame_list_init(avs2_threadpool_job_list_t *slist, int max_size)
{
    if (max_size < 0) {
        return -1;
    }
    slist->i_max_size = max_size;
    slist->i_size = 0;
    CHECKED_MALLOCZERO(slist->list, avs2_threadpool_job_t **, (max_size + 1) * sizeof(avs2_threadpool_job_t *));
    if (avs2_pthread_mutex_init(&slist->mutex, NULL) ||
        avs2_pthread_cond_init(&slist->cv_fill, NULL) ||
        avs2_pthread_cond_init(&slist->cv_empty, NULL)) {
        return -1;
    }
    return 0;
fail:
    return -1;
}

static void avs2_threadpool_list_delete(avs2_threadpool_job_list_t *slist)
{
    int i;
    for (i = 0; slist->list[i]; i++) {
        free(slist->list[i]);
        slist->list[i] = NULL;
    }
    avs2_sync_frame_list_delete(slist);
}

static void avs2_threadpool_thread(avs2_threadpool_t *pool)
{
    if (pool->init_func) {
        pool->init_func(pool->init_arg);
    }

    while (!pool->exit) {
        avs2_threadpool_job_t *job = NULL;
        avs2_pthread_mutex_lock(&pool->run.mutex);
        while (!pool->exit && !pool->run.i_size) {
            avs2_pthread_cond_wait(&pool->run.cv_fill, &pool->run.mutex);
        }
        if (pool->run.i_size) {
            job = (void *)avs2_frame_shift(pool->run.list);
            pool->run.i_size--;
        }
        
        avs2_pthread_mutex_unlock(&pool->run.mutex);
        if (!job) {
            continue;
        }
        job->ret = job->func(job->arg);   /* execute the function */
        if (job->wait) {
            avs2_sync_frame_list_push(&pool->done, (void *)job);
        }
        else {
            avs2_sync_frame_list_push(&pool->uninit, (void *)job);
        }
        
    }

    pthread_exit(0);
}

int avs2_threadpool_init(avs2_threadpool_t **p_pool, int threads, void (*init_func)(void *), void *init_arg)
{
    int i;
    avs2_threadpool_t *pool;

    if (threads <= 0) {
        return -1;
    }

    CHECKED_MALLOCZERO(pool, avs2_threadpool_t *, sizeof(avs2_threadpool_t));
    *p_pool = pool;

    pool->init_func = init_func;
    pool->init_arg  = init_arg;
    pool->threads   = threads;

    CHECKED_MALLOC(pool->thread_handle, avs2_pthread_t *, pool->threads * sizeof(avs2_pthread_t));

    if (avs2_sync_frame_list_init(&pool->uninit, pool->threads) ||
        avs2_sync_frame_list_init(&pool->run, pool->threads) ||
        avs2_sync_frame_list_init(&pool->done, pool->threads)) {
        goto fail;
    }

    for (i = 0; i < pool->threads; i++) {
        avs2_threadpool_job_t *job;
        CHECKED_MALLOC(job, avs2_threadpool_job_t*, sizeof(avs2_threadpool_job_t));
        avs2_sync_frame_list_push(&pool->uninit, (void *)job);
    }
    for (i = 0; i < pool->threads; i++) {
        if (avs2_pthread_create(pool->thread_handle + i, NULL, (avs2_tfunc_a_t)avs2_threadpool_thread, pool)) {
            goto fail;
        }
    }

    return 0;
fail:
    return -1;
}

void avs2_threadpool_run(avs2_threadpool_t *pool, void *(*func)(void *), void *arg, int wait_sign)
{
    avs2_threadpool_job_t *job = (void *)avs2_sync_frame_list_pop(&pool->uninit);
    job->func = func;
    job->arg  = arg;
    job->wait = wait_sign;
    avs2_sync_frame_list_push(&pool->run, (void *)job);
}

void *avs2_threadpool_wait(avs2_threadpool_t *pool, void *arg)
{
    avs2_threadpool_job_t *job = NULL;
    int i;
    void *ret;

    avs2_pthread_mutex_lock(&pool->done.mutex);
    while (!job) {
        for (i = 0; i < pool->done.i_size; i++) {
            avs2_threadpool_job_t *t = (void *)pool->done.list[i];
            if (t->arg == arg) {
                job = (void *)avs2_frame_shift(pool->done.list + i);
                pool->done.i_size--;
            }
        }
        if (!job) {
            avs2_pthread_cond_wait(&pool->done.cv_fill, &pool->done.mutex);
        }
    }
    avs2_pthread_mutex_unlock(&pool->done.mutex);

    ret = job->ret;
    avs2_sync_frame_list_push(&pool->uninit, (void *)job);
    return ret;
}


void avs2_threadpool_delete(avs2_threadpool_t *pool)
{
    int i;

    avs2_pthread_mutex_lock(&pool->run.mutex);
    pool->exit = 1;
    avs2_pthread_cond_broadcast(&pool->run.cv_fill);
    avs2_pthread_mutex_unlock(&pool->run.mutex);
    for (i = 0; i < pool->threads; i++) {
        avs2_pthread_join(pool->thread_handle[i], NULL);
    }

    avs2_threadpool_list_delete(&pool->uninit);
    avs2_threadpool_list_delete(&pool->run);
    avs2_threadpool_list_delete(&pool->done);
    free(pool->thread_handle);
    free(pool);
}