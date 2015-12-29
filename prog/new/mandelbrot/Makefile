ROOTDIR = ../../..
EXECUTABLE = mandelbrot
FILES = main ppmImage mandelbrot
LIBRARIES = $(RUNTIME_LIBS)

include $(ROOTDIR)/common.mk

CFLAGS +=  -O3 -ffast-math #-mfpmath=sse 
#CFLAGS += -fopenmp 
#LDFLAGS +=  -lgomp 

main.cpp: ./built/mandelbrot.cpp
