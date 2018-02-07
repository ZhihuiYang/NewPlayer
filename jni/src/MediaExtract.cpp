/*
 * MediaExtract.cpp
 *
 *  Created on: Dec 25, 2016
 *      Author: elantia
 */

#include "include/MediaExtractor.h"
#include <string.h>
#include "logger.h"

#define RUNING 1
#define PAUSED 0

static int initialize(struct _MediaExtractor *extractor)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (extractor->extractorStatus == Extractor_Initialized)
    {
        LOG_INFO("extractor initialized already");
        return -1;
    }

    int result = 0;
    av_register_all();
    result = avformat_network_init();
    if (result != 0)
    {
        LOG_ERROR("avformat_network_init failed, error code: %d", result);
        return result;
    }
    extractor->extractorStatus = Extractor_Initialized;
    return result;
}

static int setDataSource(struct _MediaExtractor *extractor,
        const char *datasource)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (NULL == datasource)
    {
        LOG_ERROR("data source is NULL, please provide datasource");
        return -1;
    }

    size_t urllength = 0;
    int result = 0;
    urllength = strlen(datasource);

    extractor->filepath = (char *) malloc(urllength + 1);
    memset(extractor->filepath, 0, urllength + 1);
    memcpy(extractor->filepath, datasource, urllength);
    const char * pathtemp = extractor->filepath;

    result = avformat_open_input(
            ((AVFormatContext **) &extractor->avFormatContext),
            pathtemp, NULL, NULL);

    if (result < 0)
    {
        LOG_ERROR("Can't open stream: %s, error code: %d", pathtemp, result);
        return result;
    }

    result = avformat_find_stream_info(
            (AVFormatContext *) extractor->avFormatContext,
            NULL);
    if (result < 0)
    {
        LOG_ERROR("Can't get stream info, error code: %d", result);
        return result;
    }

    extractor->extractorStatus = Extractor_SetDataSource;
    return result;
}

static unsigned int getTrackCount(struct _MediaExtractor *extractor)
{

    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    u_int count = 0;
    if (NULL != extractor->avFormatContext)
    {
        count = ((AVFormatContext *) extractor->avFormatContext)->nb_streams;
    }
    return count;
}

static int getVideoTrackList(struct _MediaExtractor *extractor,
        trackList* videoTracklist)
{
    return -1;
}

static int getAudioTruckList(struct _MediaExtractor *,
        trackList * audioTracklist)
{
    return -1;
}

static int getSubtitleTrackList(struct _MediaExtractor *,
        trackList * subtitleTracklist)
{
    return -1;
}

VideoTrack getBestVideoTrack(struct _MediaExtractor * extractor)
{

    VideoTrack videoTrack;
    memset(&videoTrack, 0, sizeof(VideoTrack));

    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return videoTrack;
    }

    videoTrack.index = av_find_best_stream(extractor->avFormatContext,
            AVMEDIA_TYPE_VIDEO, -1, -1,
            NULL, 0);
    if (videoTrack.index < 0)
    {
        LOG_ERROR("Can't find video stream in %s", extractor->filepath);
    }
    else
    {
        AVCodecParameters *para =
                extractor->avFormatContext->streams[videoTrack.index]->codecpar;
        videoTrack.format = para->codec_id;
    }

    return videoTrack;
}

AudioTrack getBestAudioTrack(struct _MediaExtractor * extractor)
{

    AudioTrack audioTrack;
    memset(&audioTrack, 0, sizeof(VideoTrack));

    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return audioTrack;
    }

    audioTrack.index = av_find_best_stream(extractor->avFormatContext,
            AVMEDIA_TYPE_AUDIO, -1, -1,
            NULL, 0);
    if (audioTrack.index < 0)
    {
        LOG_ERROR("Can't find video stream in %s", extractor->filepath);
    }
    else
    {
        AVCodecParameters *para =
                extractor->avFormatContext->streams[audioTrack.index]->codecpar;
        audioTrack.format = para->codec_id;
    }

    return audioTrack;
}

SubtitleTrack getBestSubtitleTrack(struct _MediaExtractor * extractor)
{

    SubtitleTrack subtitleTrack;
    memset(&subtitleTrack, 0, sizeof(VideoTrack));
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return subtitleTrack;
    }

    subtitleTrack.index = av_find_best_stream(extractor->avFormatContext,
            AVMEDIA_TYPE_SUBTITLE, -1, -1,
            NULL, 0);
    if (subtitleTrack.index < 0)
    {
        LOG_ERROR("Can't find video stream in %s", extractor->filepath);
    }
    else
    {
        AVCodecParameters *para =
                extractor->avFormatContext->streams[subtitleTrack.index]->codecpar;
        subtitleTrack.format = para->codec_id;
    }

    return subtitleTrack;
}

