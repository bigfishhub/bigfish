
Q := @

EXTRA_CLEAN_MODULES :=
ALL_MODULES :=

all:all_modules

include $(BIGPIG_TOP)/build/config.mk
include $(BIGPIG_TOP)/build/definitions.mk
include $(BIGPIG_TOP)/build/module-list.mk

#TARGET_GLOBLE_INCLUDE_H += $(BIGPIG_TOP)/base/

all_modules:$(ALL_MODULES)
	@echo "all modules building down"

.PHONY:show_modules
show_modules:
	@echo "Available sub-modules:"
	@echo "$(ALL_MODULES)" | \
	tr -s ' ' '\n' | sort -u | column

.PHONY:check
check:
	@echo "CROSS:$(CROSS)"
	@echo "TARGET_CC:$(TARGET_CC)"
	@echo "TARGET_CXX:$(TARGET_CXX)"
	@echo "TOP:$(TOP)"
	@echo "TOP_DIR:$(TOP_DIR)"
	@echo "OUTDIR:$(OUTDIR)"
	@echo "OBJ_DIR:$(OBJ_DIR)"
	@echo "BIN_DIR:$(BIN_DIR)"
	@echo "LIB_DIR:$(LIB_DIR)"
	@echo "all_modules are :$(ALL_MODULES)"

extra_cleans:$(EXTRA_CLEAN_MODULES)
	@echo "extra modules clean done!"

clean:extra_cleans
	@rm -rf $(OUTDIR)/*

.PHONY:help

help:help-info
	
help-info:
	@echo  "==============================================================================="
	@echo  "==========================     Makefile Target Help    ========================="
	@echo  "==============================================================================="
	@echo  "  Makefile targets:"
#	@echo  "  flyfish                 - build all modules and make image"
	@echo  "  clean                   - remove all generated files from all  modules"
	@echo  "  help (or blank)         - display this help"
	@echo  "  all_modules             - build all modules for this project"
	@echo  "  LOCAL_MODULE            - build module as requred,for example,use 'make libbinder_ipc' to compile ipc modules"
	@echo  "  LOCAL_MODULE-clean      - remove all generated files from module LOCAL_MODULE,for example,make libbinder-clean"
	@echo  "  LOCAL_MODULE-check      - check module flags and source,objs for LOCAL_MODULE,for example,make libbinder-check"
	@echo  "  show_modules            - show all the Available modules in the project"
#	@echo  "  image                   - build rootfs image,and copy image file to target/image directory"
#	@echo  "  menuconfig              - run the menuconfig tools to config the compile options"
	@echo  ""