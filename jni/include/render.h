/*!
 * \file render.h
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#ifndef INCLUDE_RENDER_H_
#define INCLUDE_RENDER_H_

#include "audioOutput.h"
#include "VideoRender2.h"
#include "SubtitleRender.h"
#include "LinkedQueue.h"
#include "include/logger.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

enum Render_Status
{
    Render_None = -1,
    Render_Initialized,
    Render_Paused,
    Render_Stoped,
    Render_uninitialized,
};

typedef struct _renderParam
{
    //aduio
    int sampleReate;
    int channels;
    int bufferFrames;

    //Subtitle
    subtitileInfo subInfo;
    unsigned int width;
    unsigned int height;

    LinkedQueue *videoFrameQueue;
    LinkedQueue *audioFrameQueue;
    LinkedQueue *subtitleFrameQueue;
} RenderParam;

typedef struct _render
{
    LinkedQueue *videoFrameQueue;
    LinkedQueue *audioFrameQueue;
    LinkedQueue *subtitleFrameQueue;

    Render2 *videoRender;
    AudioOutPut *audioRender;
    SubtitleRender *subtitleRender;

    bool isInterruptRenderThread;
    ThreadContext *threadCtx;
    int status;

    int (*initialize)(struct _render * render, RenderParam * param);
    int (*startRender)(struct _render * render);
    int (*pauseRender)(struct _render * render);
    int (*resumeRender)(struct _render * render);
    int (*stopRender)(struct _render * render);
    void (*uninitialize)(struct _render * render);
} Render, *Render_ptr;

Render * createRender();

void destroyRender(Render ** ppRender);

#endif /* INCLUDE_RENDER_H_ */
