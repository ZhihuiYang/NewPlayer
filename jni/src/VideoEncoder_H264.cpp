/*
 * VideoEncoder_H264.cpp
 *
 *  Created on: 2016-10-5
 *      Author: yangzhihui
 */
#include "include/VideoEncoder.h"
#include <string.h>
#include "include/logger.h"
static int g_LevelSetting = WELS_LOG_ERROR;

static bool encodeframe(Video_Encoder_ptr pVideoEncoder,
        const Lisp_AVFrame * frame, Encoder_Output_ptr * output)
{
    if (frame == NULL)
    {
        return false;
    }
    
    ISVCEncoder * pEncoder = NULL;
    pEncoder = (ISVCEncoder *) pVideoEncoder->pEncoderInstance;
    if (pEncoder == NULL)
    {
        LOG_DEBUG("please create and initialize video encoder first");
        return false;
    }
    
    SSourcePicture * pSrcPic = NULL;
    pSrcPic = (SSourcePicture *) malloc(sizeof(SSourcePicture));
    if (pSrcPic == NULL)
    {
        LOG_DEBUG("allocate SSourcePicture failed");
        return false;
    }
    memset(pSrcPic, 0, sizeof(SSourcePicture));
    //fill default pSrcPic
    pSrcPic->iColorFormat = videoFormatI420;
    //pSrcPic->uiTimeStamp = frame->pts;
    pSrcPic->iPicHeight = frame->height;
    pSrcPic->iPicWidth = frame->width;
    
    pSrcPic->iStride[0] = frame->stride[0];
    pSrcPic->iStride[1] = pSrcPic->iStride[2] = (pSrcPic->iStride[0]) >> 1;
    
    pSrcPic->pData[0] = frame->data[0];
    pSrcPic->pData[1] = frame->data[1];
    pSrcPic->pData[2] = frame->data[2];
    
    SFrameBSInfo * pInfo = (SFrameBSInfo *) *output;
    int ret = 0;
    if ((ret = pEncoder->EncodeFrame(pSrcPic, pInfo)) == 0)
    {
        goto encSuccess;
    }
    else
    {
        LOG_DEBUG("encode frame failed, errNo = %d", ret);
        goto encFail;
    }
    
    encSuccess:
    free(pSrcPic);
    pSrcPic = NULL;
    return true;
    
    encFail:
    free(pSrcPic);
    pSrcPic = NULL;
    return false;
}

static void destoryvideoencoder(Video_Encoder_ptr pVideoEncoder)
{
    if (pVideoEncoder != NULL && pVideoEncoder->pEncoderInstance != NULL)
    {
        ISVCEncoder * encTemp =
                (ISVCEncoder *) pVideoEncoder->pEncoderInstance;
        encTemp->Uninitialize();
        WelsDestroySVCEncoder(encTemp);
        encTemp = NULL;
    }
}

static bool initialize(Video_Encoder_ptr pVideoEncoder,
        const void * parameter)
{
    const SEncParamBase * para = (const SEncParamBase *) parameter;
    ISVCEncoder * pEncoder = NULL;
    int ret = 0;
    //Create Openh264 Encoder
    if (pVideoEncoder->pEncoderInstance == NULL)
    {
        if (0 != (ret = WelsCreateSVCEncoder(&pEncoder)))
        {
            LOG_ERROR("WelsCreateSVCEncoder failed, errNo : %d", ret);
            return false;
        }
        pVideoEncoder->pEncoderInstance = pEncoder;
    }
    else
    {
        pEncoder = (ISVCEncoder *) pVideoEncoder->pEncoderInstance;
    }
    
    SEncParamBase * param;
    param = (SEncParamBase *) malloc(sizeof(SEncParamBase));
    if (param == NULL)
    {
        LOG_ERROR("allocate memory for SEncParamBase failed");
        return false;
    }
    memset(param, 0, sizeof(SEncParamBase));
    
    param->iUsageType = CAMERA_VIDEO_REAL_TIME;
    param->fMaxFrameRate = para->fMaxFrameRate;
    param->iRCMode = RC_QUALITY_MODE;
    param->iPicWidth = para->iPicWidth;
    param->iPicHeight = para->iPicHeight;
    param->iTargetBitrate = para->iTargetBitrate;
    
    if ((ret = pEncoder->Initialize(param)) != 0)
    {
        LOG_ERROR("SVCEncoder initialize failed, errNo : %d", ret);
        free(param);
        param = NULL;
        return false;
    }
    
    int videoFormat = videoFormatI420;
    pEncoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
    //pEncoder->SetOption(ENCODER_OPTION_TRACE_LEVEL, &g_LevelSetting);
    
    free(param);
    param = NULL;
    return true;
}

Video_Encoder_ptr createVideoEncoderH264()
{
    Video_Encoder_ptr encoder = NULL;
    encoder = (Video_Encoder_ptr) malloc(sizeof(Video_Encoder));
    memset(encoder, 0, sizeof(Video_Encoder));
    if (encoder != NULL)
    {
        encoder->initialize = initialize;
        encoder->encodeframe = encodeframe;
        encoder->destoryvideoencoder = destoryvideoencoder;
    }
    return encoder;
}
