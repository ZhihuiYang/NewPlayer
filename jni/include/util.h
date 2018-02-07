/*
 * util.h
 *
 *  Created on: Oct 14, 2016
 *      Author: elantia
 */

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <pthread.h>
#include "logger.h"

#define container_of(ptr, type, member) \
    ((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#define RUNING 1
#define PAUSED 0

typedef struct _threadContext
{
    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int status;
} ThreadContext;

void inline resumeThread(ThreadContext *ctx)
{
    if (NULL == ctx)
    {
        return;
    }

    if (PAUSED == ctx->status)
    {
        pthread_mutex_lock(&ctx->mutex);
        ctx->status = RUNING;
        pthread_cond_signal(&ctx->cond);
        LOG_DEBUG("resume thread");
        pthread_mutex_unlock(&ctx->mutex);
    }
    else
    {
        LOG_DEBUG("thread runs already");
    }
}

void inline waitThread(ThreadContext *ctx)
{
    pthread_mutex_lock(&ctx->mutex);
    while (!ctx->status)
    {
        pthread_cond_wait(&ctx->cond, &ctx->mutex);
    }
    pthread_mutex_unlock(&ctx->mutex);
}

void inline pauseThread(ThreadContext *ctx)
{
    if (NULL == ctx)
    {
        return;
    }

    if (RUNING == ctx->status)
    {
        pthread_mutex_lock(&ctx->mutex);
        ctx->status = PAUSED;
        LOG_DEBUG("pause thread");
        pthread_mutex_unlock(&ctx->mutex);
    }
    else
    {
        LOG_DEBUG("thread pause already");
    }
}

#endif /* INCLUDE_UTIL_H_ */
