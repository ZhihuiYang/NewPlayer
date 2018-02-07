/*
 * MediaExtractor.h
 *
 *  Created on: Nov 23, 2016
 *      Author: elantia
 */

#ifndef INCLUDE_MEDIAEXTRACTOR_H_
#define INCLUDE_MEDIAEXTRACTOR_H_
#include <stdint.h>
#include "include/AVPacket.h"
#include "include/linkedList.h"
#include <pthread.h>
extern "C"
{
#include "include/ffmpeg/libavformat/avformat.h"
}

typedef struct _VideoTrack
{
    unsigned int index;
    int format;
} VideoTrack;

typedef struct _AudioTrack
{
    unsigned int index;
    int format;
    unsigned char * mime;
} AudioTrack;

typedef struct _SubtitleTrack
{
    unsigned int index;
    int format;
    unsigned char * mime;
} SubtitleTrack;

typedef LinkedList trackList;

typedef int (*callbackFuncPtr)(void *data);

enum Extractor_Status
{
    Extractor_None = -1,
    Extractor_Initialized,
    Extractor_SetDataSource,
    Extractor_Extracting,
    Extractor_Paused,
    Extractor_Stoped,
    Extractor_Uninitialized,
};

typedef struct _MediaExtractor
{
    char * filepath;
    AVFormatContext * avFormatContext;
    pthread_t extractThread;
    bool isInterruptExtractThread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int extractorStatus;
    int extractorThreadStatus;
    callbackFuncPtr videoDataCallback;
    callbackFuncPtr audioDataCallback;
    callbackFuncPtr subtitleDataCallback;

    bool extractVideoDataFlag;
    bool extractAudioDataFlag;
    bool extractSubtitleDataFlag;
    int currentVideoTrackIndex;
    int currentAudioTrackIndex;
    int currentSubtitleTrackIndex;

    int (*initialize)(struct _MediaExtractor *);
    int (*setDataSource)(struct _MediaExtractor *extractor,
            const char *datasource);
    unsigned int (*getTrackCount)(struct _MediaExtractor *extractor);

    int (*getVideoTrackList)(struct _MediaExtractor *, trackList *);
    int (*getAudioTrackList)(struct _MediaExtractor *, trackList *);
    int (*getSubtitleTrackList)(struct _MediaExtractor *, trackList *);

    VideoTrack (*getBestVideoTrack)(struct _MediaExtractor *);
    AudioTrack (*getBestAudioTrack)(struct _MediaExtractor *);
    SubtitleTrack (*getBestSubtitleTrack)(struct _MediaExtractor *);

    int (*startVideoTrack)(struct _MediaExtractor *, VideoTrack);
    int (*startAudioTrack)(struct _MediaExtractor *, AudioTrack);
    int (*startSubtitleTrack)(struct _MediaExtractor *, SubtitleTrack);

    int (*stopVideoTrack)(struct _MediaExtractor *, VideoTrack);
    int (*stopAudioTrack)(struct _MediaExtractor *, AudioTrack);
    int (*stopSubtitleTrack)(struct _MediaExtractor *, SubtitleTrack);

    int (*registerVideoDataCallback)(struct _MediaExtractor *, callbackFuncPtr);
    int (*registerAudioDataCallback)(struct _MediaExtractor *, callbackFuncPtr);
    int (*registerSubtitleDataCallback)(struct _MediaExtractor *,
            callbackFuncPtr);

    int (*pauseExtractor)(struct _MediaExtractor *);
    int (*resumeExtractor)(struct _MediaExtractor *);
    int (*stopExtractor)(struct _MediaExtractor *);
    int (*unInitialize)(struct _MediaExtractor *);
} MediaExtractor, *MediaExtractor_ptr;

MediaExtractor * createMediaExtractor();
void destroyMediaExtractor(MediaExtractor **);
#endif /* INCLUDE_MEDIAEXTRACTOR_H_ */
