LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	testEgl.cpp
#	localsocket.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
    libui \
    libgui\
	libbinder \
	libskia \
	libEGL \
	libGLESv1_CM 




LOCAL_MODULE:= resize

LOCAL_MODULE_TAGS := resize

include $(BUILD_EXECUTABLE)
