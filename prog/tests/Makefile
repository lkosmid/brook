ROOTDIR := ../..
REGRESSIONDIRS := accumulate allocate constant convert_iter_to_stream gather_dependent \
		  gather_direct gatherop indexof indexof_output \
		  input_output iterator iterator_dimensions multiple_output \
		  read_write reduce reduce_basic reducetostream \
		  replication_stride struct_gather struct_input \
		  struct_output sum toy scan scatter pack_vout vout

#ifdef BRT_RUNTIME
#ifneq ($(BRT_RUNTIME), arb)
#REGRESSIONDIRS := $(REGRESSIONDIRS) pack_vout vout
#endif
#endif

SUBDIRS := $(REGRESSIONDIRS) fft infinite tessellate
# big

include $(ROOTDIR)/common.mk

