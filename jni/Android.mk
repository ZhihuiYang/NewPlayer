LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libopenh264
LOCAL_SRC_FILES := ../libs/openh264/libopenh264.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavcodec
LOCAL_SRC_FILES := ../libs/ffmpeg/libavcodec-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavdevice
LOCAL_SRC_FILES := ../libs/ffmpeg/libavdevice-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavfilter
LOCAL_SRC_FILES := ../libs/ffmpeg/libavfilter-6.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavformat
LOCAL_SRC_FILES := ../libs/ffmpeg/libavformat-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavutil
LOCAL_SRC_FILES := ../libs/ffmpeg/libavutil-55.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libswresample
LOCAL_SRC_FILES := ../libs/ffmpeg/libswresample-2.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libswscale
LOCAL_SRC_FILES := ../libs/ffmpeg/libswscale-4.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := mediaPlayer
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./libass
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./include/ffmpeg
LOCAL_SRC_FILES := ./src/MediaPlayer.cpp \
					./src/AVFrame.cpp \
					./src/AVPacket.cpp \
					./src/memoryPool.cpp \
					./src/RtpPacket.cpp \
					./src/h264_RtpPacket.cpp \
					./src/Sender.cpp \
					./src/Receiver.cpp \
					./src/VideoRender.cpp \
					./src/VideoRender2.cpp \
					./src/linkedList.cpp \
					./src/LinkedQueue.cpp \
					./src/jitterBuffer.cpp \
					./src/VideoEncoder_H264.cpp \
					./src/VideoEncoderFactory.cpp \
					./src/decoderCommon.cpp \
					./src/decoderFactory.cpp \
					./src/decoder_AAC.cpp \
					./src/decoder_ASS.cpp \
					./src/decoder_H264.cpp \
					./src/MediaPlayer.cpp \
					./src/recorder.cpp \
					./src/com_ericsson_lispmediaplayer_Player.cpp \
					./src/com_ericsson_lispmediaplayer_Recorder.cpp \
					./src/MediaExtract.cpp \
										
LOCAL_LDFLAGS := -llog -pthread -landroid -lEGL -lGLESv3
#LOCAL_CPPFLAGS += -std=c++11

#LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/ffmpeg
LOCAL_SHARED_LIBRARIES := libopenh264 libavcodec  libavfilter libavformat libavutil libswresample libswscale libavdevice


ifdef IS_DEBUG
	LOCAL_CFLAGS += -DLOG_LEVEL_DEBUG -DDUMP_DATA -DRENDER2
endif
ifdef IS_INFO
	LOCAL_CFLAGS += -DLOG_LEVEL_INFO 
endif

include $(BUILD_SHARED_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE    := ffmpegPlayerDemo
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/./include/ffmpeg
LOCAL_SRC_FILES := ./src/ffmpegPlayerDemo.cpp \
					./src/AVFrame.cpp \
					./src/VideoRender2.cpp \
					./src/VideoRender.cpp \
					./src/ffmpegPlayerNative.cpp

LOCAL_LDFLAGS := -llog -pthread -landroid -lEGL -lGLESv3

ifdef IS_DEBUG
	LOCAL_CFLAGS += -DLOG_LEVEL_DEBUG -DRENDER2
endif
ifdef IS_INFO
	LOCAL_CFLAGS += -DLOG_LEVEL_INFO 
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/ffmpeg
LOCAL_SHARED_LIBRARIES := libopenh264 libavcodec  libavfilter libavformat libavutil libswresample libswscale


include $(BUILD_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := libass
FILE_LIST := $(./libass/*.c)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libass
LOCAL_SRC_FILES :=  $(FILE_LIST)
include $(BUILD_SHARED_LIBRARY)
