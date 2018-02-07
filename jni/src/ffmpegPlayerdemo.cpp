/*!
 * \file ffmpegPlayerdemo.cpp
 *
 * \date Created on: Feb 27, 2017
 * \author: eyngzui
 */

#include "ffmpegPlayerdemo.h"
#include <pthread.h>
#include <string.h>
#include "logger.h"
#include "videoRender2.h"
#include "AVFrame.h"
#include <stdio.h>

extern "C"
{
#include "libavutil/adler32.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

typedef struct _FFmpegDemoPlayer
{
    char url[1024];
    bool isVRPlaying;
    pthread_t playerThread;
    ANativeWindow *window;
    Render2_ptr videoRender;
} FFmpegDemoPlayer;

static FFmpegDemoPlayer ffmpegPlayer;
static bool isPlaying = false;

int setSurfaceView(ANativeWindow *window)
{
    if (NULL != window)
    {
        ffmpegPlayer.window = window;
    }
    
    return 0;
}

static void * playThread(void * args)
{
    const char * url = (char *) ((FFmpegDemoPlayer *) args)->url;
    const bool isVRPlaying = ((FFmpegDemoPlayer *) args)->isVRPlaying;
    
    AVCodec *codec = NULL;
    AVCodecContext *ctx = NULL;
    AVCodecParameters *origin_par = NULL;
    AVFrame *fr = NULL;
    uint8_t *byte_buffer = NULL;
    AVPacket pkt;
    AVFormatContext *fmt_ctx = NULL;
    int number_of_written_bytes;
    int video_stream;
    int got_frame = 0;
    int byte_buffer_size;
    int i = 0;
    int result;
    int end_of_stream = 0;
    struct _AvFrame lispFrame;
    memset(&lispFrame, 0, sizeof(struct _AvFrame));
    
    av_register_all();
    
    result = avformat_network_init();
    if (result != 0)
    {
        LOG_ERROR("avformat_network_init failed, error code: %d", result);
        return NULL;
    }
    
    result = avformat_open_input(&fmt_ctx, url, NULL, NULL);
    if (result < 0)
    {
        LOG_ERROR("Can't open stream: %s, error code: %d", url, result);
        return NULL;
    }
    
    result = avformat_find_stream_info(fmt_ctx, NULL);
    if (result < 0)
    {
        LOG_ERROR("Can't get stream info, error code: %d", result);
        return NULL;
    }
    
    video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1,
            NULL, 0);
    if (video_stream < 0)
    {
        LOG_ERROR("Can't find video stream in %s", url);
        return NULL;
    }
    
    origin_par = fmt_ctx->streams[video_stream]->codecpar;
    
    codec = avcodec_find_decoder(origin_par->codec_id);
    if (!codec)
    {
        LOG_ERROR("Can't find decoder");
        return NULL;
    }
    
    ctx = avcodec_alloc_context3(codec);
    if (!ctx)
    {
        LOG_ERROR("Can't allocate decoder context");
        return NULL;
    }
    
    result = avcodec_parameters_to_context(ctx, origin_par);
    if (result)
    {
        LOG_ERROR("Can't copy decoder parameters to context, error code: %d",
                result);
        return NULL;
    }
    
    if (ctx->pix_fmt == AV_PIX_FMT_YUV420P)
    {
        int width = ctx->width;
        int height = ctx->height;
        lispFrame.data[0] = (unsigned char *) malloc(
                width * height * 3 / 2);
        memset(lispFrame.data[0], 0, width * height * 3 / 2);
        lispFrame.dataSize[0] = width * height;
        
        lispFrame.data[1] = lispFrame.data[0] + lispFrame.dataSize[0];
        lispFrame.dataSize[1] = width * height / 4;
        
        lispFrame.data[2] = lispFrame.data[1] + lispFrame.dataSize[1];
        lispFrame.dataSize[2] = lispFrame.dataSize[1];
        
        lispFrame.width = width;
        lispFrame.height = height;
        lispFrame.stride[0] = width;
        lispFrame.stride[1] = width / 2;
        lispFrame.stride[2] = lispFrame.stride[1];
        
    }
    else
    {
        return NULL;
    }
    
    result = avcodec_open2(ctx, codec, NULL);
    if (result < 0)
    {
        LOG_ERROR("Can't open decoder");
        return NULL;
    }
    
    fr = av_frame_alloc();
    if (!fr)
    {
        LOG_ERROR("Can't allocate frame");
        return NULL;
    }
    
    ffmpegPlayer.videoRender = createRender2();
    if (NULL == ffmpegPlayer.videoRender)
    {
        LOG_ERROR("Can't create Render");
        return NULL;
    }
    
    if (isVRPlaying)
    {
        ffmpegPlayer.videoRender->setMode(ffmpegPlayer.videoRender,
                RenderType_VR);
    }
    else
    {
        ffmpegPlayer.videoRender->setMode(ffmpegPlayer.videoRender,
                RenderType_2D);
    }
    ffmpegPlayer.videoRender->setWindow(ffmpegPlayer.videoRender,
            ffmpegPlayer.window);
    
    result = ffmpegPlayer.videoRender->initialize(ffmpegPlayer.videoRender);
    if (result != 0)
    {
        LOG_ERROR("render initialize failed");
        releaseRender2(&ffmpegPlayer.videoRender);
    }
    
    i = 0;
    av_init_packet(&pkt);
    //FILE *fp_yuv;
    //fp_yuv = fopen("/sdcard/ffmpegOutput.yuv", "rb");
    
    do
    {
        if (!end_of_stream)
        {
            if (av_read_frame(fmt_ctx, &pkt) < 0)
            {
                end_of_stream = 1;
            }
        }
        
        if (end_of_stream)
        {
            pkt.data = NULL;
            pkt.size = 0;
        }
        
        if (pkt.stream_index == video_stream || end_of_stream)
        {
            got_frame = 0;
            if (pkt.pts == AV_NOPTS_VALUE)
            {
                pkt.pts = pkt.dts = i;
            }
            
            result = avcodec_decode_video2(ctx, fr, &got_frame, &pkt);
            if (result < 0)
            {
                LOG_ERROR("Error decoding frame: %d", result);
                return NULL;
            }
            
            if (got_frame)
            {
                int y_size = ctx->width * ctx->height;
                memcpy(lispFrame.data[0], fr->data[0], y_size);
                memcpy(lispFrame.data[1], fr->data[1], y_size / 4);
                memcpy(lispFrame.data[2], fr->data[2], y_size / 4);
                
                lispFrame.width = fr->width;
                lispFrame.height = fr->height;
                ffmpegPlayer
                        .videoRender->renderFrame(ffmpegPlayer.videoRender,
                        &lispFrame);
            }
            
            av_packet_unref(&pkt);
            av_init_packet(&pkt);
        }
        i++;
    } while (isPlaying && (!end_of_stream || got_frame));
    
    av_packet_unref(&pkt);
    av_frame_free(&fr);
    avcodec_close(ctx);
    avformat_close_input(&fmt_ctx);
    avcodec_free_context(&ctx);
    //ffmpegPlayer.videoRender->uninitialize(ffmpegPlayer.videoRender);
    releaseRender2(&ffmpegPlayer.videoRender);
    //fclose(fp_yuv);
    return NULL;
}

int startPlay(char * url, bool isVRPlaying)
{
    if (!isPlaying && NULL != url)
    {
        isPlaying = true;
        memset(ffmpegPlayer.url, 0, 1024);
        memcpy(ffmpegPlayer.url, url, strlen(url));
        ffmpegPlayer.isVRPlaying = isVRPlaying;
        if (0 != pthread_create(&ffmpegPlayer.playerThread, NULL, playThread,
                &ffmpegPlayer))
        {
            LOG_ERROR("startPlay failed, create playing thread failed");
            isPlaying = false;
            return -1;
        }
    }
    
    return 0;
}

int stopPlay()
{
    isPlaying = false;
    pthread_join(ffmpegPlayer.playerThread, NULL);
    return 0;
}
