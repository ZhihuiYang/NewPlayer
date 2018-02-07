/*!
 * \file h264_RtpPacket.cpp
 *
 * \date Created on: Jul 25, 2016
 * \author: eyngzui
 */

#include "include/h264_RtpPacket.h"
#include "include/logger.h"
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

H264RtpPacket::H264RtpPacket(PMEMORYPOOL rtpPool, unsigned int fpsVal)
        : RTPPacket(rtpPool)
{
    this->fps = fpsVal;
    this->fps = 30;
    unsigned char prefix[4] =
            { 0x00, 0x00, 0x00, 0x01 };
    memcpy(this->nalu_start_prefix1,
            prefix, 4);
    memcpy(this->nalu_start_prefix2,
            prefix + 1, 3);
}

H264RtpPacket::H264RtpPacket()
{
}

H264RtpPacket::~H264RtpPacket()
{
    
}

void H264RtpPacket::packFragmentHeader()
{
    RTP_HDR_T rtp_hdr;
    
    ++_sequence;
    
    rtp_hdr.version = RTP_VERSION;
    rtp_hdr.p = 0;
    rtp_hdr.x = 0;
    rtp_hdr.cc = 0;
    rtp_hdr.m = 0;
    rtp_hdr.pt = 96;
    rtp_hdr.seq = htons(_sequence);
    rtp_hdr.ts = htonl(_ts);
    rtp_hdr.ssrc = htonl(_ssrc);
    RTPPacket::setRTPHeader(rtp_hdr);
}

int H264RtpPacket::packFragmentPayload(u_char *buf, int length)
{
    RTPPacket::setRTPPayload(buf, length);
    RTPPacket::doPackage();
    return 0;
}

int H264RtpPacket::packFragmentPayloadWithFUA(u_char *buf, int length,
        uint8_t fu_indicator, uint8_t fu_header)
{
    u_char payload[MAX_RTP_PAYLOAD_LENGTH];
    memset(payload, 0, length);
    payload[0] = fu_indicator;
    payload[1] = fu_header;
    
    memcpy(payload + 2, buf, length);
    RTPPacket::setRTPPayload(payload, length + 2);        //2 is FU_A head
    RTPPacket::doPackage();
    return 0;
}

int H264RtpPacket::packFragment(u_char *buf, int length,
        void (*successFunc)(u_char *buf, int length),
        void (*failureFunc)(int errNo))
{
    int result_code = 0;
    if (NULL == pRtpPkt)
    {
        // prepare new RTP packet
        if (!rtpPool->dePool(rtpPool, &pRtpPkt))
        {
            result_code = -1;
            (*failureFunc)(result_code);
            return result_code;
        }
    }
    packFragmentHeader();
    result_code = packFragmentPayload(buf, length);
    if (result_code < 0)
    {
        (*failureFunc)(result_code);
        rtpPool->enPool(rtpPool, &pRtpPkt);
        pRtpPkt = NULL;
    }
    else
    {
        (*successFunc)(pRtpPkt, payloadLength + sizeof(RTP_HDR_T));
        rtpPool->enPool(rtpPool, &pRtpPkt);
        pRtpPkt = NULL;
    }
    
    return result_code;
}

