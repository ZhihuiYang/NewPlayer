/*
 * recorder.cpp
 *
 *  Created on: 2016年10月7日
 *      Author: yangzhihui
 */

#include "include/logger.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "include/VideoEncoder.h"
#include <pthread.h>
#include "include/h264_RtpPacket.h"
#include "include/memoryPool.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include "include/LinkedQueue.h"
#include "include/AVFrame.h"
#include "include/recorder.h"
#include "include/VideoEncoderFactory.h"

#ifdef DEGUB_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

#ifdef DUMP_DATA
bool RecorderDumpData = true;
#else
bool RecorderDumpData = false;
#endif
#define RUNING 1
#define PAUSED 0

typedef struct _InnerRecorder
{
    int status;

    //network module
    struct sockaddr_in addr_local;
    struct sockaddr_in addr_remote;
    int socket;

    //encoder module
    Video_Encoder_ptr videoEncoder;
    pthread_t encoder_thread;
    pthread_mutex_t mutex_ET;
    pthread_cond_t cond_ET;
    int ETStatus;
    bool isInterruptEncoderThread;

    //main Thread <-> encoder thread
    LinkedQueue_ptr frameQueue;

    FILE* fp_dataAfterEncoded_h264 = NULL;
} IRecorder, *IRecorder_ptr;

static IRecorder_ptr gRecorder = NULL;
static pthread_mutex_t mutex_recorder = PTHREAD_MUTEX_INITIALIZER;

int getRecorderInstance()
{
    if (0 != pthread_mutex_trylock(&mutex_recorder))
    {
        return 1;
    }
    int error = Recorder_Error_NONE;
    IRecorder_ptr iRecorder = NULL;
    //pthread_mutex_lock(&mutex_recorder);
    do
    {
        if (NULL != gRecorder)
        {
            if (Recorder_Initialized == gRecorder->status)
            {
                error = Recorder_Error_OK;
                pthread_mutex_unlock(&mutex_recorder);
                return error;
            }
            break;
        }
        
        iRecorder = (IRecorder_ptr) malloc(sizeof(IRecorder));
        if (NULL == iRecorder)
        {
            error = Recorder_Error_Malloc_Failed;
            LOG_ERROR("allocate memory for IRecorder failed, error : %d",
                    error);
            pthread_mutex_unlock(&mutex_recorder);
            return error;
        }
        memset(iRecorder, 0, sizeof(IRecorder));
        iRecorder->status = Recorder_NONE;
        gRecorder = iRecorder;
    } while (0);
    pthread_mutex_unlock(&mutex_recorder);
    return 0;
}

int destroyRecorderInstance()
{
    if (0 != pthread_mutex_trylock(&mutex_recorder))
    {
        return 1;
    }
    //pthread_mutex_lock(&mutex_recorder);
    if (NULL == gRecorder)
    {
        pthread_mutex_unlock(&mutex_recorder);
        return 0;
    }
    
    if (gRecorder->status > Recorder_Destroyed)
    {
        LOG_WARNING("player haven't been destroyed");
        pthread_mutex_unlock(&mutex_recorder);
        return -1;
    }
    
    free(gRecorder);
    gRecorder = NULL;
    pthread_mutex_unlock(&mutex_recorder);
    pthread_mutex_destroy(&mutex_recorder);
    return 0;
}

static void convertNV12toYUV420P(const unsigned char* image_src,
        unsigned char* image_dst,
        int image_width, int image_height)
{
    unsigned char* p = image_dst;
    memcpy(p, image_src, image_width * image_height * 3 / 2);
    const unsigned char* pNV = image_src + image_width * image_height;
    unsigned char* pU = p + image_width * image_height;
    unsigned char* pV = p + image_width * image_height
            + ((image_width * image_height) >> 2);
    for (int i = 0; i < (image_width * image_height) / 2; i++)
    {
        if ((i % 2) == 0)
            *pU++ = *(pNV + i);
        else
            *pV++ = *(pNV + i);
    }
}

static void convertNV21toYUV420P(const unsigned char* image_src,
        unsigned char* image_dst,
        int image_width, int image_height)
{
    unsigned char* p = image_dst;
    memcpy(p, image_src, image_width * image_height * 3 / 2);
    const unsigned char* pNV = image_src + image_width * image_height;
    unsigned char* pU = p + image_width * image_height;
    unsigned char* pV = p + image_width * image_height
            + ((image_width * image_height) >> 2);
    for (int i = 0; i < (image_width * image_height) / 2; i++)
    {
        if ((i % 2) == 0)
            *pV++ = *(pNV + i);
        else
            *pU++ = *(pNV + i);
    }
}

static unsigned char * getImageBuffer(int size)
{
    unsigned char *image = NULL;
    image = (unsigned char *) malloc(size);
    if (NULL == image)
    {
        LOG_ERROR("allocate image buffer failed!");
    }
    return image;
}

