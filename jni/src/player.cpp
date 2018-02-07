/*
 * player.cpp
 *
 *  Created on: 2016年10月7日
 *      Author: yangzhihui
 */

//#include "include/player.h"
#include <pthread.h>
#include <sys/time.h>
#include "include/h264_RtpPacket.h"
#include "include/logger.h"
#include "include/memoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include "include/decoder.h"
#include "include/jitterBuffer.h"
#include "include/LinkedQueue.h"
#include "include/VideoRender.h"
#include "include/VideoRender2.h"
#include "include/decoderFactory.h"
#include "include/AVPacket.h"

#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

#ifdef DUMP_DATA
bool PlayerDumpData = true;
#else
bool PlayerDumpData = false;
#endif

#ifndef RENDER2
#define RENDER2
#endif
#define DEFAULT_JB_LENGTH 10
#define RUNING 1
#define PAUSED 0
#define MTU 1500
#define FPS 30
enum
{
    MAX_ENCODED_IMAGE_SIZE = 32768
};
typedef struct _InnerPlayer
{
    //status
    int status;        // Player_Status

    //netWork module
    struct sockaddr_in addr_local;
    struct sockaddr_in addr_remote;
    int socket;
    pthread_t network_thread;
    bool isInterruptNetworkThread;
    pthread_mutex_t mutex_NT;
    pthread_cond_t cond_NT;
    int NTStatus;

    //network_module <-> decoder_module
    JitterBuffer_ptr jitterBuffer;

    //decoder module
    innerDecoder_ptr videoDecoder;
    pthread_t decoder_thread;
    bool isInterruptDecoderThread;
    pthread_mutex_t mutex_DT;
    pthread_cond_t cond_DT;
    int DTStatus;
    timer_t decoderTimer;

    //decoder_module <-> render_module
    LinkedQueue_ptr frameQueue;

    //render module
    ANativeWindow * window;

#ifdef RENDER2
    Render2_ptr videoRender;
    #else
    Render_ptr videoRender;

#endif
    pthread_t render_thread;
    bool isInterruptRenderThread;
    pthread_mutex_t mutex_RT;
    pthread_cond_t cond_RT;
    int RTStatus;

    FILE *fp_dataBeforeDecode_h264 = NULL;
    FILE *fp_dataAfterDecode_yuv = NULL;
    FILE *fp_receivedRTPdata_h264 = NULL;

} IPlayer, *IPlayer_ptr;

static IPlayer_ptr gPlayer = NULL;
static pthread_mutex_t mutex_player = PTHREAD_MUTEX_INITIALIZER;

int getPlayerInstance()
{

    if (0 != pthread_mutex_trylock(&mutex_player))
    {
        return 1;
    }

    int error = Player_Error_NONE;
    IPlayer_ptr iPlayer = NULL;
    //pthread_mutex_lock(&mutex_player);
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

        iPlayer = (IPlayer_ptr) malloc(sizeof(IPlayer));
        if (NULL == iPlayer)
        {
            error = Player_Error_Malloc_Failed;
            LOG_ERROR("allocate memory for IPlayer failed, error : %d",
                    error);
            //return Player_Error_Malloc_Failed;
            pthread_mutex_unlock(&mutex_player);
            return error;
        }
        memset(iPlayer, 0, sizeof(IPlayer));
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
    //pthread_mutex_lock(&mutex_player);
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
    IPlayer_ptr iPlayer = gPlayer;
    if (iPlayer->videoRender != NULL && iPlayer->RTStatus == RUNING)
    {
        iPlayer->videoRender->updateOrientation(iPlayer->videoRender, diffx,
                diffy, diffz);
    }
}

