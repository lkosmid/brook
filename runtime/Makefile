ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt profiler logger

include ../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += cpu gpu dx9
LIBRARIES := cpu gpu dx9 ogl gpu-dx9
#SUBDIRS += cpu dx9 nv30gl
#LIBRARIES := cpu dx9 nv30gl
else 
ifeq ($(OS), Linux)
SUBDIRS :=  cpu nv30gl
else
SUBDIRS := cpu
endif
endif

include ../common.mk
