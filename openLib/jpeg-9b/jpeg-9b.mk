LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libjpeg
LOCAL_SOURCE_TARBALL := $(LOCAL_PATH)/jpegsrc.v9b.tar.gz
LOCAL_SOURCE_DIR := $(LOCAL_PATH)/jpeg-9b
LOCAL_INSTALL_PATH := $(LOCAL_PATH)/jpeginstall
LOCAL_OPENSOURCE_PREFIX := $(OUTDIR)/external/$(LOCAL_MODULE)

ifeq ($(LOCAL_CC),)
LOCAL_CC := $(TARGET_CC)
endif

ifeq ($(LOCAL_AR),)
LOCAL_AR := $(TARGET_AR)
endif

LOCAL_INTERMEDIATE_TARGETS := $(LOCAL_PATH)/.$(LOCAL_MODULE) \
                              $(LOCAL_MODULE)-clean \
                              $(LOCAL_MODULE)-remove \
                              $(LOCAL_PATH)/.$(LOCAL_MODULE)-install \
                              $(LOCAL_MODULE)

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_CC := $(LOCAL_CC)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_AR := $(LOCAL_AR)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LOCAL_PATH := $(LOCAL_PATH)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_LOCAL_MODULE := $(LOCAL_MODULE)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_SOURCE_TARBALL := $(LOCAL_SOURCE_TARBALL)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_SOURCE_DIR := $(LOCAL_SOURCE_DIR)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_OPENSOURCE_PREFIX := $(LOCAL_OPENSOURCE_PREFIX)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_OPENSOURCE_INSTALL_DIR := $(LOCAL_INSTALL_PATH)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TARGET_LD_SHARED_DIRS := $(TARGET_GLOBAL_LD_SHARED_DIRS)
$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_TARGET_LD_STATIC_DIRS := $(TARGET_GLOBAL_LD_STATIC_DIRS)

LOCAL_REQUIRED_MODULES := 

$(LOCAL_MODULE):$(LOCAL_REQUIRED_MODULES) $(LOCAL_PATH)/.$(LOCAL_MODULE)-install
	@echo "====== copy libjpeg begin!"
	@cp -d -rf $(PRIVATE_OPENSOURCE_INSTALL_DIR)/lib/libjpeg.so*  $(PRIVATE_TARGET_LD_SHARED_DIRS)
	#@cp $(PRIVATE_OPENSOURCE_INSTALL_DIR)/lib/libjpeg.a  $(PRIVATE_TARGET_LD_STATIC_DIRS)
	@cp $(PRIVATE_OPENSOURCE_INSTALL_DIR)/include/*  $(OUTDIR)/include
	@echo "====== copy libjpeg end!"

$(LOCAL_PATH)/.$(LOCAL_MODULE) :
	@echo "====== build libjpeg"
	@if [ ! -d $(PRIVATE_SOURCE_DIR) ] ; then \
		echo "====== Unpack.... libjpeg"; \
		cd $(PRIVATE_LOCAL_PATH); \
		tar xvzf $(PRIVATE_SOURCE_TARBALL) -C  $(PRIVATE_LOCAL_PATH) 1> /dev/null; \
	fi

$(LOCAL_MODULE)-clean:
	@echo "====== MAKE $(PRIVATE_SOURCE_DIR) CLEAN"
	@-rm $(PRIVATE_SOURCE_DIR) -rf
	@-rm $(PRIVATE_OPENSOURCE_INSTALL_DIR) -rf
	@-rm $(PRIVATE_TARGET_LD_SHARED_DIRS)/libjpeg.so
	@-rm $(PRIVATE_TARGET_LD_SHARED_DIRS)/libjpeg.so.9.2.0
	@-rm $(PRIVATE_TARGET_LD_STATIC_DIRS)/libjpeg.a
	@echo "====== Done"

$(LOCAL_PATH)/.$(LOCAL_MODULE)-install:$(LOCAL_PATH)/.$(LOCAL_MODULE)
	@echo "====== $(PRIVATE_SOURCE_DIR) Install begin!"
	@if [ ! -e $(PRIVATE_SOURCE_DIR)/Makefile ]; then \
		echo "====== [Config.... libjpeg]"; \
		cd $(PRIVATE_SOURCE_DIR); \
		sh ./configure --prefix=$(PRIVATE_OPENSOURCE_INSTALL_DIR) \
		--host=$(CROSS) CC=$(PRIVATE_CC)  --enable-shared \
		--enable-static 1> /dev/null; \
		sleep 1; \
	fi
	@if [[ ! -e $(PRIVATE_OPENSOURCE_INSTALL_DIR)/lib/libjpeg.so || \
		! -e $(PRIVATE_OPENSOURCE_INSTALL_DIR)/lib/libjpeg.a ]]; then \
		echo "====== [Build..... libjpeg]"; \
		mkdir -p $(PRIVATE_OPENSOURCE_INSTALL_DIR); \
		$(MAKE) -C $(PRIVATE_SOURCE_DIR) 1>/dev/null; \
		$(MAKE) -C $(PRIVATE_SOURCE_DIR) install 1>/dev/null; \
	fi
	@echo "====== $(PRIVATE_SOURCE_DIR) Installed!"

$(LOCAL_MODULE)-remove:
	@echo "====== MAKE $(PRIVATE_SOURCE_DIR) remove,not implement yet !!!"

include $(BUILD_OPENSOURCE_PKG)

