ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt profiler logger

include ../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += cpu nv30gl dx9
LIBRARIES := cpu nv30gl dx9
else 
ifeq ($(OS), Linux)
SUBDIRS :=  cpu nv30gl
else
SUBDIRS := cpu
endif
endif

include ../common.mk
