ROOTDIR = ../../..
EXECUTABLE = timing
FILES = main readwrite runkernel blas sparse
LIBRARIES = $(RUNTIME_LIBS)
GRAPHDIR = graphs
SLOP = graphs

RUNTIME=CPU_RUNTIME
RUNSIZE=1024

include $(ROOTDIR)/common.mk

graph: arch
	@if test ! -d $(GRAPHDIR); then $(MKDIR) $(GRAPHDIR); fi
	env BRT_RUNTIME=$(RUNTIME) $(BINDIR)/$(BINARY) $(RUNSIZE) > $(GRAPHDIR)/graph-$(RUNTIME).jgr

cpu-graph:
	$(MAKE) RUNTIME=CPU_RUNTIME graph

nv30-graph:
	$(MAKE) RUNTIME=nv30gl graph

dx9-graph:
	$(MAKE) RUNTIME=dx9 graph
