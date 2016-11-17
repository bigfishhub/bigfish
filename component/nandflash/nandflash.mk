#####################build nandflash lib########################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := nandflash.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS := -lpthread

LOCAL_MODULE:= libnandflash

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

#####################build test bin########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := flashtest.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libnandflash
#LOCAL_STATIC_LIBRARIES := libnandflash

LOCAL_MODULE:= nandflash_test

include $(BUILD_EXECUTABLE)
