ROOTDIR := ../..
REGRESSIONDIRS := spMatrixVec fft
SUBDIRS := $(REGRESSIONDIRS) PMLSeg benchmark bitonic_sort bsearch \
	   conjugate_gradient imageproc particle_cloth timing raytracer

include ../../config/DetectOS.mk

ifeq ($(OS), Windows_NT)
SUBDIRS += renderDX
endif

include $(ROOTDIR)/common.mk

