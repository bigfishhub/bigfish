#####################build jpeg lib########################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := src/jpegmanage.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base \
					$(LOCAL_PATH)/inc \
					$(OUTDIR)/include

LOCAL_SHARED_LIBRARIES := libjpeg

LOCAL_LDLIBS := -lpthread

LOCAL_MODULE:= libjpegmanage

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

#####################build test bin########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := test/main.cpp

LOCAL_C_INCLUDES := $(BIGPIG_TOP)/base \
					$(LOCAL_PATH)/inc \
					$(OUTDIR)/include

LOCAL_LDLIBS := -lpthread

LOCAL_SHARED_LIBRARIES := libjpegmanage
#LOCAL_STATIC_LIBRARIES := libnandflash

LOCAL_MODULE:= jpegmanage_test

include $(BUILD_EXECUTABLE)
