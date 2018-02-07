/*
 * recorder.h
 *
 *  Created on: 2016年10月7日
 *      Author: yangzhihui
 */

#ifndef RECORDER_H_
#define RECORDER_H_
#include <jni.h>
#ifdef __cplusplus
extern "C"
{
#endif
    
    enum Recorder_Errors
    {
        Recorder_Error_FrameQueue_Failed = -3,
        Recorder_Error_Malloc_Failed = -1,
        Recorder_Error_OK = 0,
        Recorder_Error_Uninitialized = 1,
        Recorder_Error_Invalied_Parameters = 2,
        Recorder_Error_Thread_Create_Failed = 3,
        Recorder_Error_Socket_Create_Failed = 4,
        Recorder_Error_Socket_Bind_Failed = 41,
        Recorder_Error_Encoder_Created_Failed = 5,
        Recorder_Error_Encoder_Initialize_Failed = 51,
        Recoder_Error_Uncreated = 100,
        Recorder_Error_NONE = 0xFFFF,
    };
    
    enum Recorder_Status
    {
        Recorder_NONE = -2,
        Recorder_Destroyed = -1,
        Recorder_Initialized = 0,
        Recorder_Started,
        Recorder_Recording,
        Recorder_Paused,
        Recorder_Stoped,
        Recorder_Closed,
    
    };
    
    int getRecorderInstance();
    
    int destroyRecorderInstance();
    
    int Recorder_Init();
    
    int Recorder_Destory();
    
    int Recorder_Start();
    
    int Recorder_Stop();
    
    int Recorder_SetLocalIPAndPort(const char * ipaddr, const short port);
    
    int Recorder_SetRemoteIpAndPort(const char * ipaddr, const short port);
    
    void nativeSendYuvData(JNIEnv *env, jobject thiz, jbyteArray yuvData,
            jint width, jint height);
#ifdef __cplusplus
}
#endif

#endif /* RECORDER_H_ */
