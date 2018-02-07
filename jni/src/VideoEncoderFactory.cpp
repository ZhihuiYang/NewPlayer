/*
 * VideoEncoderFactory.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#include "include/VideoEncoder.h"
#include "include/VideoEncoderFactory.h"

Video_Encoder * getVideoEncoderInstance(int encoderType)
{
    Video_Encoder * encoderInstance = NULL;
    
    switch (encoderType)
    {
        case CODEC_TYPE_ENCODER_NONE:
            break;
        case CODEC_TYPE_ENCODER_H264:
            encoderInstance = createVideoEncoderH264();
            break;
        case CODEC_TYPE_ENCODER_H265:
            break;
        default:
            break;
    }
    return encoderInstance;
}

void destroyVideoEncoderInstance(Video_Encoder_ptr * ppEncoder)
{
    destroyVideoEncoder(ppEncoder);
}
