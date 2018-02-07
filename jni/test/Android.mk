LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := foo_unittest
LOCAL_SRC_FILES := foo_unittest.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include

LOCAL_SHARED_LIBRARIES := mediaPlayer
LOCAL_STATIC_LIBRARIES := googletest_main

NDK_APP_DST_DIR  := ./libs/$(TARGET_ARCH_ABI)

include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/../Android.mk
$(call import-module,third_party/googletest)