static void destoryImageBuffer(unsigned char ** ppImage)
{
    unsigned char * pImage = *ppImage;
    if (NULL == ppImage || NULL == pImage)
    {
        return;
    }
    free(pImage);
    pImage = NULL;
}

//JNIEnv * gEnv = NULL;
//
//JNIEXPORT jint JNI_OnLoad(JavaVM * vm, void * reserved)
//{
//    jint result = -1;
//    if (NULL == gEnv)
//    {
//        if (vm->GetEnv((void **) &gEnv, JNI_VERSION_1_4) != JNI_OK)
//        {
//            return result;
//        }
//    }
//
//    return JNI_VERSION_1_4;
//}

//void openCamera()
//{
//    //this function call java Recorder.openCamera()
//
//    jclass clazz = NULL;
//    jobject jobj = NULL;
//    jmethodID openCameraJava = NULL;
//
//    if (NULL == gEnv)
//    {
//        MY_LOG_FATAL("JNI get Env failed");
//        return;
//    }
//
//    clazz = gEnv->FindClass("com/ericsson/lispmediaplayer/Recorder");
//    if (NULL == clazz)
//    {
//        MY_LOG_ERROR(
//                "can not find class com.ericsson.lispmediaplayer.Recorder");
//        return;
//    }
//
//    openCameraJava = gEnv->GetMethodID(clazz, "openCamera", "()V");
//    //TODO 调用java 的openCamera 方法
//}

static time_t getCurrentUTCTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((time_t) tv.tv_sec * (time_t) 1000000 + tv.tv_usec);
}

void nativeSendYuvData(JNIEnv *env, jobject thiz, jbyteArray yuvData,
        jint width, jint height)
{
    
    int len = env->GetArrayLength(yuvData);
    LOG_DEBUG("width = %d, height = %d, data length = %d", width, height,
            len);
    jbyte * byteBuf = env->GetByteArrayElements(yuvData, NULL);
    
    //convert NV21 to YUV420p
    unsigned char * imageBuffer = getImageBuffer(width * height * 3 / 2);
    convertNV21toYUV420P((const unsigned char *) byteBuf, imageBuffer,
            (int) width,
            (int) height);
    
    if (NULL != gRecorder && NULL != gRecorder->frameQueue)
    {
        Lisp_AVFrame * frame = (Lisp_AVFrame *) malloc(sizeof(Lisp_AVFrame));
        if (NULL == frame)
        {
            LOG_ERROR("allocate frame failed");
            destoryImageBuffer(&imageBuffer);
        }
        
        memset(frame, 0, sizeof(Lisp_AVFrame));
        frame->width = width;
        frame->height = height;
        frame->stride[0] = width;
        frame->pts = getCurrentUTCTime();
        frame->dts = frame->pts;
        frame->data[0] = imageBuffer;
        frame->data[1] = imageBuffer + (width * height);
        frame->data[2] = imageBuffer + (width * height) + (width * height / 4);
        frame->dataSize[0] = frame->width * frame->height;
        frame->dataSize[1] = frame->width * frame->height / 4;
        frame->dataSize[2] = frame->width * frame->height / 4;
        gRecorder->frameQueue->enQueue(gRecorder->frameQueue, frame);
        LOG_DEBUG(
                "enqueue frame to frame queue success, width = %d, height = %d",
                width, height);
    }
    else
    {
        LOG_WARNING("gRecorder instance haven't been created");
    }
    env->ReleaseByteArrayElements(yuvData, (jbyte *) byteBuf, JNI_COMMIT);
}