int Player_Init()
{
    int error = Player_Error_NONE;
    IPlayer_ptr iPlayer = gPlayer;
    ANativeWindow *windows[2] =
            { NULL, NULL };

    if (NULL == iPlayer)
    {
        error = Player_Error_Uncreated;
        LOG_ERROR("gPlayer instance unCreated");
        return error;
    }

    pthread_mutex_init(&iPlayer->mutex_NT, NULL);
    pthread_cond_init(&iPlayer->cond_NT, NULL);

    pthread_mutex_init(&iPlayer->mutex_DT, NULL);
    pthread_cond_init(&iPlayer->cond_DT, NULL);

    pthread_mutex_init(&iPlayer->mutex_RT, NULL);
    pthread_cond_init(&iPlayer->cond_RT, NULL);

    iPlayer->NTStatus = PAUSED;
    iPlayer->DTStatus = PAUSED;
    iPlayer->RTStatus = PAUSED;

    //create Jitter Buffer
    iPlayer->jitterBuffer = generateJb(DEFAULT_JB_LENGTH);
    if (NULL == iPlayer->jitterBuffer)
    {
        error = Player_Error_JitterBuffer_Failed;
        LOG_ERROR("create jitter buffer failed, error : %d", error);
        goto createJitterBufferFail;
    }
    memset(&iPlayer->jitterBuffer->lastPopedTs, 0, sizeof(unsigned int));

    //create frame queue
    iPlayer->frameQueue = generateLinkedQueue();
    if (NULL == iPlayer->frameQueue)
    {
        error = Player_Error_FrameQueue_Failed;
        LOG_ERROR("create frame queue failed, error : %d", error);
        goto createFrameQueueFail;
    }

    //create socket
    iPlayer->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == iPlayer->socket)
    {
        error = Player_Error_Socket_Create_Failed;
        LOG_ERROR("socket create failed");
        goto createSocketFail;
    }
    if (-1
            == bind(iPlayer->socket, (struct sockaddr *) &iPlayer->addr_local,
                    sizeof(struct sockaddr)))
    {
        error = Player_Error_Socket_Bind_Failed;
        LOG_ERROR("socket trying to bind to %s:%d failed",
                inet_ntoa(iPlayer->addr_local.sin_addr),
                iPlayer->addr_local.sin_port);
        goto createSocketFail;
    }

    //create decoder
    iPlayer->videoDecoder = (innerDecoder_ptr) getDecoderInstance(
            CODEC_TYPE_DECODER_H264);
    if (NULL == iPlayer->videoDecoder)
    {
        error = Player_Error_Decoder_Create_Failed;
        LOG_ERROR("decoder create failed, error : %d", error);
        goto createDecoderFail;
    }
    //TODO Initialize decoder and config it
    if (-1 == iPlayer->videoDecoder->initialize(iPlayer->videoDecoder, NULL))
    {
        error = Player_Error_Decoder_Initialize_Failed;
        LOG_ERROR("decoder Initialize failed, error : %d", error);
        goto createDecoderFail;
    }

    //create render
    windows[0] = iPlayer->window;
#ifdef RENDER2
    iPlayer->videoRender = createRender2();
#else
    iPlayer->videoRender = createRender((void **) windows, 1,
            Pixel_Format_YUV420P,
            RenderType_Cropping);
#endif
    if (NULL == iPlayer->videoRender)
    {
        error = Player_Error_Render_Create_Failed;
        LOG_ERROR("render create failed , error : %d", error);
        goto createRenderFail;
    }

    iPlayer->isInterruptNetworkThread = false;
    iPlayer->isInterruptDecoderThread = false;
    iPlayer->isInterruptRenderThread = false;
    iPlayer->status = Player_Initialized;
    error = Player_Error_OK;
    goto exit;

    createRenderFail:

    createDecoderFail:
    destroyDecoder(&iPlayer->videoDecoder);

    bindSocketFail:
    if (iPlayer->socket != -1)
    {
        close(iPlayer->socket);
        iPlayer->socket = -1;
    }

    createSocketFail:
    destoryLinkedQueue(&iPlayer->frameQueue);

    createFrameQueueFail:
    destroyJb(iPlayer->jitterBuffer);
    free(iPlayer->jitterBuffer);
    iPlayer->jitterBuffer = NULL;

    createJitterBufferFail:
    iPlayer = NULL;

    exit:
    return error;
}

