
ROOTDIR = ../..
STATIC_LIBRARY = gpu
FILES = gpukernel gpuruntime gpustream gpuiterator

include ../../config/DetectOS.mk

ifeq ($(OS), Windows_NT$(MOBILEPROFILE))
SUBDIRS := dx9
ifdef BUILD_CTM
SUBDIRS += ctm
endif
else 
ifeq ($(OS), Linux$(MOBILEPROFILE))
SUBDIRS := 
ifdef BUILD_CTM
SUBDIRS += ctm
endif
endif
endif

ifdef MOBILEPROFILE
SUBDIRS += gles
else
SUBDIRS += ogl
endif

include ../../common.mk






