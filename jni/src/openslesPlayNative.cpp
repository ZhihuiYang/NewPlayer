/*!
 * \file openslesPlayNative.cpp
 *
 * \date Created on: Mar 8, 2017
 * \author: eyngzui
 */

#include <jni.h>
#include "openslesPlaydemo.h"

#ifndef _Included_com_ericsson_lispmediaplayer_MainActivityOpenSLES
#define _Included_com_ericsson_lispmediaplayer_MainActivityOpenSLES
#ifdef __cplusplus
extern "C"
{
#endif
    
    /*
     * Class:     com_ericsson_lispmediaplayer_MainActivityOpenSLES
     * Method:    createEngine
     * Signature: ()V
     */
    JNIEXPORT void JNICALL Java_com_ericsson_lispmediaplayer_MainActivityOpenSLES_createEngine
    (JNIEnv *env, jobject object)
    {   
        createEngine();
    }

    /*
     * Class:     com_ericsson_lispmediaplayer_MainActivityOpenSLES
     * Method:    createAudioPlayer
     * Signature: (Ljava/lang/String;)Z
     */
    JNIEXPORT jboolean JNICALL Java_com_ericsson_lispmediaplayer_MainActivityOpenSLES_createAudioPlayer
    (JNIEnv *env, jobject object, jstring url)
    {
        const char *cStr = NULL;
        char cUrl[1024];
        jboolean isCopy;
        memset(cUrl, 0, 1024);
        cStr = env->GetStringUTFChars(url, &isCopy);
        memcpy(cUrl, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(url, cStr);
        return createAudioPlayer(cUrl);
    }

/*
 * Class:     com_ericsson_lispmediaplayer_MainActivityOpenSLES
 * Method:    shutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_ericsson_lispmediaplayer_MainActivityOpenSLES_shutdown
(JNIEnv *env, jobject object)
{   
    shutdown();
}

#ifdef __cplusplus
}
#endif
#endif

