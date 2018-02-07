/*!
 * \file decoderCommon.h
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#ifndef INCLUDE_DECODERCOMMON_H_
#define INCLUDE_DECODERCOMMON_H_

#include "decoder.h"

int decoder_initialize(struct _innerDecoder *pDecoder, void *param);

int decoder_registerCallback(struct _innerDecoder *pDecoder,
        void (*fun)(int state, void * frame));

int decoder_decodePacket(struct _innerDecoder *pDecoder,
        AVPacket * pkt);

void decoder_uninitialize(struct _innerDecoder *pDecoder);

#endif /* INCLUDE_DECODERCOMMON_H_ */
