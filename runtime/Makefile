ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt

include ../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += uioowui cpu nv30gl dx9
LIBRARIES := asdfsdaf cpu nv30gl dx9
else 
ifeq ($(OS), Linux)
SUBDIRS := dsfasdf cpu nv30gl
else
SUBDIRS := cpu
endif
endif

include ../common.mk
