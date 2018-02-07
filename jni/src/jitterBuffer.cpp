/*!
 * \file jitterBuffer.cpp
 *
 * \date Created on: Sep 14, 2016
 * \author: eyngzui
 */
#include "include/jitterBuffer.h"
#include "include/linkedList.h"
#include "include/logger.h"
#include <string.h>
#include <assert.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

bool createJb(struct jitterBuffer * pJb, int jbStartLength)
{
    if (jbStartLength < 0 || pJb == NULL)
    {
        return false;
    }
    
    pJb->jbState = false;
    pJb->lastPopedSeq = -1;
    pJb->jbStartLength = jbStartLength;
    pthread_mutex_init(&pJb->mutex, NULL);
    pJb->jbList = createLinkedList();
    if (NULL == pJb->jbList)
    {
        return false;
    }
    return true;
}
static bool insertJbNodeAt(struct jitterBuffer * pJb, Lisp_AVPacket * data,
        int index)
{
    
    if (pJb == NULL || data == NULL)
    {
        pthread_mutex_unlock(&pJb->mutex);
        return false;
    }
    JitterBufferNode * pJbNode = (JitterBufferNode*) malloc(
            sizeof(JitterBufferNode));
    if (pJbNode == NULL)
    {
        pthread_mutex_unlock(&pJb->mutex);
        return false;
    }
    
    memset(pJbNode, 0, sizeof(JitterBufferNode));
    pJbNode->ts = data->ts;
    //TODO: add isFrameReady logic
    pJbNode->isFrameReady = true;
    pJbNode->framePacketList = createLinkedList();
    pJbNode->framePacketList->linkedListInsertDataAtFirst(
            pJbNode->framePacketList,
            data);
    pJbNode->datalen += data->length;
    if (data->isNalBeginning)
    {
        (pJbNode->NalCount)++;
    }
    pJb->jbList->linkedListInsertDataAt(pJb->jbList, pJbNode, index);
    if (pJb->jbState == false)
    {
        if (pJb->jbList->linkedListGetSize(pJb->jbList) >= pJb->jbStartLength)
        {
            pJb->jbState = true;
        }
    }
    
    pthread_mutex_unlock(&pJb->mutex);
    return true;
}

bool push(struct jitterBuffer * pJb, Lisp_AVPacket * data)
{
    //TODO: change return to enum
    if (pJb == NULL || data == NULL)
    {
        return false;
    }
    int i = 0;
    unsigned long long dataTs = data->ts;
    unsigned long long lastPopedTs = pJb->lastPopedTs;
    unsigned long long tsLeft = 0;
    unsigned long long tsRight = 0;
    unsigned long long MaxTs = 0x01;
    MaxTs = MaxTs << 32;
    
    pthread_mutex_lock(&pJb->mutex);
    
    if (dataTs < lastPopedTs)
    {
        tsLeft = dataTs + MaxTs - lastPopedTs;
        tsRight = lastPopedTs - dataTs;
        if (tsLeft < tsRight)
        {
            dataTs = dataTs + MaxTs;
        }
        else
        {
            pthread_mutex_unlock(&pJb->mutex);
            return false;
        }
    }
    
    if (pJb->jbList->linkedListGetSize(pJb->jbList) == 0)
    {
        return insertJbNodeAt(pJb, data, 0);
    }
    else
    {
        int index = 0;
        LLNode * pInsert = pJb->jbList->first;
        while (pInsert != NULL)
        {
            JitterBufferNode *pJbNode = (JitterBufferNode *) pInsert->data;
            if (pJbNode == NULL)
            {
                LOG_ERROR("JiiterBufferNode is NULL");
                return false;
            }
            
            if (pJbNode->ts == data->ts)
            {
                LinkedList * pFramePacketList =
                        pJbNode->framePacketList;
                if (pFramePacketList == NULL)
                {
                    
                    pFramePacketList = createLinkedList();
                }
                
                if (pFramePacketList->linkedListGetSize(pFramePacketList) == 0)
                {
                    pFramePacketList->linkedListInsertDataAtFirst(
                            pFramePacketList, data);
                    pJbNode->datalen += data->length;
                    if (data->isNalBeginning)
                    {
                        (pJbNode->NalCount)++;
                    }
                }
                else
                {
                    LLNode *pFrameListNode =
                            pFramePacketList->first;
                    Lisp_AVPacket * packet = (Lisp_AVPacket *) pFrameListNode->data;
                    int count = 0;
                    unsigned int dataSeq = data->seq;
                    unsigned int packetSeq = packet->seq;
                    unsigned int seqLeft = 0;
                    unsigned int seqRight = 0;
                    unsigned int MaxSeq = 0x01;
                    MaxSeq = MaxSeq << 16;
                    i = 0;
                    int length = pFramePacketList->linkedListGetSize(
                            pFramePacketList);
                    while (i < length)
                    {
                        if (dataSeq < packetSeq)
                        {
                            seqLeft = dataSeq + MaxSeq - packetSeq;
                            seqRight = packetSeq - dataSeq;
                            if (seqLeft < seqRight)
                            {
                                dataSeq = dataSeq + MaxSeq;
                            }
                        }
                        if (packetSeq < dataSeq)
                        {
                            count++;
                            pFrameListNode = pFrameListNode->next;
                            if (pFrameListNode == NULL)
                            {
                                break;
                            }
                            packet = (Lisp_AVPacket *) pFrameListNode->data;
                            packetSeq = packet->seq;
                        }
                        i++;
                    }
                    pFramePacketList->linkedListInsertDataAt(pFramePacketList,
                            data, count);
                    pJbNode->datalen += data->length;
                    if (data->isNalBeginning)
                    {
                        (pJbNode->NalCount)++;
                    }
                }
                if (pJb->jbState == false)
                {
                    if (pJb->jbList->linkedListGetSize(pJb->jbList)
                            >= pJb->jbStartLength)
                    {
                        pJb->jbState = true;
                    }
                }
                pthread_mutex_unlock(&pJb->mutex);
                return true;
            }
            pInsert = pInsert->next;
            index++;
        }
        LLNode * pInsertNode = pJb->jbList->first;
        JitterBufferNode * jbNode = (JitterBufferNode *) pInsertNode->data;
        unsigned long long dataTs = data->ts;
        unsigned long long nodeTs = jbNode->ts;
        int count = 0;
        i = 0;
        int length = pJb->jbList->linkedListGetSize(pJb->jbList);
        while (i < length)
        {
            if (dataTs < nodeTs)
            {
                tsLeft = dataTs + MaxTs - nodeTs;
                tsRight = nodeTs - dataTs;
                if (tsLeft < tsRight)
                {
                    dataTs = dataTs + MaxTs;
                }
            }
            
            if (nodeTs < dataTs)
            {
                count++;
                pInsertNode = pInsertNode->next;
                if (pInsertNode == NULL)
                {
                    break;
                }
                jbNode = (JitterBufferNode *) pInsertNode->data;
                nodeTs = jbNode->ts;
            }
            i++;
        }
        return insertJbNodeAt(pJb, data, count);
    }
}
JitterBufferNode * pop(struct jitterBuffer * pJb)
{
    if (NULL == pJb || NULL == pJb->jbList)
    {
        goto exit;
    }
    pthread_mutex_lock(&pJb->mutex);
    if (pJb->jbState)
    {
        JitterBufferNode * jbNode =
                (JitterBufferNode *) pJb->jbList->linkedListGetDataAtFirst(
                        pJb->jbList);
        
        if (NULL == jbNode)
        {
            goto exit;
        }
        pJb->jbList->linkedListDeleteDataAtFirst(
                pJb->jbList);
        
        pJb->lastPopedTs = jbNode->ts;
        pJb->lastPopedSeq =
                ((Lisp_AVPacket *) jbNode->framePacketList->linkedListGetDataAtLast(
                        jbNode->framePacketList))->seq;
        pthread_mutex_unlock(&pJb->mutex);
        return jbNode;
    }
    
    exit:
    pthread_mutex_unlock(&pJb->mutex);
    return NULL;
}

