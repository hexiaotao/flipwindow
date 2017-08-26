LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libremote
LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused
LOCAL_SRC_FILES := \
	libremote/IRemoteService.cpp \
	libremote/IRemoteServiceClient.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/
LOCAL_SHARED_LIBRARIES := \
	libbinder \
	liblog \
	libutils \
	libkeystore_binder

include $(BUILD_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_MODULE := flipwindow

LOCAL_MODULE_TAGS := tests

LOCAL_SRC_FILES := \
    flipwindow.cpp \
    bp/Remote.cpp

LOCAL_C_INCLUDES += \
    external/skia/src/core \
    external/skia/src/effects \
    external/skia/src/images \
    external/skia/include/gpu/ \
    $(LOCAL_PATH)/include/ \
    $(LOCAL_PATH)/bp/

LOCAL_REQUIRED_MODULES := libremote

LOCAL_STATIC_LIBRARIES := libremote
LOCAL_SHARED_LIBRARIES := \
    libEGL \
    libGLESv2 \
    libbinder \
    libcutils \
    libgui \
    libui \
    libutils \
    liblog \
    libkeystore_binder \
    libskia

include $(BUILD_EXECUTABLE)
