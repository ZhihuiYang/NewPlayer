/*
 * VideoDecoder.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEODECODER_H_
#define VIDEODECODER_H_

#include <stdint.h>
#include "AVFrame.h"
#include "codectypedef.h"
#include "util.h"
#include "LinkedQueue.h"

extern "C"
{
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavutil/frame.h"
}

#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

#ifdef __cpluscplus
extern "C"
{
#endif

typedef void (*callbackFun)(int state, void * frame);
typedef int (*decodeFun)(AVCodecContext *avctx, AVFrame *frame,
        int *got_frame_ptr, const AVPacket *avpkt);

typedef struct _innerDecoder
{
    int codecId;
    AVCodec * pCodec;
    AVCodecContext * pContext;
    callbackFun successCallback;
    decodeFun decode;
    int index;

    int (*initialize)(struct _innerDecoder *, void * parameter);

    int (*decodePacket)(struct _innerDecoder *pDecoder,
            AVPacket *packet);
    int (*registerCallback)(struct _innerDecoder *pDecoder,
            void (*fun)(int state, void * frame));

    void (*uninitialize)(struct _innerDecoder *);

} innerDecoder, *innerDecoder_ptr;

innerDecoder_ptr createDecoder_h264();

innerDecoder_ptr createDecoder_aac();

innerDecoder_ptr createDecoder_ass();

static inline void destroyDecoderInner(innerDecoder_ptr * ppdecoder)
{
    innerDecoder_ptr pdecoder = *ppdecoder;
    if (pdecoder != NULL)
    {
        pdecoder->uninitialize(pdecoder);
        free(pdecoder);
    }
    pdecoder = NULL;
}

enum Decoder_Status
{
    Decoder_None = -1,
    Decoder_Initialized,
    Decoder_SetDataSource,
    Decoder_Extracting,
    Decoder_Paused,
    Decoder_Stoped,
};

typedef struct _decoder
{
    LinkedQueue *videoPktQueue;
    LinkedQueue *audioPktQueue;
    LinkedQueue *subtitlePktQueue;
    innerDecoder *videoDecoder;
    innerDecoder *audioDecoder;
    innerDecoder *subtitleDecoder;
    bool isInterruptDecoderThread;
    ThreadContext * threadCtx;
    int status;

    int (*initialize)(struct _decoder *, void * vParameter, void * aParameter,
            void * sParameter, LinkedQueue *vPktQueue,
            LinkedQueue *aPktQueue, LinkedQueue *sPktQueue);
    int (*registerCallback)(struct _decoder *pDecoder,
            void (*vfun)(int state, void * frame),
            void (*afun)(int state, void * frame),
            void (*sfun)(int state, void * frame));
    int (*startDecode)(struct _decoder *pDecoder);
    int (*pauseDecode)(struct _decoder *pDecoder);
    int (*resumeDecode)(struct _decoder *pDecoder);
    int (*stopDecode)(struct _decoder *pDecoder);
    void (*uninitialize)(struct _decoder *);
} Decoder, *Decoder_ptr;

Decoder *createDecoder();

void destroyDecoder(Decoder ** ppDecoder);

#ifdef __cpluscplus
}
#endif
#endif /* VIDEODECODERBASE_H_ */
