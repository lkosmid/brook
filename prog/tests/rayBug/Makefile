ROOTDIR = ../../..
EXECUTABLE = rayBug
FILES = main kernels
LIBRARIES = $(RUNTIME_LIBS)

# The CPU runtime does not currently have support for write queries nor can
# it handle shaders that contain the 'discard' operation.
BRCCFLAGS = -y -p ps2a

include $(ROOTDIR)/common.mk

main.cpp: ./built/kernels.hpp
