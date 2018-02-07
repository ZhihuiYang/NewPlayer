/*!
 * \file render.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#include "render.h"

static int initialize(struct _render * render, RenderParam * param)
{

}

static int startRendering(struct _render * render)
{
}

static int pauseRendering(struct _render * render)
{
}

static int resumeRendering(struct _render * render)
{
}

static int stopRendering(struct _render * render)
{
}

static void uninitialize(struct _render * render)
{
}

Render * createRender()
{
    Render * pRender = NULL;
    pRender = (Render *) malloc(sizeof(Render));
    if (NULL != pRender)
    {
        memset(pRender, 0, sizeof(Render));
        pRender->initialize;
        pRender->startRender = startRendering;
        pRender->pauseRender = pauseRendering;
        pRender->renderFrame = resumeRendering;
        pRender->stopRender = stopRendering;
        pRender->uninitialize = uninitialize;

        pRender->videoRender = createRender2();
        pRender->audioRender = createAudioOutPut();
        pRender->subtitleRender = createSubRenderer();

        decoder->isInterruptDecoderThread = false;
        ThreadContext *tCtx = NULL;
        tCtx = (ThreadContext *) malloc(sizeof(ThreadContext));
        memset(tCtx, 0, sizeof(ThreadContext));
        tCtx->tid = -1;
        pthread_mutex_init(&tCtx->mutex, NULL);
        pthread_cond_init(&tCtx->cond, NULL);
        tCtx->status = PAUSED;

        pRender->threadCtx = tCtx;
        pRender->status = Render_Initialized;

    }

    return pRender;
}

void destroyRender(Render ** ppRender)
{
    if (ppRender == NULL || *ppRender == NULL)
    {
        return;
    }

    Render *pRender = *ppRender;
    free(pRender);
    pRender = NULL;
    return;
}
