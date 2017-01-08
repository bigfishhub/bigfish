#####################build utils lib########################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := iniFile.cpp \
                   timers.cpp  \
                   thread.cpp  \
                   RefBase.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base/include

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS := -lpthread

LOCAL_MODULE:= libutils

include $(BUILD_SHARED_LIBRARY)
