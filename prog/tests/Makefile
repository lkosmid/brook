ROOTDIR := ../..
REGRESSIONDIRS := accumulate allocate constant convert_iter_to_stream \
		  domain_read domain_write gather_dependent \
		  gather_direct gatherop indexof indexof_output \
		  input_output iterator iterator_dimensions multiple_output \
		  read_write reduce reduce_basic reducetostream \
		  replication_stride struct_gather struct_input \
		  struct_output sum toy scatter scan \
		  domain_streamreadwrite read_write_domain pack_vout vout

#don't remove vout--otherwise it'll break again with some random change


#ifdef BRT_RUNTIME
#ifneq ($(BRT_RUNTIME), arb)
#REGRESSIONDIRS := $(REGRESSIONDIRS) pack_vout vout
#endif
#endif

SUBDIRS := $(REGRESSIONDIRS) streamcopy writeQuery
# big  fft infinite tessellate

include $(ROOTDIR)/common.mk

