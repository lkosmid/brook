ROOTDIR := ../..

include $(ROOTDIR)/config/DetectOS.mk

REGRESSIONDIRS := accumulate allocate cachetest constant$(MOBILEPROFILE) convert_iter_to_stream \
		  domain_read domain_write domain_gather$(MOBILEPROFILE) gather_dependent$(MOBILEPROFILE) \
		  gather_direct$(MOBILEPROFILE) gatherop indexof indexof_output \
		  input_output iterator iterator_dimensions$(MOBILEPROFILE) multiple_output$(MOBILEPROFILE) \
		  multibackend read_write reduce_basic$(MOBILEPROFILE) reduce_simplest reduce_simplest_nocopy reducetostream\
		  replication_stride struct_gather struct_input \
		  struct_output sum toy scatter scan$(MOBILEPROFILE) \
		  domain_streamreadwrite read_write_domain$(MOBILEPROFILE)

#don't remove vout--otherwise it'll break again with some random change


#ifdef BRT_RUNTIME
#ifneq ($(BRT_RUNTIME), arb)
#REGRESSIONDIRS := $(REGRESSIONDIRS) pack_vout vout
#endif
#endif

SUBDIRS := $(REGRESSIONDIRS) streamcopy$(MOBILEPROFILE) #writeQuery
# big  fft infinite tessellate

include $(ROOTDIR)/common.mk

