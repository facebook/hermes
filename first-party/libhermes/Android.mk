# This file gets copied by the react-native build

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= hermes
LOCAL_SRC_FILES := jni/$(TARGET_ARCH_ABI)/libhermes.so
#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
include $(PREBUILT_SHARED_LIBRARY)
