/*
 * VideoRender.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEORENDER_H_
#define VIDEORENDER_H_

#include <android/native_window.h>
#include "commonDef.h"

#define RENDER_WINDOW_NUM 1
#define OPENGLES

#ifdef OPENGLES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

enum RenderMode
{
    RenderType_Cropping,
    RenderType_Resizing,
    RenderType_BlackBorders,
};

typedef struct _render
{
    ANativeWindow * window[RENDER_WINDOW_NUM];
    int dataFormat;
    int renderType;
    int dstWidth;
    int dstHeight;

#ifdef OPENGLES
    GLfloat positionCoordinates[8];
    EGLDisplay EglDisplay;
    EGLSurface EglSurface;
    EGLContext EglContext;
    GLuint program;
    GLuint id_y, id_u, id_v;
    GLuint textureUniformY, textureUniformU, textureUniformV;

#endif
    
    int (*renderFrame)(struct _render * render, void * frame);
} Render, *Render_ptr;

Render *createRender(void ** window, int count, int dataformat, int renderType);

void releaseRender(Render **);

#endif /* VIDEORENDER_H_ */