static void resumeExtractThread(struct _MediaExtractor * extractor)
{
    if (NULL == extractor)
    {
        return;
    }

    if (PAUSED == extractor->extractorThreadStatus)
    {
        pthread_mutex_lock(&extractor->mutex);
        extractor->extractorThreadStatus = RUNING;
        pthread_cond_signal(&extractor->cond);
        LOG_DEBUG("resume Extract thread");
        pthread_mutex_unlock(&extractor->mutex);
    }
    else
    {
        LOG_DEBUG("ExtractT thread runs already");
    }
}

static void pauseExtractThread(struct _MediaExtractor * extractor)
{
    if (NULL == extractor)
    {
        return;
    }

    if (RUNING == extractor->extractorThreadStatus)
    {
        pthread_mutex_lock(&extractor->mutex);
        extractor->extractorThreadStatus = PAUSED;
        LOG_DEBUG("pause extract thread");
        pthread_mutex_unlock(&extractor->mutex);
    }
    else
    {
        LOG_DEBUG("extract thread pause already");
    }
}

static void * extractThreadHandler(void * arguments)
{
    if (arguments == NULL)
    {
        return NULL;
    }

    struct _MediaExtractor * extractor = (struct _MediaExtractor *) arguments;
    int result;
    int end_of_stream = 0;

    while (!extractor->isInterruptExtractThread && !end_of_stream)
    {
        pthread_mutex_lock(&extractor->mutex);
        while (!extractor->extractorStatus)
        {
            pthread_cond_wait(&extractor->cond, &extractor->mutex);
        }
        pthread_mutex_unlock(&extractor->mutex);

        AVPacket * pkt = av_packet_alloc();
        av_init_packet(pkt);

        if (!end_of_stream)
        {
            if (av_read_frame(extractor->avFormatContext, pkt) < 0)
            {
                end_of_stream = 1;
            }
        }

        if (end_of_stream)
        {
            pkt->data = NULL;
            pkt->size = 0;
        }

        if (pkt->stream_index == extractor->currentVideoTrackIndex
                || end_of_stream)
        {
            if (extractor->extractVideoDataFlag)
            {
                extractor->videoDataCallback(pkt);
            }
        }
        else if (pkt->stream_index == extractor->currentAudioTrackIndex
                || end_of_stream)
        {
            if (extractor->extractVideoDataFlag)
            {
                extractor->audioDataCallback(pkt);
            }
        }
        else if (pkt->stream_index == extractor->currentSubtitleTrackIndex
                || end_of_stream)
        {
            if (extractor->extractSubtitleDataFlag)
            {
                extractor->subtitleDataCallback(pkt);
            }
        }
        else
        {
            av_packet_free(&pkt);
        }

        //av_packet_unref(pkt);
    }

    //av_packet_unref(pkt);
}

static pthread_t createExtractThread(struct _MediaExtractor * extractor)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (extractor->videoDataCallback == NULL
            && extractor->audioDataCallback == NULL
            && extractor->subtitleDataCallback)
    {
        LOG_ERROR("Please register data callback function first");
        return -1;
    }

    int ret = -1;
    pthread_t threadId = -1;
    if (0
            != (ret = pthread_create(&threadId, NULL,
                    extractThreadHandler, extractor)))
    {
        LOG_ERROR("create extractor thread failed, inner error : %d", ret);
        return -1;
    }

    extractor->extractThread = threadId;

    return extractor->extractThread;
}

static int startVideoTrack(struct _MediaExtractor * extractor,
        VideoTrack selectTrack)
{

    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (extractor->extractThread == -1)
    {
        if (createExtractThread(extractor) == -1)
        {
            return -1;
        }
    }

    extractor->currentVideoTrackIndex = selectTrack.index;
    extractor->extractVideoDataFlag = true;
    resumeExtractThread(extractor);
    return 0;
}

static int stopVideoTrack(struct _MediaExtractor * extractor,
        VideoTrack selectTrack)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    extractor->extractVideoDataFlag = false;
    resumeExtractThread(extractor);
    return 0;
}

static int startAudioTrack(struct _MediaExtractor * extractor,
        AudioTrack selectTrack)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (extractor->extractThread == -1)
    {
        if (createExtractThread(extractor) == -1)
        {
            return -1;
        }
    }

    extractor->currentAudioTrackIndex = selectTrack.index;
    extractor->extractAudioDataFlag = true;
    resumeExtractThread(extractor);
    return 0;
}

static int stopAudioTrack(struct _MediaExtractor * extractor,
        AudioTrack selectTrack)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    extractor->extractAudioDataFlag = false;
    resumeExtractThread(extractor);

    return 0;
}

static int startSubtitleTrack(struct _MediaExtractor * extractor,
        SubtitleTrack selectTrack)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    if (extractor->extractThread == -1)
    {
        if (createExtractThread(extractor) == -1)
        {
            return -1;
        }
    }

    extractor->currentSubtitleTrackIndex = selectTrack.index;
    extractor->extractSubtitleDataFlag = true;
    resumeExtractThread(extractor);
    return 0;
}

