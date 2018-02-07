/*!
 * \file jitterBuffer.h
 *
 * \date Created on: Aug 8, 2016
 * \author: eyngzui
 */

#ifndef INCLUDE_JITTERBUFFER_H_
#define INCLUDE_JITTERBUFFER_H_
#include "linkedList.h"
#include "AVPacket.h"
#include <pthread.h>

typedef struct jitterBufferNode
{
    unsigned int ts;
    LinkedList * framePacketList;
    bool isFrameReady;
    int datalen;
    int NalCount;
} JitterBufferNode, *pJitterBufferNode;

typedef struct jitterBuffer
{
    LinkedList * jbList;
    int jbStartLength;
    unsigned int lastPopedTs;
    int lastPopedSeq;
    bool jbState;
    bool (*createJb)(struct jitterBuffer * pJb, int jbStartLength);
    bool (*push)(struct jitterBuffer * pJb, Lisp_AVPacket * data);
    JitterBufferNode * (*pop)(struct jitterBuffer * pJb);
    bool (*reset)(struct jitterBuffer * pJb);
    pthread_mutex_t mutex;
} JitterBuffer, *JitterBuffer_ptr;

JitterBuffer_ptr generateJb(int jbStartLength);

void destroyJb(JitterBuffer_ptr jb);

#endif /* INCLUDE_JITTERBUFFER_H_ */

