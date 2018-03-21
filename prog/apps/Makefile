include ../../config/DetectOS.mk

ROOTDIR := ../..
REGRESSIONDIRS := spMatrixVec 
SUBDIRS := $(REGRESSIONDIRS) bitonic_sort bsearch \
	   conjugate_gradient_1D imageproc sgemm sgemm_multipass\
	   BinomialOptionPricing$(MOBILEPROFILE) bitonic_sort_ATISDK black_scholes$(MOBILEPROFILE) \
	   cholesky$(MOBILEPROFILE) floyd_warshall$(MOBILEPROFILE) haar_wavelet$(MOBILEPROFILE) image_filter\
	   matmult mandelbrot_ATISDK prefix_sum sparse_matrix_vector

	   #timing raytracer subdivision volume_division 
ifeq ($(OS), Windows_NT$(MOBILEPROFILE))
SUBDIRS += renderDX
endif

include $(ROOTDIR)/common.mk

