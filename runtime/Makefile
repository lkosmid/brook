ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt
LIBRARIES := cpu
SUBDIRS := cpu
ifeq ($(OS), Windows_NT)
LIBRARIES += dx9 nv30gl
SUBDIRS += dx9 nv30gl
endif

include ../common.mk

