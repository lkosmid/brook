ROOTDIR = ../../..
EXECUTABLE = mandelbrot
FILES = main ppmImage mandelbrot
LIBRARIES = $(RUNTIME_LIBS)

include $(ROOTDIR)/common.mk

CFLAGS +=  -O3 -mfpmath=sse -ffast-math 
CFLAGS += -fopenmp 
LDFLAGS +=  -lgomp 

main.cpp: ./built/mandelbrot.cpp
