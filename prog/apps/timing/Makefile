ROOTDIR = ../../..
EXECUTABLE = timing
FILES = main readwrite runkernel blas oneway_matmult_4x4 fourway_matmult_4x4 \
        fft fft2d fftopt
LIBRARIES = $(RUNTIME_LIBS)
GRAPHDIR = graphs
SLOP = graphs

RUNTIME=cpu
RUNSIZE=1024
RUNTESTS=rw runk

include $(ROOTDIR)/common.mk

graph: arch
	@if test ! -d $(GRAPHDIR); then $(MKDIR) $(GRAPHDIR); fi
	env BRT_RUNTIME=$(RUNTIME) $(BINDIR)/$(BINARY) $(RUNSIZE) $(RUNTESTS) > $(GRAPHDIR)/graph-$(RUNTIME).pts 2>&1
	jgraph -P -comments timing-$(RUNTIME).jgr | ps2pdf - graphs/graph-$(RUNTIME).pdf

cpu-graph:
	$(MAKE) RUNTIME=cpu graph

nv30-graph:
	$(MAKE) RUNTIME=nv30gl graph

dx9-graph:
	$(MAKE) RUNTIME=dx9 graph
