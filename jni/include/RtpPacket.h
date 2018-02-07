/*
 * RtpPacket.h
 *
 *  Created on: Jul 12, 2016
 *      Author: eyngzui
 */

#ifndef INCLUDE_RTPPACKET_H_
#define INCLUDE_RTPPACKET_H_
#include "rtpheader.h"
#include "memoryPool.h"

#define RTP_VERSION 2
#define MAX_RTP_PAYLOAD_LENGTH 1400

class RTPPacket
{
protected:
    u_char * pRtpPkt;
    u_char * pPayload;
    int payloadLength;
    RTP_HDR_T rtpHdr;
    u_char * pCsrc;
    PMEMORYPOOL rtpPool;
    static unsigned short _sequence;
    static unsigned int _ts;
    unsigned int _ssrc;

public:
    RTPPacket();
    RTPPacket(PMEMORYPOOL rtpPool);
    void setRTPHeader(RTP_HDR_T rtp_hdr);
    void setRTPPayload(u_char * payload, int length);
    u_char * getRTPPacket();
    u_char * doPackage();
    bool doParse(u_char * rtpPacket, int length);
    RTP_HDR_T * getRTPHeader(RTP_HDR_T* rtp_hdr);
    void * getRTPPayload(u_char **payload, int *length);
    ~RTPPacket();
};

#endif /* INCLUDE_RTPPACKET_H_ */
