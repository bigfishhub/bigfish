#####################build datadb lib########################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := datadb.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS := -lpthread

LOCAL_MODULE:= libdatadb

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

#####################build test bin########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := datatest.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libdatadb
#LOCAL_STATIC_LIBRARIES := libnandflash

LOCAL_MODULE:= datadb_test

include $(BUILD_EXECUTABLE)
