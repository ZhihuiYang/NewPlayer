/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_ericsson_lispmediaplayer_Recorder */
#include "include/recorder.h"
#include <string.h>

#ifndef _Included_com_ericsson_lispmediaplayer_Recorder
#define _Included_com_ericsson_lispmediaplayer_Recorder
#ifdef __cplusplus
extern "C"
{
#endif
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderGetInstance
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderGetInstance
    (JNIEnv *env, jobject object)
    {
        return getRecorderInstance();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderReleaseInstance
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderReleaseInstance
    (JNIEnv *env, jobject object)
    {
        return destroyRecorderInstance();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderInit
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderInit
    (JNIEnv *env, jobject object)
    {
        return Recorder_Init();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderDestroy
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderDestroy
    (JNIEnv *env, jobject object)
    {
        return Recorder_Destory();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderSetLocalIpAndPort
     * Signature: (Ljava/lang/String;S)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderSetLocalIpAndPort
    (JNIEnv *env, jobject object, jstring ip, jshort port)
    {
        const char *cStr = 0;
        char ipaddr[64] =
                { 0 };
        jboolean isCopy;
        cStr = env->GetStringUTFChars(ip, &isCopy);
        memcpy(ipaddr, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(ip, cStr);
        return Recorder_SetLocalIPAndPort(ipaddr, (const short) port);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderSetRemoteIpAndPort
     * Signature: (Ljava/lang/String;S)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderSetRemoteIpAndPort
    (JNIEnv *env, jobject object, jstring ip, jshort port)
    {
        const char *cStr = 0;
        char ipaddr[64] =
                { 0 };
        jboolean isCopy;
        cStr = env->GetStringUTFChars(ip, &isCopy);
        memcpy(ipaddr, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(ip, cStr);
        return Recorder_SetRemoteIpAndPort(ipaddr, (const short) port);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderStart
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderStart
    (JNIEnv *env, jobject object)
    {
        return Recorder_Start();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Recorder
     * Method:    recorderStop
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Recorder_recorderStop
    (JNIEnv *env, jobject object)
    {
        return Recorder_Stop();
    }
/*
 *
 *
 *
 */
JNIEXPORT void JNICALL Java_com_ericsson_lispmediaplayer_Recorder_nativeSendYuvData(JNIEnv *env,
        jobject thiz, jbyteArray yuvData,
        jint width, jint height)
{   
    return nativeSendYuvData(env, thiz, yuvData,
            width, height);
}

#ifdef __cplusplus
}
#endif
#endif