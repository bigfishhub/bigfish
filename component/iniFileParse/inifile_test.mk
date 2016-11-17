#####################build test bin########################
LOCAL_PATH:= $(call my-dir)
	
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libutils
#LOCAL_STATIC_LIBRARIES := libnandflash

LOCAL_MODULE:= inifile_test

include $(BUILD_EXECUTABLE)