ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt

SUBDIRS := cpu nv30gl

ifeq ($(OS), Windows_NT)
SUBDIRS += dx9
LIBRARIES := cpu nv30gl dx9
endif

include ../common.mk
