/*!
 * \file linkedList.h
 *
 * \date Created on: Aug 8, 2016
 * \author: eyngzui
 */

#ifndef INCLUDE_LINKEDLIST_H_
#define INCLUDE_LINKEDLIST_H_
#include <pthread.h>
typedef struct LLNode
{
    void * data;
    struct LLNode *next;
} LLNode;

typedef struct LinkedList
{
    LLNode * first;
    LLNode * last;
    pthread_mutex_t mutex;
    int count;
    int (*equal)(void *a, void *b);
    void (*linkedListInsertDataAtLast)(struct LinkedList * const pList,
            void * const data);

    void (*linkedListInsertDataAtFirst)(struct LinkedList * const pList,
            void * const data);

    void (*linkedListInsertDataAt)(struct LinkedList * pList, void * const data,
            int index);

    void * (*linkedListDeleteDataAtFirst)(struct LinkedList * pList);

    void * (*linkedListDeleteDataAtLast)(struct LinkedList * pList);

    void * (*linkedListDeleteDataAt)(struct LinkedList * pList, int index);

    int (*linkedListDeleteData)(struct LinkedList * pList, void *data);

    int (*linkedListGetSize)(struct LinkedList * pList);

    void (*linkedListOutput)(struct LinkedList * pList,
            void (*print)(const void * const));

    void * (*linkedListGetDataAt)(struct LinkedList * pList, int index);

    void * (*linkedListGetDataAtFirst)(struct LinkedList * pList);

    void * (*linkedListGetDataAtLast)(struct LinkedList * pList);

    int (*linkedListFindDataIndex)(struct LinkedList * pList, void *data);
} LinkedList, *pLinkedList;

typedef struct LinkedListIterator
{
    LLNode *p;
    int count;
    int allsize;

    int (*LListIteratorHasNext)(
            const struct LinkedListIterator * const iterator);

    void * (*LListIteratorNext)(struct LinkedListIterator * const iterator);
} LListIterator;

pLinkedList createLinkedList();

pLinkedList createSearchLinkedList(int (*equal)(void *a, void *b));

void freeLinkedList(pLinkedList pList);

LListIterator *createLListInterator(const pLinkedList pList);

void freeLListInterator(LListIterator * iterator);

#endif /* INCLUDE_LINKEDLIST_H_ */
