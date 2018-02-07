/*!
 * \file audioOutPut.cpp
 *
 * \date Created on: Jul 7, 2017
 * \author: eyngzui
 */

#include "audioOutPut.h"
#include "logger.h"
#include <stdlib.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

static void* createThreadLock(void)
{
    ThreadLock *pLock;
    pLock = (ThreadLock *) malloc(sizeof(ThreadLock));
    
    if (pLock == NULL)
    {
        return NULL;
    }
    
    memset(pLock, 0, sizeof(ThreadLock));
    if (pthread_mutex_init(&(pLock->mutex), NULL) != 0)
    {
        free((void *) pLock);
        return NULL;
    }
    
    if (pthread_cond_init(&pLock->cond, NULL) != 0)
    {
        free((void *) p);
        return NULL;
    }
    
    pLock->status = (unsigned char) 1;
    
    return pLock;
}

static int waitTheadLock(ThreadLock * Lock)
{
    ThreadLock *pLock;
    int ret = 0;
    pLock = lock;
    pthread_mutex_lock(&pLock->mutex);
    while (!pLock->status)
    {
        pthread_cond_wait(&pLock->cond, &pLock->mutex);
    }
    
    pLock->status = (unsigned char) 0;
    pthread_mutex_unlock(&pLock->mutex);
}

static void notifyThreadLock(ThreadLock *lock)
{
    pthread_mutex_lock(&lock->mutex);
    lock->status = (unsigned char) 1;
    pthread_cond_signal(&lock->cond);
    pthread_mutex_unlock(&lock->mutex);
}

static void destroyThreadLock(ThreadLock *lock)
{
    if (lock == NULL)
    {
        return;
    }
    
    notifyThreadLock(lock);
    pthread_cond_destroy(&lock->cond);
    pthread_mutex_destroy(&lock->mutex);
    free(lock);
}

static SLresult createOpenSLESEngine(AudioOutPut * pAudioOutPut)
{
    SLresult result;
    
    result = slCreateEngine(&pAudioOutPut->engineObject, 0, NULL, 0, NULL,
            NULL);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("create opensles engine object failed");
        return result;
    }
    
    result = (*pAudioOutPut->engineObject)->Realize(pAudioOutPut->engineObject,
            SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("realize opensles engine failed");
        return result;
    }
    
    result = (*pAudioOutPut->engineObject)->GetInterface(
            pAudioOutPut->engineObject, SL_IID_ENGINE,
            &(pAudioOutPut->engineEngine));
    if (result != SL_RESULT_SUCCESS)
    {
        LOG_ERROR("get opensles engine interface failed");
        return result;
    }
    
    return result;
}

