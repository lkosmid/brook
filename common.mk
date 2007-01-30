# ROOTDIR reflects the root of the current source tree.  It determines where
# built binaries are left.
ifndef ROOTDIR
ROOTDIR := .
endif

# BROOKDIR reflects where the root of a brook tree can be found.
# Brook-using applications outside the main brook source tree can define
# BROOKDIR and include this common.mk and everything should work.
ifndef BROOKDIR
BROOKDIR := $(ROOTDIR)
endif

BIN        := bin
BUILT      := built
INC        := include
DEP        := depends
BINDIR     := $(ROOTDIR)/$(BIN)
OBJDIR 	   := $(BUILT)
INCLUDEDIR := $(BROOKDIR)/$(INC)
DEPDIR     := $(DEP)
FASTDEPS   := $(BROOKDIR)/scripts/fastdep.pl

ECHO	 := echo
MKDIR    := mkdir

INCLUDEFILES := $(BROOKDIR)/$(INC)/*.h
MAKEFILEDEPS := $(BROOKDIR)/common.mk Makefile $(INCLUDEFILES)

include $(BROOKDIR)/config/DetectOS.mk
include $(BROOKDIR)/config/$(OS).mk

BRCC = $(BROOKDIR)/bin/brcc$(BINSUFFIX)

CFLAGS    += $(C_INCLUDE_FLAG). $(C_INCLUDE_FLAG)$(INCLUDEDIR)

ifdef EXTRAINCLUDEDIR
TEMPINC := $(addprefix $(C_INCLUDE_FLAG), $(EXTRAINCLUDEDIR))
CFLAGS  += $(TEMPINC)
endif

ifdef RELEASE
CFLAGS    +=  $(C_RELEASE_FLAG) $(SPACE)
else
CFLAGS    +=  $(C_DEBUG_FLAG) $(SPACE)
endif

LDFLAGS   += $(LD_LIBDIR_FLAG)$(BROOKDIR)/$(BIN)

TEMP1     := $(addprefix $(LD_LIBLINK_PREFIX), $(LIBRARIES))
LDFLAGS   += $(addsuffix $(LD_LIBLINK_SUFFIX), $(TEMP1))

LDFLAGS   += $(LD_DEBUG_FLAG)

OBJS      := $(addprefix $(OBJDIR)/, $(FILES))
OBJS      := $(addsuffix $(OBJSUFFIX), $(OBJS))

DEPS      := $(FILES)
DEPS      := $(addprefix $(DEPDIR)/, $(DEPS))
DEPS      := $(addsuffix .depend, $(DEPS))

ifdef STATIC_LIBRARY
BINARY_NAME := $(STATIC_LIBRARY)
BINARY   := $(addprefix $(LIBPREFIX), $(STATIC_LIBRARY))
BINARY   := $(addsuffix $(STATIC_LIBSUFFIX), $(BINARY))
CFLAGS   := $(C_STATIC_FLAG) $(CFLAGS)
else
ifdef SHARED_LIBRARY
BINARY_NAME := $(SHARED_LIBRARY)
BINARY   := $(addprefix $(LIBPREFIX), $(SHARED_LIBRARY))
BINARY   := $(addsuffix $(SHARED_LIBSUFFIX), $(BINARY))
CFLAGS   := $(C_SHARED_FLAG) $(CFLAGS)
LDFLAGS  := $(LD_SHARED_FLAG) $(LDFLAGS) 
else
ifdef EXECUTABLE
BINARY_NAME := $(EXECUTABLE)
BINARY      := $(addsuffix $(BINSUFFIX), $(EXECUTABLE))
TEMP3       := $(addprefix $(LD_LIBLINK_PREFIX), $(SYSTEM_LIBS))
LDFLAGS     += $(addsuffix $(LD_LIBLINK_SUFFIX), $(TEMP3))
endif
endif
endif

SUBDIRS_RELINK := $(addsuffix .relink, $(SUBDIRS))

all: $(SUBDIRS)
relink: $(SUBDIRS_RELINK)
ifdef BINARY
all: arch
relink:
	rm -rf $(BINDIR)/$(BINARY)
	$(MAKE)
endif

release:
	$(MAKE) RELEASE=1

ifdef EXECUTABLE
run:
	$(BINDIR)/$(BINARY) $(args)
endif

%.relink:
	$(MAKE) -C $* --no-print-directory relink

$(SUBDIRS):
	$(MAKE) -C $@ --no-print-directory

##  Build Binary ##
arch: $(PRECOMP) makedirs dep $(POSTCOMP)

recurse: $(BINDIR)/$(BINARY)

dep: $(DEPS)
	@$(MAKE) --no-print-directory recurse INCLUDEDEPS=1

##  Make directories for build files ##
makedirs:
	@if test ! -d $(OBJDIR); then $(MKDIR) $(OBJDIR); fi
	@if test ! -d $(BINDIR); then $(MKDIR) $(BINDIR); fi
	@if test ! -d $(DEPDIR); then $(MKDIR) $(DEPDIR); fi

.SUFFIXES : $(OBJSUFFIX) .c .cpp .br


## Include dependencies
ifdef INCLUDEDEPS
include $(DEPS)
else
## Make dependencies
$(DEPDIR)/%.depend: %.c
	@$(MAKE_DEPDIR)
	@$(ECHO) "Rebuilding dependencies for $<"
	@$(PERL) $(FASTDEPS) -I. -I$(INCLUDEDIR) --obj-suffix='$(OBJSUFFIX)' --obj-prefix='$(OBJDIR)/' $< > $@

$(DEPDIR)/%.depend: %.cpp
	@$(MAKE_DEPDIR)
	@$(ECHO) "Rebuilding dependencies for $<"
	@$(PERL) $(FASTDEPS) -I. -I$(INCLUDEDIR) --obj-suffix='$(OBJSUFFIX)' --obj-prefix='$(OBJDIR)/' $< > $@

$(DEPDIR)/%.depend: %.br
	@$(ECHO) "Rebuilding dependencies for $<"
	@$(PERL) $(FASTDEPS) -I. -I$(INCLUDEDIR) --obj-suffix='$(OBJSUFFIX)' --obj-prefix='$(OBJDIR)/' $< > $@

$(DEPDIR)/%.depend: %.bri
	@$(ECHO) "Rebuilding dependencies for $<"
	@$(PERL) $(FASTDEPS) -I. -I$(INCLUDEDIR) --obj-suffix='$(OBJSUFFIX)' --obj-prefix='$(OBJDIR)/' $< > $@
endif

##  Compile .brhi files  ##
$(OBJDIR)/%.hpp: %.brhi
	$(CC) $(C_CPP_FLAG) $< > $(OBJDIR)/$*.brh
	$(BRCC) $(BRCCFLAGS) -o $(OBJDIR)/$* $(OBJDIR)/$*.brh


##  Compile .brh files  ##
$(OBJDIR)/%.hpp: %.brh
ifndef COMPILER_ECHOES
	@$(ECHO) $<
endif
	$(BRCC) $(BRCCFLAGS) -o $(OBJDIR)/$* $<

##  Compile .c files  ##
$(OBJDIR)/%$(OBJSUFFIX): %.c
ifndef COMPILER_ECHOS
	@$(ECHO) $<
endif
	$(CC) $(CFLAGS)$(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

##  Compile .cpp files ##
$(OBJDIR)/%$(OBJSUFFIX): %.cpp
ifndef COMPILER_ECHOS
	@$(ECHO) $<
endif
	$(CC) $(CFLAGS)$(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

$(OBJDIR)/%$(OBJSUFFIX): $(OBJDIR)/%.cpp
ifndef COMPILER_ECHOS
	@$(ECHO) $<
endif
	$(CC) $(CFLAGS)$(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

##  Compile .bri files ##
$(OBJDIR)/%.cpp: %.bri
	$(CC) $(C_CPP_FLAG) $< > $(OBJDIR)/$*.br
ifndef COMPILER_ECHOES
	@$(ECHO) $*.br
endif
	$(BRCC) $(BRCCFLAGS) -o $(OBJDIR)/$* $(OBJDIR)/$*.br

##  Keep the generated .br files.
.PRECIOUS: $(OBJDIR)/%.br

##  Compile .br files ##
$(OBJDIR)/%.cpp: %.br
ifndef COMPILER_ECHOES
	@$(ECHO) $<
endif
	$(BRCC) $(BRCCFLAGS) -o $(OBJDIR)/$* $<

##  Keep the generated .cpp files.
.PRECIOUS: $(OBJDIR)/%.cpp

##  Link  ##
$(BINDIR)/$(BINARY):  $(ADDITIONAL_DEPENDANCIES) $(OBJS)
	@$(ECHO) Building $@
ifdef STATIC_LIBRARY
	$(AR) $(ARFLAGS) $(AR_OUTPUT_FLAG)$@ $(OBJS)
	$(RANLIB) $@
else
	$(LD) $(LD_OUTPUT_FLAG)$@ $(OBJS) $(LDFLAGS)
endif

## Clean BRCC generated .cpp/.hpp files ##
BR_FILES_CLEAN   = $(addsuffix .br-clean, $(FILES))
%.br-clean:
	if test -f $*.br; then rm -rf $*.cpp; fi	
	if test -f $*.brh; then rm -rf $*.hpp; fi	

## Clean
clean: $(BR_FILES_CLEAN)
ifdef SUBDIRS
	for i in $(SUBDIRS); do $(MAKE) --no-print-directory -C $$i clean; done
endif
ifdef BINARY
	rm -rf $(OBJDIR) $(BINDIR)/$(BINARY)
	rm -rf $(BINDIR)/$(BINARY_NAME).pdb $(BINDIR)/$(BINARY_NAME).map
	rm -rf $(DEPDIR)
	rm -rf $(BINARY_NAME).output
endif
	rm -rf *~ *.pdb .#* #*
	rm -rf $(SLOP)

## Regression testing ##

ifdef REGRESSIONDIRS

regression:
	@for i in $(REGRESSIONDIRS); do \
		make --no-print-directory -C $$i regression || k=$$k" "$$i; \
	done; test -z "$$k" || echo "FAILED: "$$k; test -z "$$k"
else

regression: arch $(BINARY_NAME).gold
	@echo  "Running $(BINARY_NAME)"
	@$(BINDIR)/$(BINARY) > $(BINARY_NAME).output
	@diff -q -w $(BINARY_NAME).output $(BINARY_NAME).gold
	@rm $(BINARY_NAME).output
	@echo
	@echo  "****************************"
	@echo  "  $(BINARY_NAME) Test Passed"
	@echo  "****************************"
	@echo

endif

ifndef VERBOSE
.SILENT:
endif

.PHONY: regression clean arch makedirs dep $(SUBDIRS)
