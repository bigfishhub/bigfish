###########################################################
#write by gj
#shared_library.mk
###########################################################

LOCAL_MODULE := $(strip $(LOCAL_MODULE))

ifeq ($(LOCAL_MODULE),)
  $(error $(LOCAL_PATH): LOCAL_MODULE is not defined)
endif

ifeq ($(LOCAL_CC),)
LOCAL_CC := $(TARGET_CC)
endif

ifeq ($(LOCAL_CXX),)
LOCAL_CXX := $(TARGET_CXX)
endif

ifeq ($(LOCAL_MODULE_SUFFIX),)
LOCAL_MODULE_SUFFIX := $(TARGET_SHAREDLIB_SUFFIX)
endif

ifneq ($(LOCAL_INTERMEDIATE_TARGETS),)
$(error LOCAL_INTERMEDIATE_TARGETS should not define in component makefiles !)
endif

ALL_MODULES +=$(LOCAL_MODULE)

LOCAL_CFLAGS += $(TARGET_GLOBLE_CFLAGS)

LOCAL_LDFLAGS += $(TARGET_GLOBLE_LDFLAGS)

LOCAL_CPPFLAGS += $(TARGET_GLOBLE_CPPFLAGS)

#output prefix set
obj_prefix :=$(OBJ_DIR)/$(LOCAL_MODULE).SHARED
lib_prefix :=$(LIB_DIR)/share
so_suffix :=$(TARGET_SHAREDLIB_SUFFIX)
a_suffix :=$(TARGET_STATICLIB_SUFFIX)
my_sharedlib_prefix :=$(TARGET_GLOBAL_LD_SHARED_DIRS)
my_staticlib_prefix :=$(TARGET_GLOBAL_LD_STATIC_DIRS)
###########################################################
##
## C: compile .c source files to .o
##
###########################################################

target_c_source := $(filter %.c, $(LOCAL_SRC_FILES))

target_c_objs   := $(addprefix $(obj_prefix)/,$(target_c_source:.c=.o))

#debug,checking the source files
#$(info checking c source:$(target_c_source))

ifneq ($(strip $(target_c_objs)),)
$(target_c_objs):$(obj_prefix)/%.o:$(LOCAL_PATH)/%.c
	@echo "<<< compile c objs $@:"
	mkdir -p $(dir $@)
	$(Q)$(PRIVATE_CC) \
	$(addprefix -I , $(PRIVATE_C_INCLUDES)) \
	-c $(PRIVATE_CFLAGS)  -o $@ $^
endif

###########################################################
##
## C++: compile .cpp source files to .o
##
###########################################################

target_cpp_source := $(filter %.cpp, $(LOCAL_SRC_FILES))

target_cpp_objs   := $(addprefix $(obj_prefix)/,$(target_cpp_source:.cpp=.o))

#debug,checking the source files
#$(info checking cpp source:$(target_cpp_source))

ifneq ($(strip $(target_cpp_objs)),)
$(target_cpp_objs):$(obj_prefix)/%.o:$(TOP_DIR)$(LOCAL_PATH)/%.cpp
	@echo "<<< compile cpp objs $@:"
	@mkdir -p $(dir $@)
	$(Q)$(PRIVATE_CXX) \
	$(addprefix -I , $(PRIVATE_C_INCLUDES)) \
	-c $(PRIVATE_CPPFLAGS)  -o $@ $^
endif

###########################################################
##
## To shared libs: compile .o  files to .so shared libs
##
###########################################################

target_all_objs := $(target_c_objs) $(target_cpp_objs) 

target_all_source := $(target_c_source) $(target_cpp_source)

#########################################################
## required modules
#########################################################

#all_required_modules := $(LOCAL_REQUIRED_MODULES)

#########################################################
##
## link libraries
##
#########################################################

built_shared_libraries := \
    $(addprefix $(my_sharedlib_prefix)/, \
      $(addsuffix $(so_suffix), \
        $(LOCAL_SHARED_LIBRARIES)))

built_static_libraries := \
    $(addprefix $(my_staticlib_prefix)/, \
      $(addsuffix $(a_suffix), \
        $(LOCAL_STATIC_LIBRARIES)))

#$(info $(target_cpp_source))
#$(info $(MAKEFILE_LIST))
#$(info $(target_all_source))

#########################################################
## required modules
#########################################################

#define LOCAL_INTERMEDIATE_TARGETS for modules
LOCAL_INTERMEDIATE_TARGETS := $(target_all_objs) $(target_all_source) \
                              $(LOCAL_MODULE)-t-shlib-check \
                              $(LOCAL_MODULE)-t-shlib-clean \
                              $(lib_prefix)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)

#$(info LOCAL_INTERMEDIATE_TARGETS ,$(LOCAL_INTERMEDIATE_TARGETS))

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CC := $(LOCAL_CC)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CXX := $(LOCAL_CXX)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CFLAGS := $(LOCAL_CFLAGS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CPPFLAGS := $(LOCAL_CPPFLAGS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_AR := $(LOCAL_AR)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ARFLAGS := $(LOCAL_ARFLAGS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LOCAL_MODULE := $(LOCAL_MODULE)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_MODULE_SUFFIX := $(LOCAL_MODULE_SUFFIX)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_OBJ_DIR := $(obj_prefix)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LIB_DIR := $(lib_prefix)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_OBJS := $(target_all_objs)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_ALL_SOURCE := $(target_all_source)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_SHARED_LIBRARIES := $(built_shared_libraries)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_STATIC_LIBRARIES := $(built_static_libraries)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TARGET_LD_SHARED_DIRS := $(TARGET_GLOBAL_LD_SHARED_DIRS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TARGET_LD_STATIC_DIRS := $(TARGET_GLOBAL_LD_STATIC_DIRS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_C_INCLUDES := $(LOCAL_C_INCLUDES)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LOCAL_PATH := $(LOCAL_PATH)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LDFLAGS := $(LOCAL_LDFLAGS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LDLIBS := $(LOCAL_LDLIBS)

all_libraries := $(built_static_libraries) $(built_shared_libraries)
	
.PHONY:$(LOCAL_MODULE)

# module final target
$(LOCAL_MODULE):$(lib_prefix)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)
$(LOCAL_MODULE)-check:$(LOCAL_MODULE)-t-shlib-check
$(LOCAL_MODULE)-clean:$(LOCAL_MODULE)-t-shlib-clean

$(lib_prefix)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX):$(target_all_objs) $(all_libraries)
	@echo "<<< compile shared lib $@:"
	@mkdir -p $(dir $@)
	$(Q)$(PRIVATE_CXX) -shared \
	-L$(PRIVATE_TARGET_LD_STATIC_DIRS) \
	-o $@ \
	$(PRIVATE_ALL_OBJS) \
	-Wl,--whole-archive \
	$(call normalize-target-libraries,$(PRIVATE_STATIC_LIBRARIES)) \
	-Wl,--no-whole-archive \
	-L$(PRIVATE_TARGET_LD_SHARED_DIRS) \
	$(call normalize-target-libraries,$(PRIVATE_SHARED_LIBRARIES)) \
	$(PRIVATE_LDFLAGS) \
	$(PRIVATE_LDLIBS)


$(LOCAL_MODULE)-t-shlib-check:
	$(dump-module-variables)

$(LOCAL_MODULE)-t-shlib-clean:
	$(Q)rm -rf $(PRIVATE_OBJ_DIR)
	$(Q)rm -rf $(PRIVATE_LIB_DIR)/$(PRIVATE_LOCAL_MODULE)$(PRIVATE_MODULE_SUFFIX)