static SLresult OpenSLESOpenPlayer(AudioOutPut * pAudioOutPut)
{
    SLresult result;
    SLuint32 sampleRate = pAudioOutPut->sampleRate;
    SLuint32 channels = pAudioOutPut->outputChannels;
    
    if (channels)
    {
        SLDataLocator_AndroidSimpleBufferQueue locator_bufq =
                { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
        switch (sampleRate)
        {
            case 8000:
                sampleRate = SL_SAMPLINGRATE_8;
                break;
            case 11025:
                sampleRate = SL_SAMPLINGRATE_11_025;
                break;
            case 16000:
                sampleRate = SL_SAMPLINGRATE_16;
                break;
            case 22050:
                sampleRate = SL_SAMPLINGRATE_22_05;
                break;
            case 24000:
                sampleRate = SL_SAMPLINGRATE_24;
                break;
            case 32000:
                sampleRate = SL_SAMPLINGRATE_32;
                break;
            case 44100:
                sampleRate = SL_SAMPLINGRATE_44_1;
                break;
            case 48000:
                sampleRate = SL_SAMPLINGRATE_48;
                break;
            case 64000:
                sampleRate = SL_SAMPLINGRATE_64;
                break;
            case 88200:
                sampleRate = SL_SAMPLINGRATE_88_2;
                break;
            case 96000:
                sampleRate = SL_SAMPLINGRATE_96;
                break;
            case 192000:
                sampleRate = SL_SAMPLINGRATE_192;
                break;
            default:
                return -1;
                break;
        }
        
        const SLInterfaceID ids[] =
                { SL_IID_VOLUME };
        const SLboolean req[] =
                { SL_BOOLEAN_FALSE };
        
        result = (*pAudioOutPut->engineEngine)->CreateOutputMix(
                pAudioOutPut->engineEngine, &(pAudioOutPut->outputMixObject), 1,
                ids, req);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("create outputmix object failed");
            return result;
        }
        
        result = (*pAudioOutPut->outputMixObject)->Realize(
                pAudioOutPut->outputMixObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("realize opensles outputMix object failed");
            return result;
        }
        
        int speakers;
        if (channels > 1)
        {
            speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        }
        else
        {
            speakers = SL_SPEAKER_FRONT_CENTER;
        }
        
        SLDataFormat_PCM format_pcm =
                { SL_DATAFORMAT_PCM, channels, sampleRate,
                        SL_PCMSAMPLEFORMAT_FIXED_16,
                        SL_PCMSAMPLEFORMAT_FIXED_16, speakers,
                        SL_BYTEORDER_LITTLEENDIAN };
        
        SLDataSource audioSrc =
                { &locator_bufq, &format_pcm };
        
        SLDataLocator_OutputMix locator_outMix =
                { SL_DATALOCATOR_OUTPUTMIX, pAudioOutPut->outputMixObject };
        SLDataSink audioSink =
                { &locator_outMix, NULL };
        
        const SLInterfaceID ids1[] =
                { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
        const SLboolean req1[] =
                { SL_BOOLEAN_TRUE };
        
        result = (*pAudioOutPut->engineEngine)->CreateAudioPlayer(
                pAudioOutPut->engineEngine, &(pAudioOutPut->playerObject),
                &audioSrc, &audioSink, 1, ids1, req1);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("opensles create audio player object failed");
            return result;
        }
        
        result = (*pAudioOutPut->playerObject)->Realize(
                pAudioOutPut->playerObject, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("realize audio player object failed");
            return result;
        }
        
        result = (*pAudioOutPut->playerObject)->GetInterface(
                pAudioOutPut->playerObject, SL_IID_PLAY,
                &(pAudioOutPut->PlayerPlay));
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("opensles  audio player get play interface failed");
            return result;
        }
        
        result = (*pAudioOutPut->playerObject)->GetInterface(
                pAudioOutPut->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                &(pAudioOutPut->PlayerBufferQueue)
                );
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR(
                    "opensles audio player get buffer queue interface failed");
            return result;
        }
        
        result = (*pAudioOutPut->PlayerBufferQueue)->RegisterCallback(
                pAudioOutPut->PlayerBufferQueue, playerCallback, pAudioOutPut);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR(
                    "opensles audio player buffer queue register callback failed");
            return result;
        }
        
        result = (*pAudioOutPut->PlayerPlay)->SetPlayState(
                pAudioOutPut->PlayerPlay, SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS)
        {
            LOG_ERROR("opensles get audio player state to playing failed");
            return result;
        }
        
        return result;
    }
}

static void openSLESDestroyEngine(struct _audioOutPut * pAudioOutPut)
{
    if (pAudioOutPut->playerObject != NULL)
    {
        (*pAudioOutPut->playerObject)->Destroy(pAudioOutPut->playerObject);
        pAudioOutPut->playerObject = NULL;
        pAudioOutPut->PlayerPlay = NULL;
        pAudioOutPut->PlayerBufferQueue = NULL;
        pAudioOutPut->PlayerEffectSend = NULL;
    }
    
    if (pAudioOutPut->outputMixObject != NULL)
    {
        (*pAudioOutPut->outputMixObject)->Destroy(
                pAudioOutPut->outputMixObject);
        pAudioOutPut->outputMixObject = NULL;
    }
    
    if (pAudioOutPut->engineObject != NULL)
    {
        (*pAudioOutPut->engineObject)->Destroy(pAudioOutPut->engineObject);
        pAudioOutPut->engineObject = NULL;
        pAudioOutPut->engineEngine = NULL;
    }
}

static int openAudioDevice(struct _audioOutPut * pAudioOutPut,
        int sampleRate, int channels, int bufferframes)
{
    if (pAudioOutPut == NULL)
    {
        LOG_ERROR("AudioOutPut is NULL");
        return -1;
    }
    
    pAudioOutPut->outputChannels = channels;
    pAudioOutPut->sampleRate = sampleRate;
    
    pAudioOutPut->lock = createThreadLock();
    
    if ((pAudioOutPut->outBufferSamples = bufferframes * channels) != 0)
    {
        if ((pAudioOutPut->outputBuffer[0] = (short *) malloc(
                pAudioOutPut->outBufferSamples * sizeof(short))) == NULL
                || (pAudioOutPut->outputBuffer[1] = (short *) malloc(
                        pAudioOutPut->outBufferSamples * sizeof(short)))
                        == NULL)
        {
            closeAudioDevice(pAudioOutPut);
            return -1;
        }
    }
    
    if (createOpenSLESEngine(pAudioOutPut) != SL_RESULT_SUCCESS)
    {
        closeAudioDevice(pAudioOutPut);
        return -1;
    }
    
    if (OpenSLESOpenPlayer(pAudioOutPut) != SL_RESULT_SUCCESS)
    {
        closeAudioDevice(pAudioOutPut);
        return -1;
    }
    
    notifyThreadLock(pAudioOutPut->lock);
    pAudioOutPut->time = 0;
    return 0;
    
}