int Player_Destory()
{
    if (NULL == gPlayer)
    {
        return Player_Error_OK;
    }

    //stop then close player
    switch (gPlayer->status)
    {
        case Player_Opened:

        case Player_Playing:

        case Player_Paused:
            Player_Stop();
        case Player_Stoped:
            Player_Close();
        case Player_Closed:
            break;
        default:        // NONE / Initialized
            break;
    }

    if (NULL != gPlayer->jitterBuffer)
    {
        destroyJb(gPlayer->jitterBuffer);
        free(gPlayer->jitterBuffer);
        gPlayer->jitterBuffer = NULL;
    }

    if (NULL != gPlayer->frameQueue)
    {
        destoryLinkedQueue(&gPlayer->frameQueue);
    }

    //close socket
    close(gPlayer->socket);
    //destory decoder
    destroyDecoderInstance(&gPlayer->videoDecoder);
    //destory render
#ifdef RENDER2
    gPlayer->videoRender->uninitialize(gPlayer->videoRender);
    releaseRender2(&gPlayer->videoRender);
#else
    releaseRender(&gPlayer->videoRender);
#endif

    gPlayer->status = Player_Destroyed;
    return Player_Error_OK;
}

static void networkReceiveSuccessFunc(Lisp_AVPacket pakcet)
{
    Lisp_AVPacket * pPacket = (Lisp_AVPacket *) malloc(sizeof(Lisp_AVPacket));
    memset(pPacket, 0, sizeof(Lisp_AVPacket));
    memcpy(pPacket, &pakcet, sizeof(Lisp_AVPacket));
    u_char startCode[4] =
            { 0x00, 0x00, 0x00, 0x01 };
    u_char * pdata = (u_char *) malloc(pakcet.length + 4);
    memset(pdata, 0, pPacket->length + 4);
    int offset = 0;
    if (pakcet.isNalBeginning)
    {
        memcpy((void *) pdata, (void *) startCode, 4);
        offset = 4;
    }
    memcpy((void *) (pdata + offset), pPacket->NalData, pPacket->length);
    if (PlayerDumpData)
    {
        fwrite(pdata, 1, pakcet.length + offset,
                gPlayer->fp_receivedRTPdata_h264);
    }
    free(pdata);

    LOG_DEBUG(
            "threadId= %d, parse receive data to rtp packet success, seq = %d, ts = %u, isNalBeginning =  %d, ",
            gettid(),
            pPacket->seq, pPacket->ts, pPacket->isNalBeginning);
    gPlayer->jitterBuffer->push(gPlayer->jitterBuffer, pPacket);
}

static void networkReceiveFailureFunc(int errNo, void *buf)
{
    LOG_ERROR("parse receive data to rtp packet failed, error : %d\n",
            errNo);
    free(buf);
}

static void * networkFunction(void *)
{
    H264RtpPacket * packet = new H264RtpPacket();
    unsigned char *recvBuffer = NULL;
    int bufferLen = 0;
    socklen_t sockaddrLen = sizeof(struct sockaddr_in);
    if (PlayerDumpData)
    {
        gPlayer->fp_receivedRTPdata_h264 = fopen(
                "/sdcard/LispReceivedRTPdata.h264",
                "wr");
    }
    while (!gPlayer->isInterruptNetworkThread)
    {
        pthread_mutex_lock(&gPlayer->mutex_NT);
        while (!gPlayer->NTStatus)
        {
            pthread_cond_wait(&gPlayer->cond_NT, &gPlayer->mutex_NT);
        }
        pthread_mutex_unlock(&gPlayer->mutex_NT);

        //TODO logic of network thread.
        recvBuffer = (unsigned char *) malloc(MTU);
        if (NULL != recvBuffer)
        {
            bufferLen = recvfrom(gPlayer->socket, recvBuffer, MTU, 0,
                    (struct sockaddr *) &(gPlayer->addr_remote), &sockaddrLen);

            packet->doParse(recvBuffer, bufferLen, networkReceiveSuccessFunc,
                    networkReceiveFailureFunc);
        }
    }
    if (PlayerDumpData)
    {
        fclose(gPlayer->fp_receivedRTPdata_h264);
    }
    delete packet;
    return NULL;
}

