/*
 * mymalloc.h
 *
 *  Created on: Jul 11, 2016
 *      Author: yangzhihui
 */


#pragma once

#ifdef __ANDROID__
#include <android/log.h>
#else ifdef __APPLE__
#include "log.h"
#endif

#define MY_LOG_LEVEL_VERBOSE    1
#define MY_LOG_LEVEL_DEBUG      2
#define MY_LOG_LEVEL_INFO       3
#define MY_LOG_LEVEL_WARNING    4
#define MY_LOG_LEVEL_ERROR      5
#define MY_LOG_LEVEL_FATAL      6
#define MY_LOG_LEVEL_SILENT     7

#ifndef MY_LOG_TAG
#define MY_LOG_TAG "###EARPHONE###"
#endif


#if defined(LOG_LEVEL_DEBUG)
#define MY_LOG_LEVEL MY_LOG_LEVEL_DEBUG
#elif defined(LOG_LEVEL_INFO)
#define MY_LOG_LEVEL MY_LOG_LEVEL_INFO
#elif defined(LOG_LEVEL_WARNING)
#define MY_LOG_LEVEL MY_LOG_LEVEL_WARNING
#else
#define MY_LOG_LEVEL MY_LOG_LEVEL_ERROR
#endif

#define MY_LOG_NOOP (void) 0


#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#ifdef __ANDROID__
#define MY_LOG_PRINT(level, fmt, ...) \
    __android_log_print(level, MY_LOG_TAG, "(%s:%u) %s: " fmt,\
            __FILENAME__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else ifdef __APPLE__
#define MY_LOG_PRINT(level, fmt, ...) \
    log_message(level, fmt, ##__VA_ARGS__)
#endif


#ifdef __ANDROID__
#define VERBOSE_LEVEL ANDROID_LOG_VERBOSE
#define DEBUG_LEVEL ANDROID_LOG_DEBUG
#define INFO_LEVEL ANDROID_LOG_INFO
#define WARN_LEVEL ANDROID_LOG_WARN
#define ERROR_LEVEL ANDROID_LOG_ERROR
#define FATAL_LEVEL ANDROID_LOG_FATAL
#endif 

#ifdef __APPLE__
#define VERBOSE_LEVEL "[V]"
#define DEBUG_LEVEL "[D]"
#define INFO_LEVEL "[I]"
#define WARN_LEVEL "[W]"
#define ERROR_LEVEL "[E]"
#define FATAL_LEVEL "[F]"
#endif


#if MY_LOG_LEVEL_VERBOSE >= MY_LOG_LEVEL
#define TVGUO_LOG_VERBOSE(fmt, ...) \
    MY_LOG_PRINT(VERBOSE_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_VERBOSE(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_DEBUG >= MY_LOG_LEVEL
#define TVGUO_LOG_DEBUG(fmt, ...) \
    MY_LOG_PRINT(DEBUG_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_DEBUG(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_INFO >= MY_LOG_LEVEL
#define TVGUO_LOG_INFO(fmt, ...) \
    MY_LOG_PRINT(INFO_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_INFO(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_WARNING >= MY_LOG_LEVEL
#define TVGUO_LOG_WARNING(fmt, ...) \
    MY_LOG_PRINT(WARN_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_WARNING(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_ERROR >= MY_LOG_LEVEL
#define TVGUO_LOG_ERROR(fmt, ...) \
    MY_LOG_PRINT(ERROR_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_ERROR(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_FATAL >= MY_LOG_LEVEL
#define TVGUO_LOG_FATAL(fmt, ...) \
    MY_LOG_PRINT(FATAL_LEVEL, fmt, ##__VA_ARGS__)
#else
#define TVGUO_LOG_FATAL(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_SILENT >= MY_LOG_LEVEL
#ifdef __ANDROID__
#define TVGUO_LOG_ASSERT(expression, fmt, ...) \
    if (!(expression)) \
    { \
        __android_log_assert(#expression, MY_LOG_TAG, \
                fmt, ##__VA_ARGS__); \
    }
#else ifdef __APPLE__
#define TVGUO_LOG_ASSERT(expression, fmt, ...) \
    if (!(expression)) \
    { \
        NSAssert(#expression, MY_LOG_TAG, \
                fmt, ##__VA_ARGS__); \
    }

#endif
#else
#define TVGUO_LOG_ASSERT(...) MY_LOG_NOOP
#endif