int Recorder_Init()
{
    int error = Recorder_Error_NONE;
    IRecorder_ptr iRecorder = gRecorder;
    if (NULL == iRecorder)
    {
        error = Recoder_Error_Uncreated;
        LOG_ERROR("gRecorder instance unCreated");
        return error;
    }
    
    pthread_mutex_init(&iRecorder->mutex_ET, NULL);
    pthread_cond_init(&iRecorder->cond_ET, NULL);
    iRecorder->ETStatus = PAUSED;
    
    //create frame queue
    iRecorder->frameQueue = generateLinkedQueue();
    if (NULL == iRecorder->frameQueue)
    {
        error = Recorder_Error_FrameQueue_Failed;
        LOG_ERROR("create frame queue failed, error : %d", error);
        goto createFrameQueeuFail;
    }
    
    //create socket
    iRecorder->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == iRecorder->socket)
    {
        error = Recorder_Error_Socket_Create_Failed;
        LOG_ERROR("socket create failed");
        goto createSocketFail;
    }
    
    if (-1
            == bind(iRecorder->socket,
                    (struct sockaddr *) &iRecorder->addr_local,
                    sizeof(struct sockaddr)))
    {
        error = Recorder_Error_Socket_Bind_Failed;
        LOG_ERROR("socket trying to bind to %s:%d failed",
                inet_ntoa(iRecorder->addr_local.sin_addr),
                iRecorder->addr_local.sin_port);
        goto bindSocketFail;
    }
    
    //create encoder
    iRecorder->videoEncoder = (Video_Encoder_ptr) getVideoEncoderInstance(
            CODEC_TYPE_ENCODER_H264);
    if (NULL == iRecorder->videoEncoder)
    {
        error = Recorder_Error_Encoder_Created_Failed;
        LOG_ERROR("encoder create failed, error : %d", error);
        goto encoderCreateFail;
    }
    
    SEncParamBase para;
    memset(&para, 0, sizeof(SEncParamBase));
    para.fMaxFrameRate = 30;
    para.iPicHeight = 480;
    para.iPicWidth = 640;
    para.iRCMode = RC_QUALITY_MODE;
    para.iTargetBitrate = 25000;
    if (false
            == iRecorder->videoEncoder->initialize(iRecorder->videoEncoder,
                    (const void *) &para))
    {
        LOG_ERROR("video encode initialize failed");
        error = Recorder_Error_Encoder_Initialize_Failed;
        goto encoderInitFail;
    }
    
    iRecorder->isInterruptEncoderThread = false;
    iRecorder->status = Recorder_Initialized;
    gRecorder = iRecorder;
    goto exit;
    
    encoderInitFail:
    destroyVideoEncoderInstance(&iRecorder->videoEncoder);
    
    encoderCreateFail:

    bindSocketFail:
    if (iRecorder->socket != -1)
    {
        close(iRecorder->socket);
        iRecorder->socket = -1;
    }
    
    createSocketFail:
    destoryLinkedQueue(&iRecorder->frameQueue);
    
    createFrameQueeuFail:
    iRecorder = NULL;
    
    exit:
    return error;
}

int Recorder_Destory()
{
    if (NULL == gRecorder)
    {
        return Recorder_Error_OK;
    }
    
//stop then close Recorder
    switch (gRecorder->status)
    {
        case Recorder_Started:

        case Recorder_Recording:

        case Recorder_Paused:
            //Stop()
            Recorder_Stop();
        case Recorder_Stoped:
            //Close()
            
        case Recorder_Closed:
            break;
            
        default:
            break;
    }
    
    if (NULL != gRecorder->videoEncoder)
    {
        destroyVideoEncoderInstance(&gRecorder->videoEncoder);
    }
    
    if (NULL != gRecorder->frameQueue)
    {
        destoryLinkedQueue(&gRecorder->frameQueue);
    }
    gRecorder->status = Recorder_Destroyed;
    
    return Recorder_Error_OK;
}

static void packageSuccessFunc(u_char *buf, int length)
{
    RTP_HDR_T * rtp_h = (RTP_HDR_T *) buf;
    LOG_DEBUG(
            "package h.264 stream success, seq = %d, ts = %u, package length = %d",
            ntohs(rtp_h->seq), ntohl(rtp_h->ts), length);
    int n = sendto(gRecorder->socket, (const void *) buf, length, 0,
            (struct sockaddr *) &gRecorder->addr_remote,
            sizeof(struct sockaddr_in));
}

static void packageFailureFunc(int errNo)
{
    LOG_ERROR("package h.264 stream failed! errNo: %d", errNo);
    
}

