LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := uninstall_monitor
LOCAL_SRC_FILES := uninstall_monitor.cpp
LOCAL_LDLIBS 	:= -llog 

include $(BUILD_SHARED_LIBRARY)
