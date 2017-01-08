LOCAL_PATH:= $(call my-dir)
	
include $(CLEAR_VARS)

LOCAL_SRC_FILES := spwptest.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_MODULE:= spwp_test

include $(BUILD_EXECUTABLE)

#=============================================================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := mytest.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_MODULE:= my_test

include $(BUILD_EXECUTABLE)