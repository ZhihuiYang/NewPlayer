/*!
 *  \file CircularQueue.h
 *
 *  \date Created on: Jul 19, 2016
 *  \author: eyngzui
 */

#ifndef INCLUDE_CIRCULARQUEUE_H_
#define INCLUDE_CIRCULARQUEUE_H_

typedef struct _CIRCULARQUEUE
{
    void **pBase; /**< the circular queue base address*/
    int front; /**< front index of circular queue */
    int rear; /**< rear index of circular queue */
    int maxQueueSize; /**< maximum size of circular queue */
    PMEMORYPOOL pMemPool; /**< point to memory pool*/
    void * (*createQueue)(PCIRQUEUE pCQ, int maxQueueSize); /**< funcion point to ceration functionality*/
    void (*traverseQueue)(PCIRQUEUE pCQ); /**< funcion point to traversion functionality*/
    bool (*isFullQueue)(PCIRQUEUE pCQ); /**< funcion point to full circular queue detection */
    bool (*isEmptyQueue)(PCIRQUEUE pCQ); /**< funcion point to empty circular queue detection */
    bool (*enQueue)(PCIRQUEUE pCQ, void **val); /**< funcion point to enter element to circular queue functionality  */
    bool (*deQueue)(PCIRQUEUE pCQ, void **val); /**< funcion point to delete element from circular queue functionality */
    void (*destoryQueue)(PCIRQUEUE pCQ); /**< funcion point to destory a circular queue */
} CIRQUEUE, /**< typedef of  struct _CIRCULARQUEUE */
*PCIRQUEUE; /**< typedef of  struct _CIRCULARQUEUE point */

PCIRQUEUE generateCircularQueue(PCIRQUEUE pCQ, int maxQueueSize,
        PMEMORYPOOL pMemPool);

void destoryCircularQueue(PCIRQUEUE pCQ);

#endif /* INCLUDE_CIRCULARQUEUE_H_ */
