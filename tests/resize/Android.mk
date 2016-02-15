LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	resize_sync.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
    libui \
    libgui\
	libbinder \
	libskia

LOCAL_MODULE:= resize

LOCAL_MODULE_TAGS := resize

include $(BUILD_EXECUTABLE)
