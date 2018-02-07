/*!
 * \file MediaPlayer.cpp
 *
 * \date Created on: Sep 15, 2017
 * \author: eyngzui
 */

#include "player.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "MediaExtractor.h"
#include "decoder.h"
#include "audioOutput.h"
#include "VideoRender2.h"
#include "SubtitleRender.h"
#include "decoderFactory.h"
#include "LinkedQueue.h"
#include "include/logger.h"

#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

typedef struct _mediaPlayer
{
    int status;

    //extractor
    MediaExtractor *mediaExtractor;

    //extractor -> decoder
    LinkedQueue *videoPktQueue;
    LinkedQueue *audioPktQueue;
    LinkedQueue *subtitlePktQueue;

    //decoder
    Decoder * decoder;

    //decoder -> render
    LinkedQueue *videoFrameQueue;
    LinkedQueue *audioFrameQueue;
    LinkedQueue *subtitleFrameQueue;

    //renders
    Render2 *videoRender;
    AudioOutPut *audioRender;
    SubtitleRender *subtitleRender;
    pthread_t render_thread;
    bool isInterruptRenderThread;
    pthread_mutex_t mutex_RT;
    pthread_cond_t cond_RT;
    int RTStatus;

    ANativeWindow *window;
} MediaPlayer, *MediaPlayer_ptr;

static MediaPlayer * gPlayer = NULL;
static pthread_mutex_t mutex_player = PTHREAD_MUTEX_INITIALIZER;

int getPlayerInstance()
{
    if (0 != pthread_mutex_trylock(&mutex_player))
    {
        return 1;
    }

    int error = Player_Error_NONE;
    MediaPlayer *iPlayer = NULL;
    do
    {
        if (NULL != gPlayer)
        {
            if (Player_Initialized == gPlayer->status)
            {
                error = Player_Error_OK;
                pthread_mutex_unlock(&mutex_player);
                return error;
            }
            break;
        }

        iPlayer = (MediaPlayer *) malloc(sizeof(MediaPlayer));
        if (NULL == iPlayer)
        {
            error = Player_Error_Malloc_Failed;
            LOG_ERROR("allocate memory for IPlayer failed, error : %d",
                    error);
            pthread_mutex_unlock(&mutex_player);
            return error;
        }
        memset(iPlayer, 0, sizeof(MediaPlayer));
        iPlayer->status = Player_NONE;
        gPlayer = iPlayer;
    } while (0);
    pthread_mutex_unlock(&mutex_player);
    return 0;
}

int destoryPlayerInstance()
{
    if (0 != pthread_mutex_trylock(&mutex_player))
    {
        return 1;
    }

    if (NULL == gPlayer)
    {
        pthread_mutex_unlock(&mutex_player);
        return 0;
    }

    if (gPlayer->status > Player_Destroyed)
    {
        LOG_WARNING("player haven't been destroied");
        pthread_mutex_unlock(&mutex_player);
        return -1;
    }

    free(gPlayer);
    gPlayer = NULL;
    pthread_mutex_unlock(&mutex_player);
    pthread_mutex_destroy(&mutex_player);
    return 0;
}

int playerUpdateOrientation(float diffx, float diffy, float diffz)
{
    return 0;
}

int Player_Init()
{
    int error = Player_Error_NONE;

    return error;
}

int Player_Destory()
{
    return 0;
}

int Player_Open()
{
    return 0;
}

int Player_Close()
{
    return 0;
}

int Player_SetANativeWindow(ANativeWindow *window)
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }
    gPlayer->window = window;

    return Player_Error_OK;
}

int Player_Play()
{
    return 0;
}

int Player_Pause()
{
    return 0;
}

int Player_Stop()
{
    return 0;
}

int Player_SetLocalIPAndPort(const char * ipaddr, const short port)
{
    return 0;
}

int Player_SetRemoteIpAndPort(const char * ipaddr, const short port)
{
    return 0;
}
