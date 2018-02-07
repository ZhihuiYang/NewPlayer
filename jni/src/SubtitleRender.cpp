/*!
 * \file SubtitleRender.cpp
 *
 * \date Created on: Jul 17, 2017
 * \author: eyngzui
 */

#include "SubtitleRender.h"
#include "logger.h"

static void msg_callback(int, const char *, va_list, void *)
{

}

static void initialize(struct _subtitleRender * subRender,
        subtitleInfo * subInfo,
        unsigned int width, unsigned int height)
{
    if (subRender == NULL)
    {
        LOG_ERROR("subtitileRenderer is NULL");
        return;
    }

    subRender->library = ass_library_init();
    ass_set_message_cb(subRender->library, msg_callback, NULL);

    for (int i = 0; i < subInfo->fontsSize; i++)
    {
        font * font = subInfo->fonts[i];
        ass_add_font(subRender->library, font->name, font->data,
                font->dataSize);
    }

    subRender->renderer = ass_renderer_init(subRender->library);
    ass_set_frame_size(subRender->renderer, width, height);
    ass_set_extract_fonts(subRender->library, 1);
    ass_set_fonts(subRender->renderer, NULL, NULL, 1, NULL, 1);
    ass_set_hinting(subRender->renderer, ASS_HINTING_LIGHT);

    subRender->track = ass_new_track(subRender->library);
    ass_process_codec_private(subRender->track, subInfo->ass_data,
            subInfo->assDataSize);
}

static void pushMsg(struct _subtitleRender * subRender, char *msg, int msgSize,
        double videoPts)
{
    ass_process_data(subRender->track, msg, msgSize);
}

static void flush(struct _subtitleRender *subRender)
{
    ass_flush_events(subRender->track);
}

static void setDimensions(struct _subtitleRender * subRender,
        unsigned int height,
        unsigned int width)
{
    if (subRender == NULL)
    {
        LOG_ERROR("subtitile renderer is NULL");
        return;
    }

    ass_set_frame_size(subRender->renderer, width, height);
}

static void unInitialize(struct _subtitleRender * subRender)
{
    if (subRender == NULL)
    {
        LOG_ERROR("subtitle renderer is NULL");
        return;
    }

    ass_free_track(subRender->track);
    ass_renderer_done(subRender->renderer);
    ass_library_done(subRender->library);
}

SubtitleRender *createSubRenderer()
{
    SubtitleRender * pSubtitleRender = (SubtitleRender *) malloc(
            sizeof(SubtitleRender));
    memset(pSubtitleRender, 0, sizeof(SubtitleRender));
    pSubtitleRender->initialize = initialize;
    pSubtitleRender->flush = flush;
    pSubtitleRender->setDimensions = setDimensions;
    pSubtitleRender->pushMsg = pushMsg;
    pSubtitleRender->unInitialize = unInitialize;

    return pSubtitleRender;

}

void releaseSubRenderer(SubtitleRender ** ppRenderer)
{

    if (NULL == ppRenderer || NULL == *ppRenderer)
    {
        return;
    }
    SubtitleRender * pSubtitleRender = *ppRenderer;
    free(pSubtitleRender);
    pSubtitleRender = NULL;

}
