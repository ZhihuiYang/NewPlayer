/*
 * VideoEncoderBase.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEOENCODER_H_
#define VIDEOENCODER_H_
#include <stdint.h>
#include "codectypedef.h"
#include "AVFrame.h"
#include <include/openh264/codec_api.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

#ifdef __cpluscplus
extern "C"
{   
#endif

#define MAX_LAYER_NUM_OF_FRAME 128

typedef enum
{
    encVideoFrameTypeInvalid,        ///< encoder not ready or parameters are invalidate
    encVideoFrameTypeIDR,        ///< IDR frame in H.264
    encvideoFrameTypeI,          ///< I frame type
    encVideoFrameTypeP,          ///< P frame type
    encVideoFrameTypeSkip,        ///< skip the frame based encoder kernel
    encVideoFrameTypeIPMixed        ///< a frame where I and P slices are mixing, not supported yet
} EncVideoFrameType;

/**
 * @brief Bitstream inforamtion of a layer being encoded
 */
typedef struct _Layer_BitStream_Info
{
    unsigned char uiTemporalId;
    unsigned char uiSpatialId;
    unsigned char uiQualityId;
    EncVideoFrameType eFrameType;
    unsigned char uiLayerType;

    /**
     * The sub sequence layers are ordered hierarchically based on their dependency on each other so that any picture in a layer shall not be
     * predicted from any picture on any higher layer.
     */
    int iSubSeqId;        ///< refer to D.2.11 Sub-sequence information SEI message semantics
    int iNalCount;              ///< count number of NAL coded already
    int* pNalLengthInByte;        ///< length of NAL size in byte from 0 to iNalCount-1
    unsigned char* pBsBuf;        ///< buffer of bitstream contained
} Layer_BS_Info, *Layer_BS_Info_ptr;

typedef struct _Encoder_Output
{
    int iLayerNum;
    Layer_BS_Info sLayerInfo[MAX_LAYER_NUM_OF_FRAME];
    EncVideoFrameType eFrameType;
    int iFrameSizeInBytes;
    long long uiTimeStamp;
} Encoder_Output, *Encoder_Output_ptr;

typedef struct _Video_Encoder
{
    void * pEncoderInstance;

    bool (*initialize)(struct _Video_Encoder *, const void * parameter);

    bool (*encodeframe)(struct _Video_Encoder *, const Lisp_AVFrame * input,
            struct _Encoder_Output ** output);

    void (*destoryvideoencoder)(struct _Video_Encoder *);
} Video_Encoder, *Video_Encoder_ptr;

Video_Encoder_ptr createVideoEncoderH264();

static inline void destroyVideoEncoder(Video_Encoder_ptr * ppendoder)
{
    Video_Encoder_ptr pendoder = *ppendoder;
    if (pendoder != NULL)
    {
        pendoder->destoryvideoencoder(pendoder);
        free(pendoder);
    }
    ppendoder = NULL;
}

#ifdef __cpluscplus
}
#endif
#endif /* VIDEOENCODER_H_ */
