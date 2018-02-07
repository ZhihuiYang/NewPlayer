/*
 * VideoDecoderFactory.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEODECODERFACTORY_H_
#define VIDEODECODERFACTORY_H_

#include "decoder.h"

#ifdef __cplusplus
extern "C"
{
#endif

    innerDecoder_ptr getDecoderInstance(CODEC_TYPE decoderType);

    void destroyDecoderInstance(innerDecoder_ptr * ppDecoder);

#ifdef __cplusplus
}
#endif
#endif /* VIDEODECODERFACTORY_H_ */