static void resumeNetworkThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (PAUSED == gPlayer->NTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_NT);
        gPlayer->NTStatus = RUNING;
        pthread_cond_signal(&gPlayer->cond_NT);
        LOG_DEBUG("resume NetWork thread");
        pthread_mutex_unlock(&gPlayer->mutex_NT);
    }
    else
    {
        LOG_DEBUG("NetWork thread runs already");
    }
}

static void pauseNetworkThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (RUNING == gPlayer->NTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_NT);
        gPlayer->NTStatus = PAUSED;
        LOG_DEBUG("pause network thread");
        pthread_mutex_unlock(&gPlayer->mutex_NT);
    }
    else
    {
        LOG_DEBUG("network thread pause already");
    }
}

static void decodeSuccessHandler(Lisp_AVFrame * frame)
{
    if (NULL != frame)
    {
        fwrite(frame->data[0], 1, 640 * 480 * 3 / 2,
                gPlayer->fp_dataAfterDecode_yuv);
        LOG_DEBUG("decode h.264 success, frame = %p", frame);
        gPlayer->frameQueue->enQueue(gPlayer->frameQueue, frame);
    }
}

static void decodeFailedHandler(int errNo)
{
    LOG_ERROR("decode h.264 stream failed, errNo =%d", errNo);
}

static void sigHandler(int sigNo)
{
    if (SIGRTMAX == sigNo)
    {
        uint8_t uiStartCode[4] =
                { 0, 0, 0, 1 };
        JitterBufferNode * jbNode = gPlayer->jitterBuffer->pop(
                gPlayer->jitterBuffer);
        if (NULL == jbNode)
        {
            return;
        }

        unsigned char * dataWithStartCode = (unsigned char *) malloc(
                jbNode->datalen + 4);
        int datalengthWithStartCode = 0;
        memcpy(dataWithStartCode, uiStartCode, 4);
        datalengthWithStartCode = 4;
        LinkedList * packetList = jbNode->framePacketList;
        Lisp_AVPacket * packet = NULL;

        while (NULL
                != (packet =
                        (Lisp_AVPacket *) packetList->linkedListDeleteDataAtFirst(
                                packetList)))
        {
            memcpy(dataWithStartCode + datalengthWithStartCode,
                    packet->NalData,
                    packet->length);
            datalengthWithStartCode += packet->length;
            freeAVPacket(&packet);
        }

//        gPlayer->videoDecoder->decodePacket(gPlayer->videoDecoder,
//                dataWithStartCode, datalengthWithStartCode,
//                NULL);
        free(dataWithStartCode);
    }
}

static timer_t createTimer(void (*handler)(int))
{
    timer_t timerid = NULL;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent));
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigevent));

    sa.sa_flags = 0;
    sa.sa_handler = handler;

    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMAX, &sa, NULL) == -1)
    {
        LOG_ERROR("bind signal action failed");
        return NULL;
    }

    evp.sigev_signo = SIGRTMAX;
    evp.sigev_notify = SIGEV_SIGNAL;

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        return NULL;
    }

    return timerid;
}

static int settimerToTimer(timer_t timerid, const struct timeval time)
{
    struct itimerspec ts;
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = time.tv_sec;
    ts.it_interval.tv_nsec = time.tv_usec;

    if (timer_settime(timerid, 0, &ts, NULL) == -1)
    {
        return -1;
    }

    return 0;
}

static int pauseTimer(timer_t timerid)
{
    struct timeval value;
    memset(&value, 0, sizeof(struct timeval));
    return settimerToTimer(timerid, value);
}

static int deleteTimer(timer_t timerid)
{
    return timer_delete(timerid);
}

bool isTimeOffSetEnough(struct timeval currentTime,
        struct timeval lastTime,
        long offsetinMs)
{
    long secOffset = currentTime.tv_sec - lastTime.tv_sec;
    long usecOffset = currentTime.tv_usec - lastTime.tv_usec;

    if ((secOffset * 1000000 + usecOffset) > (offsetinMs * 1000))
    {
        return true;
    }
    return false;
}

bool isTimeZero(struct timeval time)
{
    if (time.tv_sec == 0L && time.tv_usec == 0L)
    {
        return true;
    }
    return false;
}

