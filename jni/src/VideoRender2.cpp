/*!
 * \file VideoRender2.cpp
 *
 * \date Created on: Oct 9, 2016
 * \author: eyngzui
 */

#include "include/VideoRender2.h"
#include "include/logger.h"
#include <string.h>
#include "include/AVFrame.h"
#include <stdio.h>
#include <math.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

#define ES_PI  (3.14159265f)

// Converts degrees to radians.
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)

// Converts radians to degrees.
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / M_PI)

char VERTEX_SHADER_VR_SRC[] = "ShaderVR.vsh";
char VERTEX_SHADER_SRC[] = "Shader.vsh";
char FRAGMENT_SHADER_SRC[] = "Shader.fsh";

static AAssetManager * gAssetMgr = NULL;

void setAssetManager(AAssetManager * assetMgr)
{
    gAssetMgr = assetMgr;
}

static int generateSphere(int numSlices, float radius, float **vertices,
        float **texCoords, uint16_t **indices, int *numVertices_out)
{
    int i;
    int j;
    int numParallels = numSlices / 2;
    int numVertices = (numParallels + 1) * (numSlices + 1);
    int numIndices = numParallels * numSlices * 6;
    float angleStep = (2.0f * M_PI) / ((float) numSlices);

    if (vertices != NULL)
        *vertices = (float *) malloc(sizeof(float) * 3 * numVertices);

    if (texCoords != NULL)
        *texCoords = (float *) malloc(sizeof(float) * 2 * numVertices);

    if (indices != NULL)
        *indices = (uint16_t *) malloc(sizeof(uint16_t) * numIndices);

    for (int i = 0; i < numParallels + 1; i++)
    {
        for (int j = 0; j < numSlices + 1; j++)
        {
            int vertex = (i * (numSlices + 1) + j) * 3;

            if (vertices)
            {
                (*vertices)[vertex + 0] = radius * sinf(angleStep * (float) i) *
                        sinf(angleStep * (float) j);
                (*vertices)[vertex + 1] = radius * cosf(angleStep * (float) i);
                (*vertices)[vertex + 2] = radius * sinf(angleStep * (float) i) *
                        cosf(angleStep * (float) j);
            }

            if (texCoords)
            {
                int texIndex = (i * (numSlices + 1) + j) * 2;
                (*texCoords)[texIndex + 0] = 1.0f
                        - (float) j / (float) numSlices;
                (*texCoords)[texIndex + 1] =
                        ((float) i / (float) numParallels);
            }
        }
    }

    if (indices != NULL)
    {
        uint16_t *indexBuf = (*indices);
        for (i = 0; i < numParallels; i++)
        {
            for (j = 0; j < numSlices; j++)
            {
                *indexBuf++ = i * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);

                *indexBuf++ = i * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
                *indexBuf++ = i * (numSlices + 1) + (j + 1);
            }
        }
    }

    if (numVertices_out)
    {
        *numVertices_out = numVertices;
    }

    return numIndices;
}

/**
 *
 * return a buffer which contains file content. this buffer need be free in your code after finish your using.
 **/
static char *textFileRead(char * filename)
{
    if (NULL == gAssetMgr)
    {
        LOG_ERROR("AAssetManager is NULL");
        return NULL;
    }

    AAsset* pAsset = AAssetManager_open(gAssetMgr, filename,
            AASSET_MODE_UNKNOWN);
    if (pAsset == NULL)
    {
        return NULL;
    }
    char *pSource = NULL;
    int size = AAsset_getLength(pAsset);
    if (size > 0)
    {
        pSource = (char *) malloc(size + 1);
        int ret = AAsset_read(pAsset, pSource, size);
        if (ret <= 0)
        {
            free(pSource);
            pSource = NULL;
            return pSource;
        }
        pSource[size] = '\0';
    }

    AAsset_close(pAsset);
    return pSource;
}

