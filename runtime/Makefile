ROOTDIR = ..
STATIC_LIBRARY = brook
FILES = brt

LIBRARIES := cpu
SUBDIRS := cpu

ifeq ($(OS), Windows_NT)

ADDITIONAL_DEPENDANCIES +=  nv30gl.runtime
LIBRARIES += dx9 nv30gl
SUBDIRS += dx9 nv30gl

endif

include ../common.mk

nv30gl.runtime: $(BINDIR)/nv30gl.lib