static void * decoderFunction(void *)
{
    long offsetinMs = 33;
    struct timeval currentTime;
    memset(&currentTime, 0, sizeof(struct timeval));
    struct timezone zone;
    memset(&zone, 0, sizeof(struct timezone));
    struct timeval lastTime;
    memset(&lastTime, 0, sizeof(struct timeval));

    if (PlayerDumpData)
    {
        gPlayer->fp_dataBeforeDecode_h264 = fopen(
                "/sdcard/LispDataBeforeDecode.h264",
                "wb");
        gPlayer->fp_dataAfterDecode_yuv = fopen(
                "/sdcard/LispDataAfterDecode.yuv",
                "wb");
    }
    int frameindex = 0;
    unsigned char * dataWithStartCode =
            (unsigned char *) malloc(
                    MAX_ENCODED_IMAGE_SIZE);
    memset(dataWithStartCode, 0, MAX_ENCODED_IMAGE_SIZE);
    int datalengthWithStartCode = 0;
    while (!gPlayer->isInterruptDecoderThread)
    {
        pthread_mutex_lock(&gPlayer->mutex_DT);
        while (!gPlayer->DTStatus)
        {
            pthread_cond_wait(&gPlayer->cond_DT, &gPlayer->mutex_DT);
        }
        pthread_mutex_unlock(&gPlayer->mutex_DT);

        gettimeofday(&currentTime, &zone);
        if (isTimeZero(lastTime)
                || isTimeOffSetEnough(currentTime, lastTime,
                        offsetinMs))
        {
            memcpy(&lastTime, &currentTime, sizeof(struct timeval));
            uint8_t uiStartCode[4] =
                    { 0, 0, 0, 1 };
            JitterBufferNode * jbNode = gPlayer->jitterBuffer->pop(
                    gPlayer->jitterBuffer);
            if (NULL == jbNode)
            {
                continue;
            }

            LinkedList * packetList = jbNode->framePacketList;
            Lisp_AVPacket * packet = NULL;

            if (packetList == NULL)
            {
                continue;
            }

            while (1)
            {
                packet =
                        (Lisp_AVPacket *) packetList->linkedListDeleteDataAtFirst(
                                packetList);
                if (NULL == packet)
                {
//                    fwrite(dataWithStartCode, 1,
//                            datalengthWithStartCode,
//                            pFpBs);
//                    gPlayer->videoDecoder->decodePacket(
//                            gPlayer->videoDecoder,
//                            dataWithStartCode,
//                            datalengthWithStartCode,
//                            decodeSuccessHandler);
                    memset(dataWithStartCode, 0, MAX_ENCODED_IMAGE_SIZE);
                    datalengthWithStartCode = 0;
                    break;
                }
                int offset = 0;
                if (packet->isNalBeginning)
                {
                    if (datalengthWithStartCode != 0)
                    {

//                        fwrite(dataWithStartCode, 1,
//                                datalengthWithStartCode,
//                                pFpBs);

//                        gPlayer->videoDecoder->decodePacket(
//                                gPlayer->videoDecoder,
//                                dataWithStartCode,
//                                datalengthWithStartCode,
//                                decodeSuccessHandler);

                        memset(dataWithStartCode, 0,
                                MAX_ENCODED_IMAGE_SIZE);

                        datalengthWithStartCode = 0;
                    }

                    memcpy(dataWithStartCode, uiStartCode, 4);

                    datalengthWithStartCode += 4;
                    if (PlayerDumpData)
                    {
                        fwrite(uiStartCode, 1, 4,
                                gPlayer->fp_dataBeforeDecode_h264);
                    }
                }

                memcpy(
                        (void *) (dataWithStartCode
                                + datalengthWithStartCode),
                        packet->NalData,
                        packet->length);

                datalengthWithStartCode += packet->length;
                if (PlayerDumpData)
                {
                    fwrite(packet->NalData, 1, packet->length,
                            gPlayer->fp_dataBeforeDecode_h264);
                }

                if (packet->recvBuf != NULL)
                {
                    free(packet->recvBuf);
                }
                if (NULL != packet)
                {
                    free(packet);
                    packet = NULL;
                }
            }
            frameindex++;
            usleep(1);
        }
    }
    free(dataWithStartCode);
    if (PlayerDumpData)
    {
        fclose(gPlayer->fp_dataAfterDecode_yuv);
        fclose(gPlayer->fp_dataBeforeDecode_h264);
    }
    return NULL;
}

