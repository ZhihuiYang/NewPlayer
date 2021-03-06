/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_ericsson_lispmediaplayer_Player */
#include <include/player.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <string.h>
#include "include/VideoRender2.h"
static ANativeWindow* theNativeWindow;

#ifndef _Included_com_ericsson_lispmediaplayer_Player
#define _Included_com_ericsson_lispmediaplayer_Player
#ifdef __cplusplus
extern "C"
{
#endif
    
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerUpdateOrientation(
            JNIEnv *env, jobject object, jfloat diffx, jfloat diffy,
            jfloat diffz)
    {
        return playerUpdateOrientation(diffx, diffy, diffz);
    }
    
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_setAssetManager(
            JNIEnv *env, jobject object, jobject assetManager)
    {
        AAssetManager * AssetMgr = NULL;
        AssetMgr = AAssetManager_fromJava(env, assetManager);
        setAssetManager(AssetMgr);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerGetInstance
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerGetInstance
    (JNIEnv *env, jobject object)
    {
        return getPlayerInstance();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerReleaseInstance
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerReleaseInstance
    (JNIEnv *env, jobject object)
    {
        return destoryPlayerInstance();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerInit
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerInit
    (JNIEnv *env, jobject object)
    {
        return Player_Init();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerOpen
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerOpen
    (JNIEnv *env, jobject object)
    {
        return Player_Open();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerPlay
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerPlay
    (JNIEnv *env, jobject object)
    {
        return Player_Play();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerClose
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerClose
    (JNIEnv *env, jobject object)
    {
        return Player_Close();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerStop
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerStop
    (JNIEnv *env, jobject object)
    {
        return Player_Stop();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerPause
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerPause
    (JNIEnv *env, jobject object)
    {
        return Player_Pause();
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerDestroy
     * Signature: ()I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerDestroy
    (JNIEnv *env, jobject object)
    {
        
        int ret = Player_Destory();
        if (theNativeWindow != NULL)
        {
            ANativeWindow_release(theNativeWindow);
            theNativeWindow = NULL;
        }
        return ret;
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerSetSurfaceView
     * Signature: (Landroid/view/SurfaceView;)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerSetSurfaceView
    (JNIEnv *env, jobject object, jobject surface)
    {
        theNativeWindow = ANativeWindow_fromSurface(env, surface);
        return Player_SetANativeWindow(theNativeWindow);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerSetLocalIpAndPort
     * Signature: (Ljava/lang/String;S)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerSetLocalIpAndPort
    (JNIEnv *env, jobject object, jstring ip, jshort port)
    {
        const char *cStr = NULL;
        char ipaddr[64] =
                { 0 };
        jboolean isCopy;
        cStr = env->GetStringUTFChars(ip, &isCopy);
        memcpy(ipaddr, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(ip, cStr);
        return Player_SetLocalIPAndPort(ipaddr, (const short) port);
    }
    
    /*
     * Class:     com_ericsson_lispmediaplayer_Player
     * Method:    playerSetRemoteIpAndPort
     * Signature: (Ljava/lang/String;S)I
     */
    JNIEXPORT jint JNICALL Java_com_ericsson_lispmediaplayer_Player_playerSetRemoteIpAndPort
    (JNIEnv *env, jobject object, jstring ip, jshort port)
    {
        const char *cStr = NULL;
        char ipaddr[64] =
                { 0 };
        jboolean isCopy;
        cStr = env->GetStringUTFChars(ip, &isCopy);
        memcpy(ipaddr, cStr, strlen(cStr));
        env->ReleaseStringUTFChars(ip, cStr);
        return Player_SetRemoteIpAndPort(ipaddr, (const short) port);
    }

#ifdef __cplusplus
}
#endif
#endif
