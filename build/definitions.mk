###########################################################
# write by gj
# definitions.mk
###########################################################

###########################################################
## Retrieve the directory of the current makefile
###########################################################

# Figure out where we are.
define my-dir
$(strip \
  $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
  $(if $(filter $(CLEAR_VARS),$(LOCAL_MODULE_MAKEFILE)), \
    $(error LOCAL_PATH must be set before including $$(CLEAR_VARS)) \
   , \
    $(patsubst %/,%,$(dir $(LOCAL_MODULE_MAKEFILE))) \
   ) \
 )
endef

###########################################################
## Convert "path/to/libXXX.so" to "-lXXX".
## Any "path/to/libXXX.a" elements pass through unchanged.
###########################################################

define normalize-libraries
$(foreach so,$(filter %.so,$(1)),-l$(patsubst lib%.so,%,$(notdir $(so))))\
$(filter-out %.so,$(1))
endef

# TODO: change users to call the common version.
define normalize-host-libraries
$(call normalize-libraries,$(1))
endef

define normalize-target-libraries
$(call normalize-libraries,$(1))
endef

###########################################################
## Dump the variables that are associated with targets
###########################################################

define dump-module-variables
@echo check modules=$@
@echo LOCAL_CC=$(PRIVATE_CC)
@echo LOCAL_CXX=$(PRIVATE_CXX)
@echo LOCAL_CFLAGS=$(PRIVATE_CFLAGS)
@echo LOCAL_CPPFLAGS=$(PRIVATE_CPPFLAGS)
@echo LOCAL_LDFLAGS=$(PRIVATE_LDFLAGS)
@echo LOCAL_MODULE=$(PRIVATE_LOCAL_MODULE)
@echo LOCAL_MODULE_SUFFIX=$(PRIVATE_MODULE_SUFFIX)
@echo LOCAL_AR=$(PRIVATE_AR)
@echo LOCAL_ARFLAGS=$(PRIVATE_ARFLAGS)
@echo obj_prefix=$(PRIVATE_OBJ_DIR)
@echo bin_prefix=$(PRIVATE_BIN_DIR)
@echo LOCAL_C_INCLUDES=$(PRIVATE_C_INCLUDES)
@echo LOCAL_PATH=$(PRIVATE_LOCAL_PATH)
@echo target_all_objs=$(PRIVATE_ALL_OBJS)
@echo target_all_source=$(PRIVATE_ALL_SOURCE)
@echo built_shared_libraries=$(PRIVATE_SHARED_LIBRARIES)
@echo built_static_libraries=$(PRIVATE_STATIC_LIBRARIES)
@echo all_required_modules=$(PRIVATE_REQUIRED_MODULES)
@echo TARGET_LD_SHARED_DIRS=$(PRIVATE_TARGET_LD_SHARED_DIRS)
@echo TARGET_LD_STATIC_DIRS=$(PRIVATE_TARGET_LD_STATIC_DIRS)
@echo LOCAL_LDLIBS=$(PRIVATE_LDLIBS)
endef