/*!
 * \file linkedList.cpp
 *
 * \date Created on: Aug 8, 2016
 * \author: eyngzui
 */

#include "include/linkedList.h"
#include "include/logger.h"
#include <stdlib.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

pLinkedList createSearchLinkedList(int (*equal)(void *a, void *b))
{
    pLinkedList pList = createLinkedList();
    pList->equal = equal;
    return pList;
}

void freeLinkedList(pLinkedList pList)
{
    pthread_mutex_lock(&pList->mutex);
    LLNode *p;
    while (pList->first)
    {
        p = pList->first->next;
        free(pList->first);
        pList->first = p;
    }
    
    pthread_mutex_unlock(&pList->mutex);
    pthread_mutex_destroy(&pList->mutex);
    free(pList);
    
}

void linkedListInsertDataAtLast(pLinkedList const pList, void * const data)
{
    LLNode * node = (LLNode *) malloc(sizeof(LLNode));
    if (NULL == node)
    {
        return;
    }
    node->data = data;
    node->next = NULL;
    pthread_mutex_lock(&pList->mutex);
    if (pList->count)
    {
        pList->last->next = node;
        pList->last = node;
    }
    else
    {
        pList->first = node;
        pList->last = node;
    }
    (pList->count)++;
    pthread_mutex_unlock(&pList->mutex);
}

void linkedListInsertDataAtFirst(pLinkedList const pList, void * const data)
{
    LLNode * node = (LLNode *) malloc(sizeof(LLNode));
    if (NULL == node)
    {
        return;
    }
    node->data = data;
    node->next = NULL;
    
    pthread_mutex_lock(&pList->mutex);
    if (pList->count > 0)
    {
        node->next = pList->first;
        pList->first = node;
    }
    else
    {
        pList->first = node;
        pList->last = node;
    }
    (pList->count)++;
    pthread_mutex_unlock(&pList->mutex);
}

