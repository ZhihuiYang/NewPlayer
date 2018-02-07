/*!
 * \file CircularQueue.cpp
 *
 * \date Created on: Jul 19, 2016
 * \author: eyngzui
 */

#include "include/CircularQueue.h"
#include "include/logger.h"
#include <stddef.h>
#include <assert.h>

static void *createQueue(PCIRQUEUE pCQ, int maxQueueSize)
{
    assert(NULL != pCQ);
    pCQ->pBase = (void **) malloc(maxQueueSize);
    assert(NULL != pCQ->pBase);

    memset(pCQ->pBase, 0, maxQueueSize);
    pCQ->maxQueueSize = maxQueueSize;
    pCQ->front = pCQ->rear = 0;
    return pCQ;
}

static void traverseQueue(PCIRQUEUE pCQ)
{
    //todo
}

static bool isFullQueue(PCIRQUEUE pCQ)
{
    if (pCQ->front == (pCQ->rear + 1) % pCQ->maxQueueSize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool isEmptyQueue(PCIRQUEUE pCQ)
{
    if (pCQ->front == pCQ->rear)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool enQueue(PCIRQUEUE pCQ, void **val)
{
    if (pCQ->isFullQueue(pCQ))
    {
        return false;
    }

    pCQ->pBase[pCQ->rear] = *val;
    pCQ->rear = (pCQ->rear + 1) % pCQ->maxQueueSize;
    return true;
}

static bool deQueue(PCIRQUEUE pCQ, void **val)
{
    if (pCQ->isEmptyQueue(pCQ))
    {
        return false;
    }

    *val = pCQ->pBase[pCQ->front];
    pCQ->front = (pCQ->front + 1) % pCQ->maxQueueSize;
    return true;
}

static void destoryQueue(PCIRQUEUE pCQ)
{
    //todo
}

PCIRQUEUE generateCircularQueue(PCIRQUEUE pCQ, int maxQueueSize,
        PMEMORYPOOL pMemPool)
{
    assert(NULL != pMemPool);

    pCQ = (PCIRQUEUE) malloc(sizeof(CIRQUEUE));
    memset(pCQ, 0, sizeof(CIRQUEUE));

    pCQ->createQueue = createQueue;
    pCQ->traverseQueue = traverseQueue;
    pCQ->isFullQueue = isFullQueue;
    pCQ->isEmptyQueue = isEmptyQueue;
    pCQ->enQueue = enQueue;
    pCQ->deQueue = deQueue;
    pCQ->destoryQueue = destoryQueue;

    pCQ->createQueue(pCQ, maxQueueSize);
    pCQ->pMemPool = pMemPool;

    return pCQ;
}

void destoryCircularQueue(PCIRQUEUE pCQ)
{
    if (pCQ == NULL)
    {
        return;
    }

    if (pCQ->pBase == NULL)
    {
        return;
    }

    if (pCQ->pMemPool == NULL)
    {
        LOG_ERROR("mempool is null");
        return;
    }

    int size = (pCQ->rear + pCQ->maxQueueSize - pCQ->front) % pCQ->maxQueueSize;
    for (int i = 0; i < size; i++)
    {
        if (pCQ->pBase[i] != NULL)
        {
            //pCQ->pMemPool->enPool(pCQ->pBase[i]);
        }
    }

    free(pCQ->pBase);
    free(pCQ);
}