static void resumeDecoderThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (PAUSED == gPlayer->DTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_DT);
        gPlayer->DTStatus = RUNING;
        pthread_cond_signal(&gPlayer->cond_DT);
        LOG_DEBUG("resume decoder thread");
        pthread_mutex_unlock(&gPlayer->mutex_DT);
    }
    else
    {
        LOG_DEBUG("decoder thread runs already");
    }
}

static void pauseDecoderThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (RUNING == gPlayer->DTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_DT);
        gPlayer->DTStatus = PAUSED;
        LOG_DEBUG("pause decoder thread");
        pthread_mutex_unlock(&gPlayer->mutex_DT);
    }
    else
    {
        LOG_DEBUG("decoder thread pause already");
    }
}

static void * renderFunction(void *)
{
    Lisp_AVFrame * pFrame = NULL;
    int error = 0;
    gPlayer->videoRender->setMode(gPlayer->videoRender, RenderType_2D);
    gPlayer->videoRender->setWindow(gPlayer->videoRender, gPlayer->window);
    if (0 != gPlayer->videoRender->initialize(gPlayer->videoRender))
    {
        error = Player_Error_Render_Create_Failed;
        LOG_ERROR("render create failed , error : %d", error);
        releaseRender2(&gPlayer->videoRender);
        return NULL;
    }
    while (!gPlayer->isInterruptRenderThread)
    {
        pthread_mutex_lock(&gPlayer->mutex_RT);
        while (!gPlayer->RTStatus)
        {
            pthread_cond_wait(&gPlayer->cond_RT, &gPlayer->mutex_RT);
        }
        pthread_mutex_unlock(&gPlayer->mutex_RT);

//TODO logic of render thread.

//        gPlayer->videoRender->renderFrame(gPlayer->videoRender,
//                NULL);
        gPlayer->frameQueue->deQueue(gPlayer->frameQueue,
                (void **) &pFrame);
        if (NULL != pFrame)
        {
            gPlayer->videoRender->renderFrame(gPlayer->videoRender,
                    pFrame);

            unsigned char * yuv = pFrame->data[0];
            if (yuv != NULL)
            {
                free(pFrame->data[0]);
                pFrame->data[0] = NULL;
            }
            if (pFrame != NULL)
            {
                free(pFrame);
                pFrame = NULL;
            }
        }

    }
    return NULL;
}

static void resumeRenderThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (PAUSED == gPlayer->RTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_RT);
        gPlayer->RTStatus = RUNING;
        pthread_cond_signal(&gPlayer->cond_RT);
        LOG_DEBUG("resume render thread");
        pthread_mutex_unlock(&gPlayer->mutex_RT);
    }
    else
    {
        LOG_DEBUG("render thread runs already");
    }
}

static void pauseRenderThread()
{
    if (NULL == gPlayer)
    {
        return;
    }

    if (RUNING == gPlayer->RTStatus)
    {
        pthread_mutex_lock(&gPlayer->mutex_RT);
        gPlayer->RTStatus = PAUSED;
        LOG_DEBUG("pause render thread");
        pthread_mutex_unlock(&gPlayer->mutex_RT);
    }
    else
    {
        LOG_DEBUG("render thread pause already");
    }
}

