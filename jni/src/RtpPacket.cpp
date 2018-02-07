/*
 * RtpPacket.cpp
 *
 *  Created on: Jul 12, 2016
 *      Author: eyngzui
 */

#include "include/RtpPacket.h"
#include <string.h>
#include <ctime>
#include <stdlib.h>
#include "include/logger.h"

unsigned short RTPPacket::_sequence = 0;
unsigned int RTPPacket::_ts = 0;

RTPPacket::RTPPacket()
{
    payloadLength = 0;
    pRtpPkt = NULL;
    pPayload = NULL;
    pCsrc = NULL;
    rtpPool = NULL;
    memset(&rtpHdr, 0, sizeof(RTP_HDR_T));
}

RTPPacket::RTPPacket(PMEMORYPOOL pRTPPool)
{
    RTPPacket();
    rtpPool = pRTPPool;
    if (NULL != rtpPool)
    {
        rtpPool->dePool(rtpPool, &pRtpPkt);
    }
    
    srand (time(NULL));
    RTPPacket::_ts = abs(rand());
    
    srand (time(NULL));
    RTPPacket::_sequence = abs(rand());
    
    srand (time(NULL));
    _ssrc = rand();
}

RTPPacket::~RTPPacket()
{
    if (pRtpPkt != NULL || NULL != rtpPool)
    {
        rtpPool->enPool(rtpPool, &pRtpPkt);
    }
}

u_char * RTPPacket::getRTPPacket()
{
    return pRtpPkt;
}

u_char * RTPPacket::doPackage()
{
    int rtpPktLeng = sizeof(RTP_HDR_T) + payloadLength;
    memset(pRtpPkt, 0, rtpPktLeng);
    memcpy(pRtpPkt, &this->rtpHdr, sizeof(RTP_HDR_T));
    memcpy(pRtpPkt + sizeof(RTP_HDR_T), pPayload, payloadLength);
    return pRtpPkt;
}

void RTPPacket::setRTPHeader(RTP_HDR_T rtp_hdr)
{
    memcpy(&this->rtpHdr, &rtp_hdr, sizeof(RTP_HDR_T));
}

void RTPPacket::setRTPPayload(u_char * payload, int length)
{
    if (payload != NULL && length > 0)
    {
        this->pPayload = payload;
        payloadLength = length;
    }
    
}

bool RTPPacket::doParse(u_char * rtpPacket, int length)
{
    if (rtpPacket == NULL || length <= sizeof(RTP_HDR_T))
    {
        LOG_ERROR("rtp packet is invalid");
        return false;
    }
    
    RTP_HDR_T * pRtp_hdr = (RTP_HDR_T *) rtpPacket;
    
    if (pRtp_hdr->version != RTP_VERSION)
    {
        LOG_ERROR("rtp version is error : the version is %d \n",
                pRtp_hdr->version);
        return false;
    }
    memcpy(&rtpHdr, pRtp_hdr, sizeof(RTP_HDR_T));
    payloadLength = length - sizeof(RTP_HDR_T);
    
    pRtpPkt = rtpPacket;
    pPayload = pRtpPkt + sizeof(RTP_HDR_T);
    
    if (pRtp_hdr->cc != 0)
    {
        pPayload = pPayload + pRtp_hdr->cc * 4;
        pCsrc = pRtpPkt + sizeof(RTP_HDR_T);
        payloadLength -= pRtp_hdr->cc * 4;
    }
    
    return true;
}

RTP_HDR_T * RTPPacket::getRTPHeader(RTP_HDR_T* rtp_hdr)
{
    rtp_hdr = &rtpHdr;
    return &rtpHdr;
}

void * RTPPacket::getRTPPayload(u_char ** payload, int *length)
{
    *payload = this->pPayload;
    *length = payloadLength;
    return this->pPayload;
}
