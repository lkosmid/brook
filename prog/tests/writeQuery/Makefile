ROOTDIR = ../../..
EXECUTABLE = writeQuery
FILES = main writeKernels
LIBRARIES = $(RUNTIME_LIBS)

# The CPU runtime does not currently have support for write queries nor can
# it handle shaders that contain the 'discard' operation.
BRCCFLAGS = -p arb -p fp40 -p fp30 -p ps20 -p ps2a -p ps2b -p ps30

include $(ROOTDIR)/common.mk

main.cpp: ./built/writeKernels.hpp
