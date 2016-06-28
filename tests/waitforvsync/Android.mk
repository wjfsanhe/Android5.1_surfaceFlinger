LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	waitforvsync.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_MODULE:= vsync

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