void linkedListInsertDataAt(pLinkedList const pList, void * const data,
        int index)
{
    if (0 == index)
    {
        linkedListInsertDataAtFirst(pList, data);
        return;
    }
    if (index == pList->count)
    {
        linkedListInsertDataAtLast(pList, data);
        return;
    }
    
    LLNode * node = (LLNode *) malloc(sizeof(LLNode));
    if (NULL == node)
    {
        return;
    }
    node->data = data;
    node->next = NULL;
    pthread_mutex_lock(&pList->mutex);
    LLNode * p = pList->first;
    for (int i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    
    node->next = p->next;
    p->next = node;
    
    (pList->count)++;
    pthread_mutex_unlock(&pList->mutex);
}

void * linkedListDeleteDataAtFirst(pLinkedList pList)
{
    pthread_mutex_lock(&pList->mutex);
    LLNode * p = pList->first;
    if (p == NULL)
    {
        pthread_mutex_unlock(&pList->mutex);
        return NULL;
    }
    pList->first = p->next;
    void *ret = p->data;
    free(p);
    (pList->count)--;
    if (0 == pList->count)
    {
        pList->first = pList->last = NULL;
    }
    pthread_mutex_unlock(&pList->mutex);
    return ret;
}

void * linkedListDeleteDataAtLast(pLinkedList pList)
{
    if (1 == pList->count)
    {
        return linkedListDeleteDataAtFirst(pList);
    }
    pthread_mutex_lock(&pList->mutex);
    LLNode *p = pList->first;
    while (p->next != pList->last)
    {
        p = p->next;
    }
    
    if (p == NULL)
    {
        pthread_mutex_unlock(&pList->mutex);
        return NULL;
    }
    void * ret = pList->last->data;
    free(pList->last);
    p->next = NULL;
    pList->last = p;
    (pList->count)--;
    pthread_mutex_unlock(&pList->mutex);
    return ret;
}

void * linkedListDeleteDataAt(pLinkedList pList, int index)
{
    if (0 == index)
    {
        return linkedListDeleteDataAtFirst(pList);
    }
    if (index == pList->count - 1)
    {
        return linkedListDeleteDataAtLast(pList);
    }
    
    pthread_mutex_lock(&pList->mutex);
    LLNode *p = pList->first;
    for (int i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    if (p == NULL)
    {
        pthread_mutex_unlock(&pList->mutex);
        return NULL;
    }
    LLNode *temp = p->next;
    p->next = p->next->next;
    void * ret = temp->data;
    free(temp);
    (pList->count)--;
    pthread_mutex_unlock(&pList->mutex);
    return ret;
}

int LListIteratorHasNext(const LListIterator * const iterator)
{
    return iterator->count < iterator->allsize;
}

void *LListIteratorNext(LListIterator * const iterator)
{
    void *data = iterator->p->data;
    iterator->p = iterator->p->next;
    (iterator->count)++;
    return data;
}

int linkedListDeleteData(pLinkedList pList, void *data)
{
    
    LListIterator * it = createLListInterator(pList);
    int isExist = 0;
    while (LListIteratorHasNext(it))
    {
        void * inData = LListIteratorNext(it);
        if (inData == data
                || (pList->equal != NULL && (*(pList->equal))(inData, data)))
        {
            isExist = 1;
            break;
        }
    }
    if (isExist)
    {
        linkedListDeleteDataAt(pList, it->count - 1);
    }
    return isExist;
}

int linkedListGetSize(pLinkedList pList)
{
    pthread_mutex_lock(&pList->mutex);
    int size = pList->count;
    pthread_mutex_unlock(&pList->mutex);
    return size;
}

void linkedListOutput(pLinkedList pList, void (*print)(const void * const))
{
    LLNode *p = pList->first;
    while (p)
    {
        (*print)(p->data);
        p = p->next;
    }
}

void *linkedListGetDataAtLast(pLinkedList pList)
{
    pthread_mutex_lock(&pList->mutex);
    void * data = pList->last->data;
    pthread_mutex_unlock(&pList->mutex);
    return data;
}

void *linkedListGetDataAt(pLinkedList pList, int index)
{
    void * data = NULL;
    if (index == pList->count - 1)
    {
        return linkedListGetDataAtLast(pList);
    }
    pthread_mutex_lock(&pList->mutex);
    LLNode *p = pList->first;
    for (int i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    if (p == NULL)
    {
        pthread_mutex_unlock(&pList->mutex);
        return NULL;
    }
    data = p->data;
    pthread_mutex_unlock(&pList->mutex);
    return data;
}

void *linkedListGetDataAtFirst(pLinkedList pList)
{
    void * data = NULL;
    pthread_mutex_lock(&pList->mutex);
    if (pList->first == NULL)
    {
        pthread_mutex_unlock(&pList->mutex);
        return NULL;
    }
    data = pList->first->data;
    pthread_mutex_unlock(&pList->mutex);
    return data;
}

int linkedListFindDataIndex(pLinkedList pList, void *data)
{
    pthread_mutex_lock(&pList->mutex);
    LLNode *p = pList->first;
    int ret = 0;
    if (pList->equal)
    {
        while (p)
        {
            if (p->data == data || (*(pList->equal))(p->data, data))
            {
                pthread_mutex_unlock(&pList->mutex);
                return ret;
            }
            ret++;
            p = p->next;
        }
    }
    else
    {
        while (p)
        {
            if (p->data == data)
            {
                pthread_mutex_unlock(&pList->mutex);
                return ret;
            }
            ret++;
            p = p->next;
        }
    }
    pthread_mutex_unlock(&pList->mutex);
    return -1;
}

LListIterator * createLListInterator(const pLinkedList pList)
{
    LListIterator * iter = (LListIterator *) malloc(sizeof(LListIterator));
    iter->p = pList->first;
    iter->allsize = pList->count;
    iter->count = 0;
    iter->LListIteratorHasNext = LListIteratorHasNext;
    iter->LListIteratorNext = LListIteratorNext;
    return iter;
}

void freeLListInterator(LListIterator * iterator)
{
    free(iterator);
}

pLinkedList createLinkedList()
{
    pLinkedList pList = (pLinkedList) malloc(sizeof(LinkedList));
    if (NULL == pList)
    {
        return NULL;
    }
    memset(pList, 0, sizeof(LinkedList));
    pList->count = 0;
    pList->first = NULL;
    pList->last = NULL;
    pList->equal = NULL;
    pthread_mutex_init(&pList->mutex, NULL);
    pList->linkedListInsertDataAtLast = linkedListInsertDataAtLast;
    pList->linkedListInsertDataAtFirst = linkedListInsertDataAtFirst;
    pList->linkedListInsertDataAt = linkedListInsertDataAt;
    pList->linkedListDeleteDataAtFirst = linkedListDeleteDataAtFirst;
    pList->linkedListDeleteDataAtLast = linkedListDeleteDataAtLast;
    pList->linkedListDeleteDataAt = linkedListDeleteDataAt;
    pList->linkedListDeleteData = linkedListDeleteData;
    pList->linkedListGetSize = linkedListGetSize;
    pList->linkedListOutput = linkedListOutput;
    pList->linkedListGetDataAt = linkedListGetDataAt;
    pList->linkedListGetDataAtFirst = linkedListGetDataAtFirst;
    pList->linkedListGetDataAtLast = linkedListGetDataAtLast;
    pList->linkedListFindDataIndex = linkedListFindDataIndex;
    
    return pList;
}
