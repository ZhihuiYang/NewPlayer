/*
 * VideoDecoderFactory.cpp
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#include <string.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif
#include "include/decoderFactory.h"

innerDecoder_ptr getDecoderInstance(CODEC_TYPE decoderType)
{
    innerDecoder_ptr retDecoder = NULL;
    switch (decoderType)
    {
        case CODEC_TYPE_DECODER_H265:
            break;
        case CODEC_TYPE_DECODER_H264:
            retDecoder = createDecoder_h264();
            break;
        case CODEC_TYPE_DECODER_AAC:
            retDecoder = createDecoder_aac();
            break;
    }
    return retDecoder;
}

void destroyDecoderInstance(innerDecoder_ptr * ppDecoder)
{
    destroyDecoderInner(ppDecoder);
}
