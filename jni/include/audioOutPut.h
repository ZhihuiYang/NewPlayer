/*!
 * \file audioOutput.h
 *
 * \date Created on: Jul 7, 2017
 * \author: eyngzui
 */

#ifndef INCLUDE_AUDIOOUTPUT_H_
#define INCLUDE_AUDIOOUTPUT_H_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>

typedef struct _threadLock
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned char status;
} ThreadLock;

typedef struct _audioOutPut
{
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;

    SLObjectItf playerObject;
    SLPlayItf PlayerPlay;
    SLAndroidSimpleBufferQueueItf PlayerBufferQueue;
    SLEffectSendItf PlayerEffectSend;

    int currentOutputIndex;
    int currentOutputBuffer;
    short *outputBuffer[2];
    int outBufferSamples;

    ThreadLock *lock;
    double time;
    int outputChannels;
    int sampleRate;

    int (*openAudioDevice)(struct _audioOutPut * pAudioOutPut,
            int sampleRate, int channels, int bufferframes);
    int (*closeAudioDevice)(struct _audioOutPut * pAudioOutPut);
    int (*outputAudioData)(struct _audioOutPut * pAudioOutPut, short *buffer,
            int size);
    double (*getTimestamp)(struct _audioOutPut * pAudioOutPut);
    
} AudioOutPut, *pAudioOutPut;

AudioOutPut * createAudioOutPut();

void destroyAudioOutPut(AudioOutPut **);

#endif /* INCLUDE_AUDIOOUTPUT_H_ */
