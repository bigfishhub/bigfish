###########################################################                                                                         # Copyright (C) 2015-2020 Hisilion
# config.mk
###########################################################

export CROSS:=$(CFG_TOOLCHAINS_NAME)

#define build varibales
TOPDIR        := $(BIGPIG_TOP)
BUILD_SYSTEM  := $(TOPDIR)/build
OUTDIR        := $(TOPDIR)/target

##########################################################
# define build template variable
##########################################################
BUILD_OPENSOURCE_PKG      := $(BUILD_SYSTEM)/opensource_pkg.mk
BUILD_STATIC_LIBRARY      := $(BUILD_SYSTEM)/static_library.mk
BUILD_SHARED_LIBRARY      := $(BUILD_SYSTEM)/shared_library.mk
BUILD_EXECUTABLE          := $(BUILD_SYSTEM)/executable.mk
CLEAR_VARS                := $(BUILD_SYSTEM)/clear_vars.mk

##########################################################
# define comile cmd for target machine
##########################################################

ifeq ($(CFG_HOST),n)
TARGET_AR      := $(CROSS)-ar
TARGET_AS      := $(CROSS)-as
TARGET_LD      := $(CROSS)-ld
TARGET_CXX     := $(CROSS)-g++
TARGET_CC      := $(CROSS)-gcc
TARGET_NM      := $(CROSS)-nm
TARGET_STRIP   := $(CROSS)-strip
TARGET_OBJCOPY := $(CROSS)-objcopy
TARGET_OBJDUMP := $(CROSS)-objdump
TARGET_RANLIB  := $(CROSS)-ranlib
else
TARGET_AR      := ar
TARGET_AS      := as
TARGET_LD      := ld
TARGET_CXX     := g++
TARGET_CC      := gcc
TARGET_NM      := nm
TARGET_STRIP   := strip
TARGET_OBJCOPY := objcopy
TARGET_OBJDUMP := objdump
TARGET_RANLIB  := ranlib
endif

TARGET_GLOBLE_INCLUDE_H  := 
TARGET_GLOBLE_LDFLAGS    :=
TARGET_GLOBLE_CFLAGS     := -Wall -O2 -fPIC 
TARGET_GLOBLE_CPPFLAGS   := -Wall -O2 -fPIC 
TARGET_GLOBLE_ARFLAGS    := rcs

TARGET_EXECUTABLE_SUFFIX :=
TARGET_SHAREDLIB_SUFFIX  :=.so
TARGET_STATICLIB_SUFFIX  :=.a

#####################################################
# target output dirs
#####################################################
LIB_DIR      := $(OUTDIR)/lib
BIN_DIR      := $(OUTDIR)/bin
INCLUDE_DIR  := $(OUTDIR)/include
CONFIG_DIR   := $(OUTDIR)/config
OBJ_DIR      := $(OUTDIR)/obj

TARGET_GLOBAL_LD_DIRS        :=$(LIB_DIR)
TARGET_GLOBAL_LD_SHARED_DIRS :=$(LIB_DIR)/share
TARGET_GLOBAL_LD_STATIC_DIRS :=$(LIB_DIR)/static

####################################################
# common shell cmds
####################################################
MKDIR :=mkdir
CP :=cp
RM :=rm
TOUCH := touch
$(shell mkdir -p $(TOPDIR)/target/include)
$(shell mkdir -p $(TOPDIR)/target/lib)
$(shell mkdir -p $(TOPDIR)/target/lib/static)
$(shell mkdir -p $(TOPDIR)/target/lib/share)
$(shell mkdir -p $(TOPDIR)/target/lib/extern)
$(shell mkdir -p $(TOPDIR)/target/image)
$(shell mkdir -p $(TOPDIR)/target/config)
$(shell mkdir -p $(TOPDIR)/target/bin)
