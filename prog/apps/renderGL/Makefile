ROOTDIR = ../../..
EXECUTABLE = renderGL
FILES = main renderGL renderWindow simulationKernel
SUBDIRS = fibble
LIBRARIES = $(RUNTIME_LIBS) fibble

include $(ROOTDIR)/common.mk

renderWindow.cpp: ./built/simulationKernel.hpp

