ROOTDIR := ../..

include $(ROOTDIR)/config/DetectOS.mk

REGRESSIONDIRS := accumulate allocate constant$(MOBILEPROFILE) convert_iter_to_stream \
		  domain_read domain_write gather_dependent$(MOBILEPROFILE) \
		  gather_direct$(MOBILEPROFILE) gatherop indexof indexof_output \
		  input_output iterator iterator_dimensions multiple_output \
		  multibackend read_write reduce reduce_basic reducetostream \
		  replication_stride struct_input \
		  struct_output sum toy scatter scan \
		  domain_streamreadwrite read_write_domain pack_vout vout
#gatherop indexof struct_gather 

#don't remove vout--otherwise it'll break again with some random change


#ifdef BRT_RUNTIME
#ifneq ($(BRT_RUNTIME), arb)
#REGRESSIONDIRS := $(REGRESSIONDIRS) pack_vout vout
#endif
#endif

SUBDIRS := $(REGRESSIONDIRS) streamcopy$(MOBILEPROFILE) #writeQuery
# big  fft infinite tessellate

include $(ROOTDIR)/common.mk