int Player_Open()
{
    int error = Player_Error_NONE;
    int ret = -1;
    if (NULL == gPlayer
            && (Player_NONE == gPlayer->status
                    || Player_Destroyed == gPlayer->status))
    {
        LOG_ERROR("please initialize player first");
//return Player_Error_Uninitialized;
        error = Player_Error_Uninitialized;
        goto exit;
    }

    if (gPlayer->status >= Player_Opened)
    {
        LOG_DEBUG("player has been opened");
//return Player_Error_OK;
        error = Player_Error_OK;
        goto exit;
    }

    gPlayer->isInterruptNetworkThread = false;
    gPlayer->NTStatus = PAUSED;
    if (0
            != (ret = pthread_create(&gPlayer->network_thread, NULL,
                    networkFunction, gPlayer)))
    {
        LOG_ERROR("create network thread failed, inner error : %d",
                ret);
        error = Plyaer_Error_Thread_Create_Failed;
        goto networkThreadCreateFail;
    }

    gPlayer->isInterruptDecoderThread = false;
    gPlayer->DTStatus = PAUSED;
    if (0
            != (ret = pthread_create(&gPlayer->decoder_thread, NULL,
                    decoderFunction, gPlayer)))
    {
        LOG_ERROR("create decoder thread failed, inner error : %d",
                ret);
        error = Plyaer_Error_Thread_Create_Failed;
        goto decoderThreadCreateFail;
    }

    gPlayer->isInterruptRenderThread = false;
    gPlayer->RTStatus = PAUSED;
    if (0
            != (ret = pthread_create(&gPlayer->render_thread, NULL,
                    renderFunction, gPlayer)))
    {
        LOG_ERROR("create render thread failed, inner error : %d",
                ret);
        error = Plyaer_Error_Thread_Create_Failed;
        goto renderThreadCreateFaile;
    }

    gPlayer->status = Player_Opened;
    error = Player_Error_OK;
    goto exit;

    networkThreadCreateFail:
    //TODO NONE

    decoderThreadCreateFail:
    //TODO cancel network thread
    gPlayer->isInterruptNetworkThread = true;
    pthread_join(gPlayer->network_thread, NULL);

    renderThreadCreateFaile:
    //TODO cancel network thread, cancel decoder thread
    gPlayer->isInterruptDecoderThread = true;
    pthread_join(gPlayer->decoder_thread, NULL);

    exit:
    return error;
}

int Player_Close()
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }

    gPlayer->isInterruptNetworkThread = true;
    gPlayer->isInterruptDecoderThread = true;
    gPlayer->isInterruptRenderThread = true;

    pthread_join(gPlayer->network_thread, NULL);
    pthread_join(gPlayer->decoder_thread, NULL);
    pthread_join(gPlayer->render_thread, NULL);
    gPlayer->status = Player_Closed;
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

    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }
    resumeNetworkThread();
    resumeDecoderThread();
    resumeRenderThread();
    gPlayer->status = Player_Playing;

    return Player_Error_OK;
}

int Player_Pause()
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }
    pauseNetworkThread();
    pauseDecoderThread();
    pauseRenderThread();
    gPlayer->status = Player_Paused;

    return Player_Error_OK;
}

int Player_Stop()
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }
    pauseNetworkThread();
    pauseDecoderThread();
    pauseRenderThread();
    gPlayer->status = Player_Stoped;

    return Player_Error_OK;
}

int Player_SetLocalIPAndPort(const char * ipaddr, const short port)
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }

    if (NULL == ipaddr && port <= 0)
    {
        LOG_ERROR("please provide valid parameter");
        return Player_Error_Invalied_Parameters;
    }

    bzero(&gPlayer->addr_local, sizeof(gPlayer->addr_local));
    gPlayer->addr_local.sin_family = AF_INET;
    gPlayer->addr_local.sin_port = htons(port);
    gPlayer->addr_local.sin_addr.s_addr = inet_addr(ipaddr);

    return Player_Error_OK;
}

int Player_SetRemoteIpAndPort(const char * ipaddr, const short port)
{
    if (NULL == gPlayer)
    {
        LOG_ERROR("please initialize player first");
        return Player_Error_Uninitialized;
    }

    if (NULL == ipaddr && port <= 0)
    {
        LOG_ERROR("please provide valid parameter");
        return Player_Error_Invalied_Parameters;
    }

    bzero(&gPlayer->addr_remote, sizeof(gPlayer->addr_remote));
    gPlayer->addr_remote.sin_family = AF_INET;
    gPlayer->addr_remote.sin_port = htons(port);
    gPlayer->addr_remote.sin_addr.s_addr = inet_addr(ipaddr);

    return Player_Error_OK;
}
