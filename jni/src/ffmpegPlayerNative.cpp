#include <jni.h>
#include "ffmpegPlayerdemo.h"
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <string.h>
#include "logger.h"
#include "include/VideoRender2.h"

static ANativeWindow* theNativeWindow = NULL;

#ifndef _Included_com_ericsson_lispmediaplayer_MainActivity
#define _Included_com_ericsson_lispmediaplayer_MainActivity
#ifdef __cplusplus
extern "C"
{
#endif
    
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_MainActivity_setAssetManager(
            JNIEnv *env, jobject object, jobject assetManager)
    {
        AAssetManager * AssetMgr = NULL;
        AssetMgr = AAssetManager_fromJava(env, assetManager);
        setAssetManager(AssetMgr);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_MainActivity
     * Method:    setSurfaceView
     * Signature: (Landroid/view/Surface;)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_MainActivity_setSurfaceView
    (JNIEnv *env, jobject object, jobject surface)
    {
        
        theNativeWindow = ANativeWindow_fromSurface(env, surface);
        
        return setSurfaceView(theNativeWindow);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_MainActivity
     * Method:    startPlay
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_MainActivity_startPlay
    (JNIEnv *env, jobject object, jstring url, jboolean isVRPlaying)
    {
        const char *cStr = NULL;
        char cUrl[1024];
        jboolean isCopy;
        memset(cUrl, 0, 1024);
        cStr = env->GetStringUTFChars(url, &isCopy);
        memcpy(cUrl, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(url, cStr);

        return startPlay(cUrl, isVRPlaying);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_MainActivity
     * Method:    stopPlay
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_MainActivity_stopPlay
    (JNIEnv * env, jobject object)
    {
        
        int ret = stopPlay();
        if (theNativeWindow != NULL)
        {
            ANativeWindow_release(theNativeWindow);
            theNativeWindow = NULL;
        }
        return ret;
    }

#ifdef __cplusplus
}
#endif
#endif
