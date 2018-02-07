/*
 * commenType.h
 *
 *  Created on: Jul 14, 2016
 *      Author: eyngzui
 */

#ifndef INCLUDE_COMMENTYPE_H_
#define INCLUDE_COMMENTYPE_H_

#define MTU 1500
#define _720P (1280 * 720)
#define _1080P (1920 * 1080)

typedef enum _yuvtype
{
    YUV444 = 1,
    YUV422,
    YUV420
} YUVTYPE;

typedef enum _pixelFormt
{
    Pixel_Format_NONE = -1,
    Pixel_Format_YUV420P,
    Pixel_Format_YUV422P,
    Pixel_Format_YUV444P,
    Pixel_Format_GRAY8,
    Pixel_Format_RGB565LE,
    Pixel_Format_RGB565BL,
    Pixel_Format_BGR565LE,
    Pixel_Format_BGR565BL
} PixelFormat;

#endif /* INCLUDE_COMMENTYPE_H_ */
