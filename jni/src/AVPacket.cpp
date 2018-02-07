/*!
 * \file AVPakcet.cpp
 *
 * \date Created on: Nov 12, 2016
 * \author: eyngzui
 */

#include "include/AVPacket.h"
#include <string.h>
#ifdef DEBUG_MEM
#include "include/mymalloc.h"
#else
#include <malloc.h>
#endif

Lisp_AVPacket * createAVPacket()
{
    Lisp_AVPacket * pPacket = NULL;
    pPacket = (Lisp_AVPacket *) malloc(sizeof(Lisp_AVPacket));
    if (NULL == pPacket)
    {
        return NULL;
    }
    memset(pPacket, 0, sizeof(Lisp_AVPacket));
    return pPacket;
}

void freeAVPacket(Lisp_AVPacket ** ppPacket)
{
    Lisp_AVPacket * pPacket = *ppPacket;
    if (NULL == ppPacket || NULL == pPacket)
    {
        return;
    }
    
    if (NULL != pPacket->recvBuf)
    {
        free(pPacket->recvBuf);
    }

    free(pPacket);
    pPacket = NULL;
}