static int closeAudioDevice(struct _audioOutPut * pAudioOutPut)
{
    if (pAudioOutPut == NULL)
    {
        LOG_ERROR("AudioOutPut is NULL");
        return -1;
    }
    
    openSLESDestroyEngine(pAudioOutPut);
    
    if (pAudioOutPut->lock != NULL)
    {
        notifyThreadLock(pAudioOutPut->lock);
        destroyThreadLock(pAudioOutPut->lock);
        pAudioOutPut->lock = NULL;
    }
    
    if (pAudioOutPut->outputBuffer[0] != NULL)
    {
        free(pAudioOutPut->outputBuffer[0]);
        pAudioOutPut->outputBuffer[0] = NULL;
    }
    
    if (pAudioOutPut->outputBuffer[1] != NULL)
    {
        free(pAudioOutPut->outputBuffer[1]);
        pAudioOutPut->outputBuffer[1] = NULL;
    }
    
    return 0;
}

static int outputAudioData(struct _audioOutPut * pAudioOutPut, short *buffer,
        int size)
{
    if (pAudioOutPut == NULL)
    {
        LOG_ERROR("AudioOutPut is NULL");
        return -1;
    }
    
    short *outBuffer;
    int i, bufSamples = pAudioOutPut->outBufferSamples, index =
            pAudioOutPut->currentOutputIndex;
    
    if (bufSamples == 0)
    {
        return -1;
    }
    
    outBuffer = pAudioOutPut->outputBuffer[pAudioOutPut->currentOutputBuffer];
    for (i = 0; i < size; i++)
    {
        outBuffer[index++] = (short) (buffer[i]);
        if (index >= pAudioOutPut->outBufferSamples)
        {
            waitThreadLock(pAudioOutPut->lock);
            (*pAudioOutPut->PlayerBufferQueue)->Enqueue(
                    pAudioOutPut->PlayerBufferQueue,
                    outBuffer, bufSamples * sizeof(short));
            
            pAudioOutPut->currentOutputBuffer = (
                    pAudioOutPut->currentOutputBuffer ? 0 : 1);
            index = 0;
            outBuffer =
                    pAudioOutPut->outputBuffer[pAudioOutPut->currentOutputBuffer];
        }
    }
    
    pAudioOutPut->currentOutputIndex = index;
    pAudioOutPut->time += (double) size
            / (pAudioOutPut->sampleRate * pAudioOutPut->outputChannels);
    return i;
}

static double getTimestamp(struct _audioOutPut * pAudioOutPut)
{
    if (pAudioOutPut == NULL)
    {
        LOG_ERROR("AudioOutPut is NULL");
        return -1.0;
    }
    
    return pAudioOutPut->time;
}

AudioOutPut * createAudioOutPut()
{
    AudioOutPut * pAudioOutPut = NULL;
    pAudioOutPut = (AudioOutPut *) malloc(sizeof(AudioOutPut));
    if (NULL == pAudioOutPut)
    {
        LOG_ERROR("allocate audioOutPut failed");
        return NULL;
    }
    
    memset(pAudioOutPut, 0, sizeof(AudioOutPut));
    pAudioOutPut->openAudioDevice = openAudioDevice;
    pAudioOutPut->closeAudioDevice = closeAudioDevice;
    pAudioOutPut->outputAudioData = outputAudioData;
    pAudioOutPut->getTimestamp = getTimestamp;
    pAudioOutPut->lock->status = 1;
}

void destroyAudioOutPut(AudioOutPut ** ppAudioOutPut)
{
    if (NULL == ppAudioOutPut || NULL == *ppAudioOutPut)
    {
        return;
    }
    
    AudioOutPut * pAudioOutPut = *ppAudioOutPut;
    pAudioOutPut->closeAudioDevice(pAudioOutPut);
    pAudioOutPut->openAudioDevice = NULL;
    pAudioOutPut->closeAudioDevice = NULL;
    pAudioOutPut->outputAudioData = NULL;
    pAudioOutPut->getTimestamp = NULL;
    free(pAudioOutPut);
    pAudioOutPut = NULL;
}