JitterBuffer_ptr generateJb(int jbStartLength)
{
    if (jbStartLength < 0)
    {
        LOG_ERROR(
                "please provide a valid value of jbStartLength, jbStartLength = %d\n",
                jbStartLength);
    }
    JitterBuffer * pJB = (JitterBuffer *) malloc(sizeof(JitterBuffer));
    memset(pJB, 0, sizeof(JitterBuffer));
    pJB->createJb = createJb;
    pJB->push = push;
    pJB->pop = pop;
    if (pJB->createJb(pJB, jbStartLength))
    {
        return pJB;
    }
    
    return NULL;
}

void destroyJb(JitterBuffer_ptr pJb)
{
    if (NULL == pJb)
    {
        return;
    }
    
    if (NULL != pJb->jbList)
    {
        while (pJb->jbList->first)
        {
            jitterBufferNode * jBNode = (jitterBufferNode *) pJb->jbList->first;
            if (jBNode)
            {
                if (jBNode->framePacketList)
                {
                    freeLinkedList(jBNode->framePacketList);
                }
                pJb->jbList->first = pJb->jbList->first->next;
                free(jBNode);
                continue;
            }
            pJb->jbList->first = pJb->jbList->first->next;
        }
        free(pJb->jbList);
        pJb->jbList = NULL;
    }
    pthread_mutex_destroy(&pJb->mutex);
    free(pJb);
    pJb = NULL;
}

bool reset(struct jitterBuffer * pJb)
{
    pthread_mutex_lock(&pJb->mutex);
    if (NULL == pJb)
    {
        goto success;
    }
    
    if (NULL != pJb->jbList)
    {
        if ((NULL != pJb->jbList->last)
                && (NULL
                        != ((jitterBufferNode *) pJb->jbList->last)->framePacketList))
        {
            pJb->lastPopedTs = ((jitterBufferNode *) pJb->jbList->last)->ts;
            LinkedList * list =
                    ((jitterBufferNode *) pJb->jbList->last)->framePacketList;
        }
        
        while (pJb->jbList->first)
        {
            jitterBufferNode * jBNode = (jitterBufferNode *) pJb->jbList->first;
            if (jBNode)
            {
                if (jBNode->framePacketList)
                {
                    freeLinkedList(jBNode->framePacketList);
                }
                pJb->jbList->first = pJb->jbList->first->next;
                free(jBNode);
                continue;
            }
            pJb->jbList->first = pJb->jbList->first->next;
        }
    }
    
    success:
    pthread_mutex_unlock(&pJb->mutex);
    return true;
    failure:
    pthread_mutex_unlock(&pJb->mutex);
    return false;
}
