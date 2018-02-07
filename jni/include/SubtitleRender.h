/*!
 * \file SubtitleRender.h
 *
 * \date Created on: Jul 17, 2017
 * \author: eyngzui
 */

#ifndef INCLUDE_SUBTITLERENDER_H_
#define INCLUDE_SUBTITLERENDER_H_

#include "ass.h"

typedef struct
{
    char * name;
    char * data;
    int nameSize;
    int dataSize;
} font;

typedef struct subtitleInfo
{
    bool active;
    font ** fonts;
    int fontsSize;
    char * ass_data;
    int assDataSize;
} subtitleInfo, *pSubtitleInfo;

typedef struct _subtitleRender
{
    ASS_Library *library;
    ASS_Renderer *renderer;
    ASS_Track *track;

    void (*initialize)(struct _subtitleRender * subRender,
            subtitleInfo * subInfo, unsigned int width, unsigned int height);
    void (*pushMsg)(struct _subtitleRender * subRender, char *msg, int msgSize,
            double videoPts);
    void (*flush)(struct _subtitleRender *subRender);
    void (*setDimensions)(struct _subtitleRender * subRender,
            unsigned int height, unsigned int width);
    void (*unInitialize)(struct _subtitleRender * subRender);
} SubtitleRender, *pSubtitleRender;

SubtitleRender * createSubRenderer();
void releaseSubRenderer(SubtitleRender ** ppRenderer);

#endif /* INCLUDE_SUBTITLERENDER_H_ */
