/*
 * VideoEncoderFactory.h
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */

#ifndef VIDEOENCODERFACTORY_H_
#define VIDEOENCODERFACTORY_H_

#include "include/VideoEncoder.h"

Video_Encoder * getVideoEncoderInstance(int encoderType);

void destroyVideoEncoderInstance(Video_Encoder_ptr * ppEncoder);

#endif /* VIDEOENCODERFACTORY_H_ */
