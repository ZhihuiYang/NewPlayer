/*!
 *  \file rtpheader.h
 *
 *  \date Created on: Jul 12, 2016
 *  \author: eyngzui
 *  \version   V1.0
 */

#ifndef INCLUDE_RTPHEADER_H_
#define INCLUDE_RTPHEADER_H_

#include <sys/types.h>
#include <stddef.h>

/*!
 * \def RTP_VERSION
 * \brief definition of RTP version, value is 2
 */
#define RTP_VERSION 2

/*!
 * \struct _rtp_hdr_t
 * \brief the memory pool structure
 */
typedef struct _rtp_hdr_t
{
    unsigned short version :2; /**< protocol version */
    unsigned short p :1; /**< padding flag */
    unsigned short x :1; /**< header extension flag */
    unsigned short cc :4; /**< CSRC bit */
    unsigned short m :1; /**< market bit */
    unsigned short pt :7; /**< payload type */
    unsigned short seq :16; /**< sequence number */
    unsigned int ts; /**< timestamp */
    unsigned int ssrc; /**< synchronization source */
} RTP_HDR_T, /**< typedef of struct _rtp_hdr_t */
*PRTP_HDR_T; /**< typedef of struct _rtp_hdr_t point*/

#endif /* INCLUDE_RTPHEADER_H_ */