static GLuint loadShader(GLenum type, const char * shaderSrc)
{
    GLuint shader;
    GLint complied;

    shader = glCreateShader(type);
    if (shader == 0)
    {
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);

    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &complied);
    if (!complied)
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char * infoLog = (char *) malloc(sizeof(char *) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOG_ERROR("Error compiling shader : \n %s", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

/*
 *  return value:
 *      0 means load program failed.
 *      An integer number indicate a program object.
 * */
static GLuint loadProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint programObject;
    GLint linked;

    programObject = glCreateProgram();
    if (programObject == 0)
    {
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glLinkProgram(programObject);

    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char *infoLog = (char *) malloc(sizeof(char *) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            LOG_ERROR("Error linking program: \n %s", infoLog);
            free(infoLog);
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(programObject);
        return 0;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return programObject;
}

static int uninitialize(Render2_ptr pRender)
{
    glDeleteShader(pRender->vShader);
    glDeleteShader(pRender->fShader);
    glDeleteProgram(pRender->programObject);

    eglMakeCurrent(pRender->mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
            EGL_NO_CONTEXT);
    eglDestroyContext(pRender->mDisplay, pRender->mContext);
    eglDestroySurface(pRender->mDisplay, pRender->mSurface);
    eglTerminate(pRender->mDisplay);

    pRender->mDisplay = EGL_NO_DISPLAY;
    pRender->mSurface = EGL_NO_SURFACE;
    pRender->mContext = EGL_NO_CONTEXT;

    if (pRender->sphereData.vVertices != NULL)
    {
        free(pRender->sphereData.vVertices);
    }

    if (pRender->sphereData.vTexCoord != NULL)
    {
        free(pRender->sphereData.vTexCoord);
    }

    if (pRender->sphereData.vVertNormals != NULL)
    {
        free(pRender->sphereData.vVertNormals);
    }

    if (pRender->sphereData.indices != NULL)
    {
        free(pRender->sphereData.indices);
    }

    return 0;
}

static int initializeEGL(Render2_ptr pRender)
{
    const EGLint attribs[] =
            {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                    EGL_BLUE_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_RED_SIZE, 8,
                    EGL_NONE
            };

    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    EGLint error = EGL_SUCCESS;

    if (pRender->mWindow == NULL)
    {
        LOG_ERROR("nWindow is NULL, please setWindow first");
        return -1;
    }

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
    {
        LOG_ERROR("eglGetDisplay() return error %d", error = eglGetError());
        return error;
    }

    if (!eglInitialize(display, NULL, NULL))
    {
        LOG_ERROR("eglInitialize() return error %d", error = eglGetError());
        return error;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs))
    {
        LOG_ERROR("eglChooseConfig() return error %d", error =
                eglGetError());
        uninitialize(pRender);
        return error;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
    {
        LOG_ERROR("eglGetConfigAttrib() return error %d", error =
                eglGetError());
        uninitialize(pRender);
        return error;
    }

    ANativeWindow_setBuffersGeometry(pRender->mWindow, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, pRender->mWindow,
            NULL)))
    {
        LOG_ERROR("eglCreateWindowSurface() return error %d", error =
                eglGetError());
        uninitialize(pRender);
        return error;
    }

    EGLint contextAttrs[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };
    if (!(context = eglCreateContext(display, config, 0, contextAttrs)))
    {
        LOG_ERROR("eglCreateContext() return error %d", error =
                eglGetError());
        uninitialize(pRender);
        return error;
    }

    if (!eglMakeCurrent(display, surface, surface, context))
    {
        LOG_ERROR("eglMakeCurrent() return error %d", error = eglGetError());
        uninitialize(pRender);
        return error;
    }

    pRender->mDisplay = display;
    pRender->mSurface = surface;
    pRender->mContext = context;
    pRender->mWidth = ANativeWindow_getWidth(pRender->mWindow);
    pRender->mHeight = ANativeWindow_getHeight(pRender->mWindow);
    return error;
}

static int initialize(Render2_ptr pRender)
{
    GLuint vShader;
    GLuint fShader;
    GLuint yTextureId, uTextureId, vTextureId;
    if (EGL_SUCCESS != initializeEGL(pRender))
    {
        LOG_ERROR("initialize EGL failed");
        return -1;
    }

    char *vShaderStr = NULL;
    if (pRender->isVR)
    {
        vShaderStr = textFileRead(VERTEX_SHADER_VR_SRC);
    }
    else
    {
        vShaderStr = textFileRead(VERTEX_SHADER_SRC);
    }
    char *fShaderStr = textFileRead(FRAGMENT_SHADER_SRC);
    pRender->vShader = loadShader(GL_VERTEX_SHADER, vShaderStr);
    pRender->fShader = loadShader(GL_FRAGMENT_SHADER, fShaderStr);
    free(vShaderStr);
    free(fShaderStr);
    pRender->programObject = loadProgram(pRender->vShader, pRender->fShader);

    pRender->YsamplerLoc = glGetUniformLocation(pRender->programObject,
            "y_texture");

    pRender->UsamplerLoc = glGetUniformLocation(pRender->programObject,
            "u_texture");

    pRender->VsamplerLoc = glGetUniformLocation(pRender->programObject,
            "v_texture");

    if (pRender->isVR)
    {
        pRender->modelLoc = glGetUniformLocation(pRender->programObject,
                "model");
        pRender->viewLoc = glGetUniformLocation(pRender->programObject, "view");
        pRender->projectionLoc = glGetUniformLocation(pRender->programObject,
                "projection");
        pRender->isLeftLoc = glGetUniformLocation(pRender->programObject,
                "isLeft");
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &yTextureId);
    pRender->YtextureId = yTextureId;

    glGenTextures(1, &uTextureId);
    pRender->UtextureId = uTextureId;

    glGenTextures(1, &vTextureId);
    pRender->VtextureId = vTextureId;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    if (pRender->isVR)
    {

        pRender->sphereData.numIndices = generateSphere(300, 1.0f,
                &(pRender->sphereData.vVertices),
                &(pRender->sphereData.vTexCoord),
                &(pRender->sphereData.indices),
                &(pRender->sphereData.numIndices));

        pRender->model = glm::mat4(1.0f);
        pRender->view = glm::mat4(1.0f);
        pRender->projection = glm::perspective(45.0f,
                (GLfloat)(pRender->mWidth / 2.0) / (GLfloat) pRender->mHeight,
                0.1f, 10.0f);

    }
    return 0;
}

static int drawFrame(Render2_ptr pRender, void * data)
{

    GLfloat vVertices_2D[] =
            { -1.0f, 1.0f, 0.0f,
                    -1.0f, -1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
                    1.0f, -1.0f, 0.0f
            };

    GLfloat vTexCoord_2D[] =
            { 0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f };

    GLfloat *vVertices = NULL;

    GLfloat *vTexCoord = NULL;

    if (pRender->isVR)
    {
        vVertices = pRender->sphereData.vVertices;
        vTexCoord = pRender->sphereData.vTexCoord;
    }
    else
    {
        vVertices = vVertices_2D;
        vTexCoord = vTexCoord_2D;
    }

    Lisp_AVFrame * frame = (Lisp_AVFrame *) data;
    unsigned char *plane[3];
    plane[0] = frame->data[0];
    plane[1] = frame->data[1];
    plane[2] = frame->data[2];
    //plane[1] = plane[0] + frame->stride[0] * frame->height;
    // plane[2] = plane[1] + frame->stride[0] * frame->height / 4;

    glViewport(0, 0, pRender->mWidth, pRender->mHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(pRender->programObject);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
            vVertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2,
            vTexCoord);
    glEnableVertexAttribArray(1);

    //Y
    glActiveTexture (GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pRender->YtextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width, frame->height, 0,
            GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            plane[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(pRender->YsamplerLoc, 0);

    //U
    glActiveTexture (GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pRender->UtextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width / 2,
            frame->height / 2, 0, GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            plane[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(pRender->UsamplerLoc, 1);

    //V
    glActiveTexture (GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pRender->VtextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width / 2,
            frame->height / 2, 0, GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            plane[2]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(pRender->VsamplerLoc, 2);

    int scale = 1;
    if (pRender->isVR)
    {
        glUniformMatrix4fv(pRender->modelLoc, 1, GL_FALSE,
                glm::value_ptr(pRender->model));
        pthread_mutex_lock(&pRender->mMutex);
        glUniformMatrix4fv(pRender->viewLoc, 1, GL_FALSE,
                glm::value_ptr(pRender->view));
        pthread_mutex_unlock(&pRender->mMutex);
        glUniformMatrix4fv(pRender->projectionLoc, 1, GL_FALSE,
                glm::value_ptr(pRender->projection));

        glUniform1i(pRender->isLeftLoc, 0);
        glViewport(0, 0, pRender->mWidth * scale / 2.0,
                pRender->mHeight * scale);
        glDrawElements(GL_TRIANGLES, pRender->sphereData.numIndices,
                GL_UNSIGNED_SHORT,
                pRender->sphereData.indices);

        glUniform1i(pRender->isLeftLoc, 1);
        glViewport(pRender->mWidth * scale / 2.0, 0,
                pRender->mWidth * scale / 2.0, pRender->mHeight * scale);
        glDrawElements(GL_TRIANGLES, pRender->sphereData.numIndices,
                GL_UNSIGNED_SHORT,
                pRender->sphereData.indices);
    }
    else
    {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    if (!eglSwapBuffers(pRender->mDisplay, pRender->mSurface))
    {
        LOG_ERROR("eglSwapBuffers() returned error %d",
                eglGetError());
    }

    return 0;
}

int UpdateOrientation(Render2_ptr pRender, float diffx, float diffy,
        float diffz)
{
    if (pRender != NULL)
    {
        pthread_mutex_lock(&pRender->mMutex);
        pRender->view = glm::rotate(pRender->view,
                diffx,
                glm::vec3(0.0f, 1.0f, 0.0f));
        pRender->view = glm::rotate(pRender->view,
                diffy,
                glm::vec3(1.0f, 0.0f, 0.0f));
        pthread_mutex_unlock(&pRender->mMutex);
    }
}

static int setWindow(Render2_ptr pRender, void * window)
{
    if (NULL == pRender)
    {
        LOG_ERROR("invalid parameters, Render: %p", pRender);
        return -1;
    }

    if (NULL == window)
    {
        LOG_ERROR("invalid parameters, window: %p", window);
        return -2;
    }

    pRender->mWindow = (ANativeWindow *) window;
    return 0;
}
static int setMode(Render2_ptr pRender, RenderMode2 mode)
{
    if (NULL == pRender)
    {
        LOG_ERROR("invalid parameters, Render: %p", pRender);
        return -1;

    }

    if (mode != RenderType_2D && mode != RenderType_VR)
    {
        LOG_ERROR(
                "mode error, just RenderType_2D or RenderType_VR be supported");
        return -2;
    }

    pRender->isVR = false;
    pRender->mRenderMode = mode;
    if (pRender->mRenderMode == RenderType_VR)
    {
        pRender->isVR = true;
    }
    return 0;
}

static int renderFrameVR(Render2_ptr pRender, void * data)
{
    drawFrame(pRender, data);
    return 0;
}

static int renderFrame(Render2_ptr pRender, void * data)
{
    if (NULL == pRender || NULL == data)
    {
        LOG_ERROR("invalid parameters, Render: %p, data:%p", pRender,
                data);
        return -1;
    }
    switch (pRender->mRenderMode)
    {
        case RenderType_2D:
            default:
            drawFrame(pRender, data);
            break;
        case RenderType_VR:
            renderFrameVR(pRender, data);
            break;
    }

    return 0;
}

Render2 * createRender2()
{
    Render2 * pRender = NULL;
    pRender = (Render2 *) malloc(sizeof(Render2));
    if (NULL == pRender)
    {
        LOG_ERROR("allocate render failed");
        return NULL;
    }

    memset(pRender, 0, sizeof(Render2));
    pRender->initialize = initialize;
    pRender->renderFrame = renderFrame;
    //pRender->renderFrameWithMode = renderFrameWithMode;
    pRender->setMode = setMode;
    pRender->setWindow = setWindow;
    pRender->uninitialize = uninitialize;
    pRender->updateOrientation = UpdateOrientation;
    pRender->isVR = false;
    pRender->mRenderMode = RenderType_2D;
    pthread_mutex_init(&pRender->mMutex, 0);
    return pRender;
}

void releaseRender2(Render2 ** ppRender)
{

    if (NULL == ppRender || NULL == *ppRender)
    {
        return;
    }
    Render2 *pRender = *ppRender;
    pRender->uninitialize(pRender);

    //ANativeWindow_release(pRender->mWindow);

    pRender->initialize = NULL;
    pRender->setMode = NULL;
    pRender->setWindow = NULL;
    pRender->renderFrame = NULL;
    //pRender->renderFrameWithMode = NULL;
    pRender->uninitialize = NULL;
    pthread_mutex_destroy(&pRender->mMutex);
    free(pRender);
    pRender = NULL;
}
