/*!
 * \file decoder_ass.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#include "decoder.h"
#include "decoderCommon.h"
#include "logger.h"
#include <string.h>

static int decodeSubtitle(struct _innerDecoder *pDecoder,
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
        return -2;
    }

    AVSubtitle *subtitle;
    subtitle = (AVSubtitle *) malloc(sizeof(AVSubtitle));
    if (NULL == subtitle)
    {
        LOG_ERROR("Can't allocate subtitle");
        return -3;
    }
    memset(subtitle, 0, sizeof(AVSubtitle));
    subtitle->pts = AV_NOPTS_VALUE;

    int got_frame = 0;

    if (pkt->pts == AV_NOPTS_VALUE)
    {
        pkt->pts = pkt->dts = pDecoder->index;
    }

    result = avcodec_decode_subtitle2(pDecoder->pContext, subtitle, &got_frame,
            pkt);
    if (result < 0)
    {
        LOG_ERROR("Error decoding subtitle: %d", result);
        return NULL;
    }

    if (got_frame)
    {
        pDecoder->successCallback(got_frame, subtitle);
    }

    av_packet_free(&pkt);
    return result;
}

innerDecoder_ptr createDecoder_ass()
{
    innerDecoder_ptr assdecoder = NULL;

    assdecoder = (innerDecoder_ptr) malloc(sizeof(innerDecoder));
    memset(assdecoder, 0, sizeof(innerDecoder));
    if (assdecoder != NULL)
    {

        assdecoder->initialize = decoder_initialize;
        assdecoder->decodePacket = decodeSubtitle;
        assdecoder->registerCallback = decoder_registerCallback;
        assdecoder->uninitialize = decoder_uninitialize;
        assdecoder->codecId = AV_CODEC_ID_ASS;
    }
    return assdecoder;
}