static int stopSubtitleTrack(struct _MediaExtractor * extractor,
        SubtitleTrack selectTrack)
{
    if (NULL == extractor)
    {
        LOG_ERROR("extractor is NULL");
        return -1;
    }

    extractor->extractSubtitleDataFlag = false;
    resumeExtractThread(extractor);
    return 0;
}

static int registerVideoDataCallback(struct _MediaExtractor * extractor,
        callbackFuncPtr videoCallbackFunc)
{
    if (extractor == NULL || videoCallbackFunc == NULL)
    {
        LOG_ERROR("MediaExtractor is %p, videoDataCallback is %p",
                extractor, videoCallbackFunc);
        return -1;
    }

    extractor->videoDataCallback = videoCallbackFunc;
    return 0;
}

static int registerAudioDataCallback(struct _MediaExtractor * extractor,
        callbackFuncPtr audioCallbackFunc)
{
    if (extractor == NULL || audioCallbackFunc == NULL)
    {
        LOG_ERROR("MediaExtractor is %p, audioDataCallbakc is %p",
                extractor, audioCallbackFunc);
        return -1;
    }

    extractor->audioDataCallback = audioCallbackFunc;
    return 0;
}

static int registerSubtitleDataCallback(struct _MediaExtractor * extractor,
        callbackFuncPtr subtitleCallbackFunc)
{
    if (extractor == NULL || subtitleCallbackFunc == NULL)
    {
        LOG_ERROR("MediaExtractor is %p, subtitleDataCallbakc is %p",
                extractor, subtitleCallbackFunc);
        return -1;
    }

    extractor->subtitleDataCallback = subtitleCallbackFunc;
    return 0;
}

static int pauseExtractor(struct _MediaExtractor * extractor)
{
    pauseExtractThread(extractor);
    return 0;
}

static int resumeExtractor(struct _MediaExtractor * extractor)
{
    resumeExtractThread(extractor);
    return 0;
}

static int stopExtractor(struct _MediaExtractor * extractor)
{
    extractor->isInterruptExtractThread = true;
    extractor->extractSubtitleDataFlag = false;
    extractor->extractAudioDataFlag = false;
    extractor->extractVideoDataFlag = false;
    resumeExtractThread(extractor);
    return 0;
}

static int unInitialize(struct _MediaExtractor *)
{

}

MediaExtractor * createMediaExtractor()
{
    MediaExtractor_ptr extractorPtr = NULL;
    extractorPtr = (MediaExtractor *) malloc(sizeof(MediaExtractor));
    if (NULL != extractorPtr)
    {
        memset(extractorPtr, 0, sizeof(MediaExtractor));
        extractorPtr->initialize = initialize;
        extractorPtr->setDataSource = setDataSource;
        extractorPtr->getTrackCount = getTrackCount;
        extractorPtr->getVideoTrackList = getVideoTrackList;
        extractorPtr->getAudioTrackList = getAudioTruckList;
        extractorPtr->getSubtitleTrackList = getSubtitleTrackList;
        extractorPtr->startVideoTrack = startVideoTrack;
        extractorPtr->startAudioTrack = startAudioTrack;
        extractorPtr->startSubtitleTrack = startSubtitleTrack;
        extractorPtr->stopVideoTrack = stopVideoTrack;
        extractorPtr->stopAudioTrack = stopAudioTrack;
        extractorPtr->stopSubtitleTrack = stopSubtitleTrack;
        extractorPtr->registerVideoDataCallback = registerVideoDataCallback;
        extractorPtr->registerAudioDataCallback = registerAudioDataCallback;
        extractorPtr->registerVideoDataCallback = registerSubtitleDataCallback;
        extractorPtr->unInitialize = unInitialize;
        extractorPtr->pauseExtractor = pauseExtractor;
        extractorPtr->resumeExtractor = resumeExtractor;
        extractorPtr->stopExtractor = stopExtractor;
        extractorPtr->filepath = NULL;
        extractorPtr->extractThread = -1;
        extractorPtr->extractVideoDataFlag = false;
        extractorPtr->extractAudioDataFlag = false;
        extractorPtr->extractSubtitleDataFlag = false;
        pthread_mutex_init(&extractorPtr->mutex, NULL);
        pthread_cond_init(&extractorPtr->cond, NULL);
        extractorPtr->extractorStatus = Extractor_Uninitialized;
        extractorPtr->extractorThreadStatus = PAUSED;
    }
    return extractorPtr;
}
void destroyMediaExtractor(MediaExtractor ** ppExtractor)
{
    if (NULL == ppExtractor || NULL == *ppExtractor)
    {
        return;
    }

    MediaExtractor * pExtractor = *ppExtractor;
    pExtractor->unInitialize(pExtractor);

    pthread_mutex_destroy(&pExtractor->mutex);
    pthread_cond_destroy(&pExtractor->cond);

    memset(pExtractor, 0, sizeof(MediaExtractor));
    free(pExtractor);
    pExtractor = NULL;
}
