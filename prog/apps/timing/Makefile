ROOTDIR = ../../..
EXECUTABLE = timing
FILES = main readwrite runkernel blas oneway_matmult_4x4 fourway_matmult_4x4 \
        fft2d qfft reduction wfft fft sparseMat
LIBRARIES = $(RUNTIME_LIBS)
BRCCFLAGS = 
GRAPHDIR = graphs
SLOP = graphs

RUNTIME=nv30gl
RUNSIZE=350
RUNTESTS=runk2d

include $(ROOTDIR)/common.mk

.PRECIOUS: $(OBJDIR)/%.pts

$(GRAPHDIR)/graph-$(RUNTIME).pts: $(DEPS) $(BINDIR)/$(BINARY)
	$(ECHO) "Generating $@"
	@if test ! -d $(GRAPHDIR); then $(MKDIR) $(GRAPHDIR); fi
	env BRT_RUNTIME=$(RUNTIME) $(BINDIR)/$(BINARY) $(RUNSIZE) $(RUNTESTS) > $@ 2>&1

$(GRAPHDIR)/graph-$(RUNTIME).pdf: makedirs $(GRAPHDIR)/graph-$(RUNTIME).pts
	$(ECHO) "Generating $@"
	jgraph -P -comments timing-$(RUNTIME).jgr | ps2pdf - $@

cpu-graph:
	$(MAKE) RUNTIME=cpu $(GRAPHDIR)/graph-cpu.pdf

nv30-graph:
	$(MAKE) RUNTIME=nv30gl $(GRAPHDIR)/graph-nv30gl.pdf

dx9-graph:
	$(MAKE) RUNTIME=dx9 $(GRAPHDIR)/graph-dx9.pdf

arb-graph:
	$(MAKE) RUNTIME=arb $(GRAPHDIR)/graph-arb.pdf
