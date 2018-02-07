/*
 * codectypedef.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef CODECTYPEDEF_H_
#define CODECTYPEDEF_H_
#include <stddef.h>

typedef enum _codec_type
{
    CODEC_TYPE_ENCODER_NONE = -1,
    CODEC_TYPE_ENCODER_H264 = 0,
    CODEC_TYPE_ENCODER_H265,
    CODEC_TYPE_ENCODER_AAC,

    CODEC_TYPE_DECODER_H264 = 1000,
    CODEC_TYPE_DECODER_H265,
    CODEC_TYPE_DECODER_AAC

} CODEC_TYPE;

#endif /* CODECTYPEDEF_H_ */
