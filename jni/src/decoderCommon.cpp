/*!
 * \file decoderCommon.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#include "decoderCommon.h"
#include "logger.h"

int decoder_initialize(struct _innerDecoder *pDecoder, void *param)
{
    if (pDecoder == NULL || param == NULL)
    {
        LOG_ERROR("parameter invalid, pDecoder = %p, param = %p", pDecoder,
                param);
        return -1;
    }

    int result;
    AVCodecParameters *codecParam = (AVCodecParameters *) param;
    AVCodec *decoder = NULL;
    decoder = avcodec_find_decoder((enum AVCodecID) pDecoder->codecId);
    if (NULL == decoder)
    {
        LOG_ERROR("Codec not found\n");
        return -1;
    }
    pDecoder->pCodec = decoder;

    AVCodecContext *context = NULL;
    /* create codec context*/
    context = avcodec_alloc_context3(decoder);
    if (NULL == context)
    {
        LOG_ERROR("Could not allocate video codec context");
        free(decoder);
        return -1;
    }
    pDecoder->pContext = context;

    result = avcodec_parameters_to_context(pDecoder->pContext, codecParam);
    if (result)
    {
        LOG_ERROR("Can't copy decoder parameters to context, error code: %d",
                result);
        return result;
    }

    result = avcodec_open2(pDecoder->pContext,
            pDecoder->pCodec, NULL);
    if (result < 0)
    {
        {
            LOG_ERROR("Could not open video codec, err = %d", result);
            return result;
        }
    }
}

int decoder_registerCallback(struct _innerDecoder *pDecoder,
        void (*fun)(int state, void * frame))
{
    if (pDecoder == NULL || fun == NULL)
    {
        LOG_ERROR("invalid paramters, pDecoder = %p, fun = %p", pDecoder, fun);
    }

    pDecoder->successCallback = fun;
}

int decoder_decodePacket(struct _innerDecoder *pDecoder,
        AVPacket * pkt)
{
    int result = 0;
    if (NULL == pDecoder)
    {
        return -1;
    }
    AVCodecContext * context = (AVCodecContext *) pDecoder->pContext;
    if (NULL == context)
    {
        return -1;
    }

    AVFrame *frame;
    frame = av_frame_alloc();
    if (NULL == frame)
    {
        LOG_ERROR("Can't allocate frame");
        return -1;
    }
    int got_frame = 0;

    if (pkt->pts == AV_NOPTS_VALUE)
    {
        pkt->pts = pkt->dts = pDecoder->index;
    }

    result = pDecoder->decode(pDecoder->pContext, frame, &got_frame, pkt);
    if (result < 0)
    {
        LOG_ERROR("Error decoding frame: %d", result);
        return result;
    }

    if (got_frame && pDecoder->successCallback)
    {
        pDecoder->successCallback(got_frame, frame);
    }

    av_packet_free(&pkt);
    return result;
}

void decoder_uninitialize(struct _innerDecoder *pDecoder)
{
    if (pDecoder == NULL)
    {
        LOG_ERROR("invalid parameter, pDecoder = %p", pDecoder);
        return;
    }
    avcodec_close(pDecoder->pContext);
    avcodec_free_context(&pDecoder->pContext);
}
