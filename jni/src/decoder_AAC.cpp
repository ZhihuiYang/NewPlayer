/*!
 * \file decoder_AAC.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#include "decoder.h"
#include "logger.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "decoderCommon.h"

innerDecoder_ptr createDecoder_aac()
{
    innerDecoder_ptr aacdecoder = NULL;

    aacdecoder = (innerDecoder_ptr) malloc(sizeof(innerDecoder));
    memset(aacdecoder, 0, sizeof(innerDecoder));
    if (aacdecoder != NULL)
    {
        aacdecoder->decode = avcodec_decode_audio4;
        aacdecoder->initialize = decoder_initialize;
        aacdecoder->decodePacket = decoder_decodePacket;
        aacdecoder->registerCallback = decoder_registerCallback;
        aacdecoder->uninitialize = decoder_uninitialize;
        aacdecoder->codecId = AV_CODEC_ID_AAC;
    }
    return aacdecoder;
}
