/*!
 * \file h264_RtpPacket.h
 *
 * \date Created on: Jul 25, 2016
 * \author: eyngzui
 */

#ifndef INCLUDE_H264_RTPPACKET_H_
#define INCLUDE_H264_RTPPACKET_H_

#include "RTPPacket.h"
#include "AVPacket.h"

class H264RtpPacket: public RTPPacket
{
public:
    H264RtpPacket(PMEMORYPOOL rtpPool, unsigned int fps);
    H264RtpPacket();
    ~H264RtpPacket();

    int doPackage(u_char *buf, int length, long long timeStamp,
            void (*successFunc)(u_char *buf, int length),
            void (*failureFunc)(int errNo));
    int doParse(u_char *buf, int length,
            void (*successFunc)(Lisp_AVPacket data),
            void (*failureFunc)(int errNo, void *));

private:
    void packFragmentHeader();
    int packFragmentPayload(u_char *buf, int length);
    int packFragmentPayloadWithFUA(u_char *buf, int length,
            uint8_t fu_indicator, uint8_t fu_header);
    int packFragment(u_char *buf, int length,
            void (*successFunc)(u_char *buf, int length),
            void (*failureFunc)(int errNo));
    int packFragmentWithFUA(u_char *buf, int length,
            void (*successFunc)(u_char *buf, int length),
            void (*failureFunc)(int errNo));
    void parseNal(u_char * nal, int length);
    unsigned int fps;
    unsigned char nalu_start_prefix1[4];
    unsigned char nalu_start_prefix2[3];
};
//endof class H264RtpPacket

#endif /* INCLUDE_H264_RTPPACKET_H_ */
