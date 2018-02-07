/*!
 * \file VideoRender.cpp
 *
 * \date Created on: Oct 9, 2016
 * \author: eyngzui
 */

#include "include/VideoRender.h"
#include "include/logger.h"
#include <string.h>
#include "include/AVFrame.h"
#include <stdio.h>

#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

typedef unsigned short __u16;

static void ccvt_420p_rgb565(int width, int height, const unsigned char *src,
        __u16 *dst)
{
    int line, col, linewidth;
    int y, u, v, yy, vr, ug, vg, ub;
    int r, g, b;
    const unsigned char *py, *pu, *pv;
    
    linewidth = width >> 1;
    py = src;
    pu = py + (width * height);
    pv = pu + (width * height) / 4;
    
    y = *py++;
    yy = y << 8;
    u = *pu - 128;
    ug = 88 * u;
    ub = 454 * u;
    v = *pv - 128;
    vg = 183 * v;
    vr = 359 * v;
    
    for (line = 0; line < height; line++)
    {
        for (col = 0; col < width; col++)
        {
            r = (yy + vr) >> 8;
            g = (yy - ug - vg) >> 8;
            b = (yy + ub) >> 8;
            
            if (r < 0)
                r = 0;
            if (r > 255)
                r = 255;
            if (g < 0)
                g = 0;
            if (g > 255)
                g = 255;
            if (b < 0)
                b = 0;
            if (b > 255)
                b = 255;
            *dst++ = (((__u16 ) r >> 3) << 11) | (((__u16 ) g >> 2) << 5)
                    | (((__u16 ) b >> 3) << 0);
            
            y = *py++;
            yy = y << 8;
            if (col & 1)
            {
                pu++;
                pv++;
                
                u = *pu - 128;
                ug = 88 * u;
                ub = 454 * u;
                v = *pv - 128;
                vg = 183 * v;
                vr = 359 * v;
            }
        }
        
        if ((line & 1) == 0)
        {
            pu -= linewidth;
            pv -= linewidth;
        }
    }
}

static int renderFrameANativeWindow(struct _render * pRender, void * data)
{
    if (NULL == pRender || NULL == data)
    {
        LOG_ERROR("invalid parameters, Render: %p, data:%p", pRender,
                data);
        return -1;
    }
    ANativeWindow * window = pRender->window[0];
    Lisp_AVFrame * frame = (Lisp_AVFrame *) data;
    if (NULL == window)
    {
        LOG_ERROR("haven't provide window");
        return -1;
    }
    if (0
            > ANativeWindow_setBuffersGeometry(window, frame->width,
                    frame->height, WINDOW_FORMAT_RGB_565))
    {
        LOG_ERROR("unable to set buffer geometry");
        return -1;
    }
    
    ANativeWindow_Buffer windowBuffer;
    if (0 > ANativeWindow_lock(window, &windowBuffer, NULL))
    {
        LOG_ERROR("unable to lock native window");
        return -1;
    }
    
    ccvt_420p_rgb565(frame->width, frame->height,
            (const unsigned char *) frame->data[0],
            (__u16 *) windowBuffer.bits);
    if (0 > ANativeWindow_unlockAndPost(window))
    {
        LOG_ERROR("unable to unlock and post to native window");
        
        return -1;
    }
    return 0;
}

Render *createRender(void ** window, int count, int dataformat, int renderType)
{
    Render * pRender = NULL;
    pRender = (Render *) malloc(sizeof(Render));
    if (NULL == pRender)
    {
        LOG_ERROR("allocate render failed");
        return NULL;
    }
    
    memset(pRender, 0, sizeof(Render));
    pRender->dataFormat = dataformat;
    pRender->renderType = renderType;
    switch (pRender->renderType)
    {
        case RenderType_Cropping:
            pRender->renderFrame = renderFrameANativeWindow;
            break;
        case RenderType_Resizing:

        case RenderType_BlackBorders:

//        case RenderType_VR:
            goto unSupportType;
            break;
        default:
            pRender->renderFrame = renderFrameANativeWindow;
            break;
            
    }
    
    for (int i = 0; i < (RENDER_WINDOW_NUM > count ? count : RENDER_WINDOW_NUM);
            i++)
    {
        pRender->window[i] = (ANativeWindow *) window[i];
        ANativeWindow_acquire(pRender->window[i]);
    }
    
    goto exit;
    
    unSupportType:
    free(pRender);
    pRender = NULL;
    exit:
    return pRender;
}

void releaseRender(Render ** ppRender)
{
    if (NULL == ppRender || NULL == *ppRender)
    {
        return;
    }
    Render * pRender = *ppRender;
    for (int i = 0; i < RENDER_WINDOW_NUM; i++)
    {
        if (pRender->window[i])
        {
            ANativeWindow_release(pRender->window[i]);
        }
        pRender->window[i] = NULL;
        pRender->renderFrame = NULL;
    }
    free(pRender);
    pRender = NULL;
}
