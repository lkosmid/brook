ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt profiler logger

include ../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += cpu nv30gl dx9 gpu
LIBRARIES := cpu nv30gl dx9 gpu
else 
ifeq ($(OS), Linux)
SUBDIRS :=  cpu nv30gl gpu
else
SUBDIRS := cpu
endif
endif

include ../common.mk
