/*!
 * \file memoryPool.h
 *
 * \author eyngzui
 * \date Created on: Jul 14, 2016
 * \version   V1.0
 */

#ifndef INCLUDE_MEMORYPOOL_H_
#define INCLUDE_MEMORYPOOL_H_

#include "include/commonDef.h"

/*!
 * \typedef DataType
 * \brief typedef u_char DataType.
 */
typedef u_char DataType;

/*!
 * \struct POOL
 * \brief the memory pool structure
 */
typedef struct POOL
{
    DataType **pBase; /**< point to memory pool */
    int front; /**< front index in memory pool */
    int rear; /**< rear index in memory pool */
    int maxPoolSize; /**< maximum size of memory pool */
    int dataLength; /**< length of memory slice in memory pool */
    void (*createPool)(struct POOL * pMP, int maxPoolSize, int dataLength); /**< funcion point to ceration */
    void (*traversePool)(struct POOL * pMP); /**< funcion point to traversion */
    bool (*isFullPool)(struct POOL * pMP); /**< funcion point to full memory pool detection */
    bool (*isEmptyPool)(struct POOL * pMP); /**< funcion point to empty memory pool detection */
    bool (*enPool)(struct POOL * pMP, DataType **val); /**< funcion point to enter memory slice to memory pool functionality  */
    bool (*dePool)(struct POOL * pMP, DataType **val); /**< funcion point to delete memory slice from memory pool functionality */
    void (*destoryPool)(struct POOL * pMP); /**< funcion point to destory memory pool */
} MEMORYPOOL, /**< typedef of struce POOL */
*PMEMORYPOOL; /**< typedef of struce POOL point*/

/*! \fn PMEMORYPOOL generateRTPMemoryPool(int queueSize, int dataLength = MTU)
 \brief generate a RTP memory pool.
 \param queueSize specifies the memory pool size.
 \param dataLength specifies the RTP package lenth, the defalut value is MTU(1500) bytes.
 */
PMEMORYPOOL generateRTPMemoryPool(int poolSize, int dataLength = MTU);

/*! \fn void destoryRTPMemoryPool(PMEMORYPOOL q)
 \brief destroy a RTP memory pool.
 \param q specifies the memory pool address.
 */
void destoryRTPMemoryPool(PMEMORYPOOL pMP);

/*! \fn PMEMORYPOOL generateYUVMemoryPool(int queueSize, int width, int height , int type = YUV420);
 \brief generate a YUV memory pool.
 \param queueSize specifies the memory pool size.
 \param width specifies the YUV frame width.
 \param height specifies the YUV frame width.
 \param type specifies the YUV frame type, default type is YUV420.
 */
PMEMORYPOOL generateYUVMemoryPool(int poolSize, int width, int height,
        int type = YUV420);

/*! \fn void destoryYUVMemoryPool(PMEMORYPOOL q)
 \brief destroy a YUV memory pool.
 \param q specifies the memory pool address.
 */
void destoryYUVMemoryPool(PMEMORYPOOL pMP);

#endif /* INCLUDE_MEMORYPOOL_H_ */