static void * encoderFunction(void * recorder)
{
    IRecorder * iRecorder = (IRecorder *) recorder;
    Lisp_AVFrame * frame = NULL;
    PMEMORYPOOL pMemoryPool = generateRTPMemoryPool(30, 1500);
    H264RtpPacket * packet = new H264RtpPacket(pMemoryPool, 30);
    struct _Encoder_Output *outputStream = (struct _Encoder_Output *) malloc(
            sizeof(struct _Encoder_Output));
    
    if (RecorderDumpData)
    {
        gRecorder->fp_dataAfterEncoded_h264 = fopen(
                "/sdcard/LispDataAfterEncoded.h264", "wb");
    }
    while (!iRecorder->isInterruptEncoderThread)
    {
        memset(outputStream, 0, sizeof(struct _Encoder_Output));
        frame = NULL;
        iRecorder->frameQueue->deQueue(iRecorder->frameQueue,
                (void **) &frame);
        if (NULL != frame)
        {
            LOG_DEBUG(
                    "got frame (%p) from framequeue, start to encoding it",
                    frame);
            bool ret = iRecorder->videoEncoder->encodeframe(
                    iRecorder->videoEncoder,
                    frame, &outputStream);
            if (frame->data != NULL && frame->data[0] != NULL)
            {
                free(frame->data[0]);
                frame->data[0] = NULL;
                frame->data[1] = NULL;
                frame->data[2] = NULL;
            }
            if (frame != NULL)
            {
                free(frame);
                frame = NULL;
            }
            
            if (encVideoFrameTypeSkip == outputStream->eFrameType)
            {
                continue;
            }
            int iLayer = 0;
            int iFrameSize = 0;
            while (iLayer < outputStream->iLayerNum)
            {
                Layer_BS_Info * playerBsInfo = &outputStream->sLayerInfo[iLayer];
                if (NULL != playerBsInfo)
                {
                    char nal_type = 0;
                    int offset = 0;
                    int iLayerSize = 0;
                    int iNalIdx = playerBsInfo->iNalCount - 1;
                    do
                    {
                        iLayerSize += playerBsInfo->pNalLengthInByte[iNalIdx];
                        --iNalIdx;
                    } while (iNalIdx >= 0);
                    
                    if (RecorderDumpData)
                    {
                        fwrite(playerBsInfo->pBsBuf, 1, iLayerSize,
                                gRecorder->fp_dataAfterEncoded_h264);
                    }
                    
                    for (int nal_index = 0; nal_index < playerBsInfo->iNalCount;
                            nal_index++)
                    {
                        u_char * buf = playerBsInfo->pBsBuf + offset;
                        nal_type = (buf[4] & 0x1F);
//                        if (nal_type == 14)
//                        {
//                            continue;
//                        }
                        packet->doPackage(
                                buf,
                                playerBsInfo->pNalLengthInByte[nal_index],
                                0,
                                packageSuccessFunc, packageFailureFunc);
                        offset += playerBsInfo->pNalLengthInByte[nal_index];
                    }
                }
                ++iLayer;
            }
        }
    }
    if (RecorderDumpData)
    {
        fclose(gRecorder->fp_dataAfterEncoded_h264);
    }
    free(outputStream);
    delete packet;
    destoryRTPMemoryPool(pMemoryPool);
}

int Recorder_Start()
{
    int error = Recorder_Error_NONE;
    int ret = -1;
    if (NULL == gRecorder || gRecorder->status < Recorder_Initialized)
    {
        LOG_ERROR("please initialize recorder first");
        error = Recorder_Error_Uninitialized;
        return error;
    }
    
    if (gRecorder->status >= Recorder_Started)
    {
        LOG_DEBUG("recorder has been started");
        error = Recorder_Error_OK;
        return error;
    }
    
    gRecorder->isInterruptEncoderThread = false;
    gRecorder->ETStatus = RUNING;
    if (0
            != (ret = pthread_create(&gRecorder->encoder_thread, NULL,
                    encoderFunction, gRecorder)))
    {
        LOG_ERROR("create encoder thread failed, inner error : %d", ret);
        error = Recorder_Error_Thread_Create_Failed;
        return error;
    }
    
    error = Recorder_Error_OK;
    
    return error;
}

int Recorder_Stop()
{
    if (NULL == gRecorder)
    {
        LOG_ERROR("please initialize recorder first");
        return Recorder_Error_Uninitialized;
    }
    
    gRecorder->isInterruptEncoderThread = true;
    pthread_join(gRecorder->encoder_thread, NULL);
    gRecorder->encoder_thread = -1;
    gRecorder->status = Recorder_Stoped;
    gRecorder->ETStatus = PAUSED;
    
    return Recorder_Error_OK;
}

int Recorder_SetLocalIPAndPort(const char * ipaddr, const short port)
{
    if (NULL == gRecorder)
    {
        LOG_ERROR("please initialize recorder first");
        return Recorder_Error_Uninitialized;
    }
    
    if (NULL == ipaddr && port <= 0)
    {
        LOG_ERROR("please provide valid parameter");
        return Recorder_Error_Invalied_Parameters;
    }
    
    bzero(&gRecorder->addr_local, sizeof(gRecorder->addr_local));
    gRecorder->addr_local.sin_family = AF_INET;
    gRecorder->addr_local.sin_port = htons(port);
    gRecorder->addr_local.sin_addr.s_addr = inet_addr(ipaddr);
    
    return Recorder_Error_OK;
}

int Recorder_SetRemoteIpAndPort(const char * ipaddr, const short port)
{
    if (NULL == gRecorder)
    {
        LOG_ERROR("please initialize recorder first");
        return Recorder_Error_Uninitialized;
    }
    
    if (NULL == ipaddr && port <= 0)
    {
        LOG_ERROR("please provide valid parameter");
        return Recorder_Error_Invalied_Parameters;
    }
    
    bzero(&gRecorder->addr_remote, sizeof(gRecorder->addr_remote));
    gRecorder->addr_remote.sin_family = AF_INET;
    gRecorder->addr_remote.sin_port = htons(port);
    gRecorder->addr_remote.sin_addr.s_addr = inet_addr(ipaddr);
    
    return Recorder_Error_OK;
}
