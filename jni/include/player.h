/*
 * player.h
 *
 *  Created on: 2016年10月7日
 *      Author: yangzhihui
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <android/native_window.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum Player_Errors
    {
        Player_Error_FrameQueue_Failed = -3,
        Player_Error_PacketQueue_Failed = -2,
        Player_Error_Malloc_Failed = -1,
        Player_Error_OK = 0,
        Player_Error_Uninitialized = 1,
        Player_Error_Invalied_Parameters = 2,
        Plyaer_Error_Thread_Create_Failed = 3,
        Player_Error_Extractor_Create_Failed = 4,
        Player_Error_Socket_Bind_Failed = 41,
        Player_Error_Decoder_Create_Failed = 5,
        Player_Error_Decoder_Initialize_Failed = 51,
        Player_Error_Render_Create_Failed = 6,
        Player_Error_Uncreated = 100,
        Player_Error_NONE = 0xFFFF,
    }
    ;

    enum Player_Status
    {
        Player_NONE = -2,
        Player_Destroyed = -1,
        Player_Initialized = 0,
        Player_Opened,
        Player_Playing,
        Player_Paused,
        Player_Stoped,
        Player_Closed,

    };
    int getPlayerInstance();

    int destoryPlayerInstance();

    int Player_Init();

    int Player_Destory();

    int Player_Open();

    int Player_Close();

    int Player_SetANativeWindow(ANativeWindow *window);

    int Player_Play();

    int Player_Pause();

    int Player_Stop();

    int Player_SetLocalIPAndPort(const char * ipaddr, const short port);

    int Player_SetRemoteIpAndPort(const char * ipaddr, const short port);

    int playerUpdateOrientation(float diffx, float diffy, float diffz);

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_H_ */
