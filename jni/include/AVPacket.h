/*
 * AVStream.h
 *
 *  Created on: 2016年8月8日
 *      Author: ELEIJYA
 */

#ifndef INCLUDE_AVPACKET_H_
#define INCLUDE_AVPACKET_H_

typedef struct _av_packet
{
    unsigned short seq;
    unsigned int ts;
    unsigned int ssrc;
    unsigned char *NalData;
    unsigned char *recvBuf;
    int length;
    bool isNalBeginning;
} Lisp_AVPacket;

Lisp_AVPacket * createAVPacket();

void freeAVPacket(Lisp_AVPacket ** ppPacket);

#endif /* INCLUDE_AVPACKET_H_ */