int H264RtpPacket::packFragmentWithFUA(u_char *buf, int length,
        void (*successFunc)(u_char *buf, int length),
        void (*failureFunc)(int errNo))
{
    int nalu_length = length;
    int header_size = 2;
    int payload_size = MAX_RTP_PAYLOAD_LENGTH - header_size;        //1398
    int result_code = 0;
    int offset = 0;
    
    // process NALU header
    u_char nalu_header = buf[0];
    uint8_t nri = nalu_header & 0x60;
    uint8_t fu_indicator = nri | 0x1C;
    uint8_t nalu_type = nalu_header & 0x1F;
    uint8_t fu_header = nalu_type | 0x80;
    
    // skip the NALU header(1 byte)
    buf += 1;
    nalu_length -= 1;
    offset += 1;
    
    if (NULL == pRtpPkt)
    {
        // prepare new RTP packet
        if (!rtpPool->dePool(rtpPool, &pRtpPkt))
        {
            result_code = -1;
            (*failureFunc)(result_code);
            return result_code;
        }
    }
    
    while (nalu_length > 0)
    {
//        MY_LOG_DEBUG("packFragmentWithFUA: nalu_length = %d", nalu_length);
        packFragmentHeader();
        int result_code = packFragmentPayloadWithFUA(buf, payload_size,
                fu_indicator, fu_header);
        if (result_code < 0)
        {
            (*failureFunc)(result_code);
            rtpPool->enPool(rtpPool, &pRtpPkt);
            pRtpPkt = NULL;
            return result_code;
        }
        else
        {
//            MY_LOG_DEBUG(
//                    "packFragmentWithFUA: payloadLength = %d, RTP_HDR_T size = %d",
//                    payloadLength, sizeof(RTP_HDR_T));
            (*successFunc)(pRtpPkt, payloadLength + sizeof(RTP_HDR_T));
            rtpPool->enPool(rtpPool, &pRtpPkt);
            pRtpPkt = NULL;
        }
        
        buf += payload_size;
        offset += payload_size;
        nalu_length -= payload_size;
        
        // prepare new RTP packet
        if (!rtpPool->dePool(rtpPool, &pRtpPkt))
        {
            result_code = -1;
            (*failureFunc)(result_code);
            return result_code;
        }
        
        if (offset + payload_size < length)
        {
            fu_header = nalu_type | 0x00;
        }
        else
        {
            // it is the last RTP packet for the same video frame
            fu_header = nalu_type | 0x40;
            
            // remain byte to be sent
            payload_size = length - offset;
        }
    }
    return 0;
}

int H264RtpPacket::doPackage(u_char *buf, int length, long long timeStamp,
        void (*successFunc)(u_char *buf, int length),
        void (*failureFunc)(int errNo))
{
    int result_code = 0;
    assert(buf != NULL && successFunc != NULL && failureFunc != NULL);
    
    // Verify the NALU
    if (0 == memcmp(nalu_start_prefix1, buf, 4))
    {
        // skip the start prefix
        buf += 4;
        length -= 4;
        //TODO: Use PTS instead if it is set; otherwise apply the following line
        if (timeStamp == 0)
        {
            _ts += 90000 / fps;        // 90000 (Video Clock Rate) / 30 (30FPS video) TODO: Use the real time from application layer
        }
        else
        {
            _ts = timeStamp;
        }
    }
    else if (0 == memcmp(nalu_start_prefix2, buf, 3))
    {
        // skip the start prefix
        buf += 3;
        length -= 3;
    }
    else
    {
        result_code = -1;
        return result_code;
    }
    
    if (length <= MAX_RTP_PAYLOAD_LENGTH)
    {
        result_code = packFragment(buf, length, successFunc, failureFunc);
    }
    else
    {
        result_code = packFragmentWithFUA(buf, length, successFunc,
                failureFunc);
    }
    
    return result_code;
}

int H264RtpPacket::doParse(u_char *buf, int length,
        void (*successFunc)(Lisp_AVPacket NalData),
        void (*failureFunc)(int errNo, void * buf))
{
    if (true == RTPPacket::doParse(buf, length))
    {
        RTP_HDR_T * pRtp_hdr = NULL;
        pRtp_hdr = getRTPHeader(pRtp_hdr);
        u_char * pRtp_payload = NULL;
        int pld_length = 0;
        getRTPPayload(&pRtp_payload, &pld_length);
        
        u_char type = pRtp_payload[0] & 0x1f;
        u_char * nal = pRtp_payload;
        Lisp_AVPacket packet;
        packet.isNalBeginning = true;
        
        if (type == 28)
        {
            LOG_DEBUG("This is a FU_A package");
            u_char nri = pRtp_payload[0] & 0x60;
            u_char startbit = pRtp_payload[1] & 0x80;
            u_char nal_type = pRtp_payload[1] & 0x1F;
            if (startbit)
            {
                u_char nal_header = nri | nal_type;
                pRtp_payload[1] = nal_header;
                //(pRtp_payload[1] & 0x00) | nal_header;
                nal += 1;
                pld_length -= 1;
                packet.isNalBeginning = true;
            }
            else
            {
                nal += 2;
                pld_length -= 2;
                packet.isNalBeginning = false;
            }
        }
        
        packet.seq = ntohs(pRtp_hdr->seq);
        packet.ts = ntohl(pRtp_hdr->ts);
        packet.ssrc = ntohl(pRtp_hdr->ssrc);
        packet.NalData = nal;
        packet.recvBuf = buf;
        packet.length = pld_length;
        (*successFunc)(packet);
    }
    else
    {
        (*failureFunc)(-1, buf);
    }
    return 0;
}

void H264RtpPacket::parseNal(u_char * nal, int length)
{
    
}

