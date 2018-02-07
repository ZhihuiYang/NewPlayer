/*
 * queue.c
 *
 *  Created on: Jul 14, 2016
 *      Author: eyngzui
 */

#include "include/logger.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "../include/memoryPool.h"

#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

void createPool(PMEMORYPOOL pMP, int maxQueueSize, int dataLength)
{
    pMP->pBase = (DataType **) malloc(sizeof(DataType *) * maxQueueSize + 1);
    assert(NULL != pMP->pBase);
    
    memset(pMP->pBase, 0, sizeof(DataType *) * maxQueueSize + 1);
    for (int i = 0; i < maxQueueSize; i++)
    {
        pMP->pBase[i] = (DataType *) malloc(sizeof(DataType) * dataLength);
        assert(NULL != pMP->pBase[i]);
    }
    
    pMP->front = 0;
    pMP->rear = maxQueueSize;
    pMP->maxPoolSize = maxQueueSize + 1;
}

void traversePool(PMEMORYPOOL pMP)
{
    int i = pMP->front;
    LOG_INFO("the elements  in the queue are:\n");
    while (i % pMP->maxPoolSize != pMP->rear)
    {
        LOG_INFO("%d : %p \n", i, pMP->pBase[i]);
        i++;
    }
    
    LOG_INFO("\n");
}

bool isFullPool(PMEMORYPOOL pMP)
{
    if (pMP->front == (pMP->rear + 1) % pMP->maxPoolSize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isEmptyPool(PMEMORYPOOL pMP)
{
    if (pMP->front == pMP->rear)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool enPool(PMEMORYPOOL pMP, DataType **val)
{
    if (isFullPool(pMP))
    {
        return false;
    }
    else
    {
        pMP->pBase[pMP->rear] = *val;
        pMP->rear = (pMP->rear + 1) % pMP->maxPoolSize;
        return true;
    }
}

bool dePool(PMEMORYPOOL pMP, DataType **val)
{
    if (isEmptyPool(pMP))
    {
        return false;
    }
    else
    {
        *val = pMP->pBase[pMP->front];
        pMP->pBase[pMP->front] = NULL;
        pMP->front = (pMP->front + 1) % pMP->maxPoolSize;
        return true;
    }
}

void destoryPool(PMEMORYPOOL pMP)
{
    if (pMP->pBase == NULL)
    {
        return;
    }
    
    for (int i = 0; i < pMP->maxPoolSize; i++)
    {
        if (pMP->pBase[i] != NULL)
        {
            free(pMP->pBase[i]);
        }
    }
    free(pMP->pBase);
}

PMEMORYPOOL generateRTPMemoryPool(int poolSize, int dataLength)
{
    PMEMORYPOOL pRTPMP = (PMEMORYPOOL) malloc(sizeof(MEMORYPOOL));
    memset(pRTPMP, 0, sizeof(MEMORYPOOL));
    pRTPMP->createPool = createPool;
    pRTPMP->isFullPool = isFullPool;
    pRTPMP->isEmptyPool = isEmptyPool;
    pRTPMP->enPool = enPool;
    pRTPMP->dePool = dePool;
    pRTPMP->destoryPool = destoryPool;
    
    pRTPMP->createPool(pRTPMP, poolSize, dataLength);
    return pRTPMP;
}

void destoryMemoryPool(PMEMORYPOOL pMP)
{
    if (pMP == NULL)
    {
        return;
    }
    
    if (pMP->pBase == NULL)
    {
        return;
    }
    
    if (pMP->maxPoolSize <= 0)
    {
        return;
    }
    
    for (int i = 0; i < pMP->maxPoolSize; i++)
    {
        if (pMP->pBase[i] != NULL)
        {
            free(pMP->pBase[i]);
        }
    }
    
    free(pMP->pBase);
    free(pMP);
    pMP = NULL;
}

void destoryRTPMemoryPool(PMEMORYPOOL Q)
{
    destoryMemoryPool(Q);
}

PMEMORYPOOL generateYUVMemoryPool(int queueSize, int width, int height,
        int type)
{
    PMEMORYPOOL pYUVMP = (PMEMORYPOOL) malloc(sizeof(MEMORYPOOL));
    pYUVMP->createPool = createPool;
    pYUVMP->isFullPool = isFullPool;
    pYUVMP->isEmptyPool = isEmptyPool;
    pYUVMP->enPool = enPool;
    pYUVMP->dePool = dePool;
    pYUVMP->destoryPool = destoryPool;
    
    int dataLength = 0;
    switch (type)
    {
        case YUV444:
            dataLength = width * height * 3;
            break;
        case YUV422:
            dataLength = width * height * 2;
            break;
        case YUV420:
            dataLength = width * height * 3 / 2;
            break;
        default:
            dataLength = width * height * 3 / 2;
            break;
    }
    pYUVMP->createPool(pYUVMP, queueSize, dataLength);
    return pYUVMP;
}

void destoryYUVMemoryPool(PMEMORYPOOL Q)
{
    destoryMemoryPool(Q);
}

