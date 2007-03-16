
ROOTDIR = ../..
STATIC_LIBRARY = gpu
FILES = gpukernel gpuruntime gpustream gpuiterator

include ../../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS := ogl dx9
ifdef BUILD_CTM
SUBDIRS += ctm
endif
else 
ifeq ($(OS), Linux)
SUBDIRS :=  ogl
ifdef BUILD_CTM
SUBDIRS += ctm
endif
endif
endif

include ../../common.mk






