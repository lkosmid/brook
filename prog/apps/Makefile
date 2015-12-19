ROOTDIR := ../..
REGRESSIONDIRS := spMatrixVec fft
SUBDIRS := $(REGRESSIONDIRS) PMLSeg bitonic_sort bsearch \
	   conjugate_gradient imageproc particle_cloth rapid_collide #timing raytracer subdivision volume_division 

include ../../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += renderDX
endif

include $(ROOTDIR)/common.mk

