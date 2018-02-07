/*!
 * \file openslesPlaydemo.cpp
 *
 * \date Created on: Mar 8, 2017
 * \author: eyngzui
 */

#include "openslesPlaydemo.h"
#include "logger.h"
#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class PlaybackThread;

static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

static SLObjectItf outputMixObject = NULL;

static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLVolumeItf bqPlayerVolume;
static PlaybackThread mThread;

class PlaybackThread
{
    
private:
    FILE *mFile;
    void* mBuffer;
    size_t mSize;
    bool read;

public:
    PlaybackThread(const char* url)
            :
              mFile(NULL),
              mBuffer(NULL),
              mSize(0),
              read(true)
    {
        mFile = fopen((char *) url, "r");
        if (NULL == mFile)
        {
            LOG_ERROR("open file: %s error", url);
            return;
        }
        
        mBuffer = malloc(8192);
    }
    
    void start()
    {
        if (NULL == mFile)
        {
            return;
        }
        
        enqueneBuffer();
    }
    
    void release()
    {
        if (mFile != NULL)
        {
            fclose(mFile);
            mFile = NULL;
        }
        
        if (mBuffer != NULL)
        {
            free(mBuffer);
            mBuffer = NULL;
        }
    }
    
    ~PlaybackThread()
    {
        release();
    }
    
    void enqueneBuffer()
    {
        if (bqPlayerBufferQueue == NULL)
        {
            return;
        }
        
        while (true)
        {
            if (read)
            {
                mSize = fread(mBuffer, 1, 8192, mFile);
            }
            
            if (mSize > 0)
            {
                SLresult result;
                result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                        mBuffer, mSize);
                if (result == SL_RESULT_BUFFER_INSUFFICIENT)
                {
                    read = false;
                    return;
                }
                read = true;
            }
            else
            {
                return;
            }
        }
    }
    
    static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
        if (NULL == context)
        {
            LOG_ERROR("context is NULL");
            return;
        }
        
        PlaybackThread * thread = (PlaybackThread *) context;
        if (thread != NULL)
        {
            thread->enqueneBuffer();
        }
    }
}
;

void createEngine()
{
    SLresult result;
    
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("slCreateEngine fail");
        return;
    }
    
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("engineObject realize fail");
        return;
    }
    
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
            &engineEngine);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("engineObject GetInterface fail");
        return;
    }
    
    result = (*engineEngine)->CreateObject(engineEngine, &outputMixObject, 0, 0,
            0);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("engineEngine create outputMixObject fail");
        return;
    }
    
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("outputMixObject realize fail");
        return;
    }
}

bool createAudioPlayer(char * url)
{
    SLresult result;
    
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
            { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 3 };
    SLDataFormat_PCM format_pcm =
            { SL_DADAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                    SL_PCMSAMPLEFORMAT_FIXED_16,
                    SL_PCMSAMPLEFORMAT_FIXED_16,
                    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                    SL_BYTEORDER_LITTLEENDIAN };
    SLDataSource audioSrc =
            { &loc_bufq, &format_pcm };
    
    SLDataLocator_OutputMix loc_outmix =
            { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
    SLDataSink audioSink =
            { &loc_outmix, NULL };
    
    const SLInterfaceID ids[3] =
            { SL_IID_BUFFERQUEUE, SL_EFFECTSEND, SL_IID_VOLUME };
    const SLboolean req[3] =
            { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject,
            &audioSrc, &audioSink, 3, ids, req);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("engineEngine CreateAudioPlayer fail");
        return false;
    }
    
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject Realize fail");
        return false;
    }
    
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
            &bqPlayerPlay);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject play GetInterface fail");
        return false;
    }
    
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject bufferqueue GetInterface fail");
        return false;
    }
    
    mThread = new PlaybackThread(url);
    
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
            PlaybackThread::playerCallback, mThread);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject bufferqueue RegisterCallback fail");
        return false;
    }
    
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
            &bqPlayerEffectSend);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject effectsend GetInterface fail");
        return false;
    }
    
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME,
            &bqPlayerVolume);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject volume GetInterface fail");
        return false;
    }
    
    result = (*bqPlayerObject)->SetPlayState(bqPlayerObject,
            SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("bqPlayerObject SetPlayState playing fail");
        return false;
    }
    
    mThread->start();
    return true;
}

void shutdown()
{
    //destory player object  
    if (bqPlayerObject != NULL)
    {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerVolume = NULL;
    }
    
    // destroy output mix object, and invalidate all associated interfaces  
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }
    
    // destroy engine object, and invalidate all associated interfaces  
    if (engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    
    if (mThread != NULL)
    {
        delete mThread;
        mThread = NULL;
    }
}
