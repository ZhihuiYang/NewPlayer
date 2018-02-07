/*
 * VideoRender2.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEORENDER2_H_
#define VIDEORENDER2_H_

#include <pthread.h>
#include <android/native_window.h>
#include "commonDef.h"
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#define RENDER_WINDOW_NUM 1

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef enum RenderMode2
{
    RenderType_2D,
    RenderType_VR
} RenderMode2;

typedef struct _shapeData
{
    GLfloat *vVertices;
    GLfloat *vTexCoord;
    GLfloat *vVertNormals;
    unsigned short *indices;
    int numIndices;
} ShapeData;

typedef struct _render2
{
    pthread_mutex_t mMutex;
    ANativeWindow* mWindow;
    int dataFormat;
    RenderMode2 mRenderMode;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    int mWidth;
    int mHeight;
    GLuint vShader;
    GLuint fShader;
    GLuint programObject;
    GLint YsamplerLoc;
    GLint UsamplerLoc;
    GLint VsamplerLoc;
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint isLeftLoc;
    GLint YtextureId;
    GLint UtextureId;
    GLint VtextureId;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    ShapeData sphereData;

    bool isVR;

    int (*renderFrame)(struct _render2 * render, void * frame);
    //int (*renderFrameWithMode)(struct _render2 * render, void * frame, RenderMode2 mode);
    int (*initialize)(struct _render2 * render);        //Initialize egl-* context
    int (*setWindow)(struct _render2 * render, void * window);
    int (*setMode)(struct _render2 * render, RenderMode2 mode);
    int (*updateOrientation)(struct _render2 * render, float diffx,
            float diffy,
            float diffz);
    int (*uninitialize)(struct _render2 * render);
} Render2, *Render2_ptr;

Render2 *createRender2();

void releaseRender2(Render2 **);

void setAssetManager(AAssetManager * assetMgr);

#endif /* VIDEORENDER2_H_ */
