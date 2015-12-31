ROOTDIR = ../../..
include $(ROOTDIR)/config/DetectOS.mk
EXECUTABLE = benchmark$(MOBILEPROFILE)
FILES = main benchmark$(MOBILEPROFILE)
LIBRARIES = $(RUNTIME_LIBS)

include $(ROOTDIR)/common.mk

CFLAGS +=  -O3 -mfpmath=sse -ffast-math 
CFLAGS += -fopenmp 
LDFLAGS +=  -lgomp 

main.cpp: ./built/benchmark$(MOBILEPROFILE).cpp
