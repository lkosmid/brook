ROOTDIR := ../..
REGRESSIONDIRS := spMatrixVec fft
SUBDIRS := $(REGRESSIONDIRS) PMLSeg bitonic_sort bsearch \
	   conjugate_gradient imageproc particle_cloth timing raytracer subdivision rapid_collide

include ../../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += renderDX
endif

include $(ROOTDIR)/common.mk

