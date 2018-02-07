/*!
 * \file decoder.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */
#include "decoder.h"
#include "decoderCommon.h"
#include "logger.h"

#define RUNING 1
#define PAUSED 0

static int initialize(struct _decoder * decoder, void * vParameter,
        void * aParameter, void * sParameter, LinkedQueue *vPktQueue,
        LinkedQueue *aPktQueue, LinkedQueue *sPktQueue)
{

    int result = 0;
    if (NULL == decoder)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }

    if (decoder->status == Decoder_Initialized)
    {
        LOG_INFO("decoder initialized already");
        return 0;
    }

    if (NULL == vParameter || NULL == aParameter || NULL == sParameter)
    {
        LOG_ERROR(
                "parameters invalid, vParameter = %p, aParameter = %p, sParameter = %p",
                vParameter, aParameter, sParameter);
        return -1;
    }

    decoder->videoPktQueue = vPktQueue;
    decoder->audioPktQueue = aPktQueue;
    decoder->subtitlePktQueue = sPktQueue;

    result = decoder->videoDecoder->initialize(decoder->videoDecoder,
            vParameter);
    if (result < 0)
    {
        return result;
    }
    result = decoder->audioDecoder->initialize(decoder->audioDecoder,
            aParameter);
    if (result < 0)
    {
        decoder->videoDecoder->uninitialize(decoder->videoDecoder);
        return result;
    }
    result = decoder->subtitleDecoder->initialize(decoder->subtitleDecoder,
            sParameter);
    if (result < 0)
    {
        decoder->videoDecoder->uninitialize(decoder->videoDecoder);
        decoder->audioDecoder->uninitialize(decoder->audioDecoder);
        return result;
    }

    decoder->status = Decoder_Initialized;
    return result;
}

static int registerCallback(struct _decoder *pDecoder,
        void (*vfun)(int state, void * frame),
        void (*afun)(int state, void * frame),
        void (*sfun)(int state, void * frame))
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }
    if (pDecoder->videoDecoder)
    {
        pDecoder->videoDecoder->registerCallback(pDecoder->videoDecoder, vfun);
    }

    if (pDecoder->audioDecoder)
    {
        pDecoder->audioDecoder->registerCallback(pDecoder->audioDecoder, afun);
    }

    if (pDecoder->subtitleDecoder)
    {
        pDecoder->subtitleDecoder->registerCallback(pDecoder->subtitleDecoder,
                sfun);
    }

    return 0;
}

static void * decoderThreadHandler(void * arguments)
{
    if (arguments == NULL)
    {
        return NULL;
    }

    Decoder * decoder = (Decoder *) arguments;
    int result;

    while (!decoder->isInterruptDecoderThread)
    {
        waitThread(decoder->threadCtx);

        AVPacket * videoPkt = NULL, *audioPkt = NULL, *subtitlePkt = NULL;
        decoder->videoPktQueue->deQueue(decoder->videoPktQueue, &videoPkt);
        decoder->audioPktQueue->deQueue(decoder->audioPktQueue, &audioPkt);
        decoder->subtitlePktQueue->deQueue(decoder->subtitlePktQueue,
                &subtitlePkt);
        if (videoPkt)
        {
            decoder->videoDecoder->decodePacket(decoder->videoDecoder,
                    videoPkt);
            av_packet_free(&videoPkt);
        }

        if (audioPkt)
        {
            decoder->audioDecoder->decodePacket(decoder->audioDecoder,
                    audioPkt);
            av_packet_free(&audioPkt);
        }

        if (subtitlePkt)
        {
            decoder->subtitleDecoder->decodePacket(decoder->subtitleDecoder,
                    subtitlePkt);
            av_packet_free(&subtitlePkt);
        }

    }
}

static int startDecode(struct _decoder *pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }

    pthread_t threadId = -1;
    if (pDecoder->threadCtx->tid == -1)
    {
        if (0
                != pthread_create(&threadId, NULL, decoderThreadHandler,
                        pDecoder))
        {
            LOG_ERROR("create decoder thread failed");
            return -1;
        }
    }

    pDecoder->threadCtx->tid = threadId;
    resumeThread(pDecoder->threadCtx);
    return 0;
}

static int pauseDecode(struct _decoder *pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }
    pauseThread(pDecoder);
    return 0;
}

static int resumeDecode(struct _decoder *pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }
    resumeThread(pDecoder);
    return 0;
}

static int stopDecode(struct _decoder *pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return -1;
    }

    pDecoder->isInterruptDecoderThread = true;
    resumeThread(pDecoder->threadCtx);
    return 0;
}

static void uninitialize(struct _decoder * pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("decoder is NULL");
        return;
    }
    pDecoder->videoDecoder->uninitialize(pDecoder->videoDecoder);
    pDecoder->audioDecoder->uninitialize(pDecoder->audioDecoder);
    pDecoder->subtitleDecoder->uninitialize(pDecoder->subtitleDecoder);
}

Decoder *createDecoder()
{
    Decoder * decoder = NULL;
    decoder = (Decoder *) malloc(sizeof(Decoder));
    if (NULL != decoder)
    {
        memset(decoder, 0, sizeof(Decoder));
        decoder->initialize = initialize;
        decoder->registerCallback = registerCallback;
        decoder->startDecode = startDecode;
        decoder->pauseDecode = pauseDecode;
        decoder->resumeDecode = resumeDecode;
        decoder->stopDecode = stopDecode;
        decoder->uninitialize = uninitialize;
        decoder->videoDecoder = createDecoder_h264();
        decoder->audioDecoder = createDecoder_aac();
        decoder->subtitleDecoder = createDecoder_ass();
        decoder->isInterruptDecoderThread = false;
        ThreadContext * tCtx = NULL;
        tCtx = (ThreadContext *) malloc(sizeof(ThreadContext));
        memset(tCtx, 0, sizeof(ThreadContext));
        tCtx->tid = -1;
        pthread_mutex_init(&tCtx->mutex, NULL);
        pthread_cond_init(&tCtx->cond, NULL);
        tCtx->status = PAUSED;
        decoder->threadCtx = tCtx;
        decoder->status = Decoder_Initialized;
    }

    return decoder;
}

void destroyDecoder(Decoder ** ppDecoder)
{
    if (ppDecoder == NULL)
    {
        return;
    }
    Decoder *pDecoder = *ppDecoder;

    if (pDecoder == NUL)
    {
        return;
    }

    free(pDecoder->threadCtx);
    free(pDecoder);
    pDecoder = NULL;
}
