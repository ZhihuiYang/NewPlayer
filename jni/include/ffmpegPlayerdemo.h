/*!
 * \file ffmpegPlayerdemo.h
 *
 * \date Created on: Feb 27, 2017
 * \author: eyngzui
 */

#ifndef INCLUDE_FFMPEGPLAYERDEMO_H_
#define INCLUDE_FFMPEGPLAYERDEMO_H_

#include <android/native_window_jni.h>

int setSurfaceView(ANativeWindow *window);

int startPlay(char * url, bool isVRPlaying);

int stopPlay();

#endif /* INCLUDE_FFMPEGPLAYERDEMO_H_ */
