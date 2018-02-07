/*
 * VideoDecoder_H264.cpp
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */
#include "decoder.h"
#include "logger.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "decoderCommon.h"

innerDecoder_ptr createDecoder_h264()
{
    innerDecoder_ptr h264decoder = NULL;

    h264decoder = (innerDecoder_ptr) malloc(sizeof(innerDecoder));
    memset(h264decoder, 0, sizeof(innerDecoder));
    if (h264decoder != NULL)
    {

        h264decoder->initialize = decoder_initialize;
        h264decoder->decodePacket = decoder_decodePacket;
        h264decoder->registerCallback = decoder_registerCallback;
        h264decoder->uninitialize = decoder_uninitialize;
        h264decoder->decode = avcodec_decode_video2;
        h264decoder->codecId = AV_CODEC_ID_H264;
    }
    return h264decoder;
}
