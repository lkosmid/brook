ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt profiler logger writequery writemask

include ../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS := cpu gpu
LIBRARIES := cpu gpu ogl gpu-dx9
ifdef BUILD_CTM
LIBRARIES += gpu-ctm
EXTRAINCLUDEDIR = $(CTM_HOME)/libraries/amucgen $(CTM_HOME)/libraries/amuabi $(CTM_HOME)/libraries/amuisa $(CTM_HOME)/libraries/amucbuf $(CTM_HOME)/../../include
endif
else 
ifeq ($(OS), Linux)
SUBDIRS :=  cpu gpu
LIBRARIES := cpu gpu ogl
ifdef BUILD_CTM
LIBRARIES += gpu-ctm
EXTRAINCLUDEDIR = $(CTM_HOME)/libraries/amucgen $(CTM_HOME)/libraries/amuabi $(CTM_HOME)/libraries/amuisa $(CTM_HOME)/libraries/amucbuf $(CTM_HOME)/../../include
endif
else
SUBDIRS := cpu
endif
endif

include ../common.mk
