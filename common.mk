BRCCFLAGS := -m
ifndef ROOTDIR
ROOTDIR := .
endif

BIN        := bin
BUILT      := built
INC        := include
DEP        := depends
BINDIR     := $(ROOTDIR)/$(BIN)
OBJDIR 	   := $(BUILT)
INCLUDEDIR := $(ROOTDIR)/$(INC)
DEPDIR     := $(DEP)
FASTDEPS   := $(ROOTDIR)/scripts/fastdep.pl

ECHO	 := echo
MKDIR    := mkdir

INCLUDEFILES := $(ROOTDIR)/$(INC)/*.h
MAKEFILEDEPS := $(ROOTDIR)/common.mk Makefile $(INCLUDEFILES)

ifdef OS
include $(ROOTDIR)/config/$(OS).mk
else
# Linux.mk is a misnomer.  Really Linux.mk should build on many GNU
# toolchain using systems and OS isn't a builtin variable on my (or other
# that I've tested) Linux versions of make so we just use it as a default.
# --Jeremy.

include $(ROOTDIR)/config/Linux.mk
endif

CFLAGS    += $(C_INCLUDE_FLAG). $(C_INCLUDE_FLAG)$(INCLUDEDIR) $(C_DEBUG_FLAG)
LDFLAGS   += $(LD_LIBDIR_FLAG)$(ROOTDIR)/$(BIN)

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

all: $(SUBDIRS)
ifdef BINARY
all: arch
endif

ifdef EXECUTABLE
run:
	 $(BINDIR)/$(BINARY)
endif

$(SUBDIRS):
	$(MAKE) -C $@ --no-print-directory

##  Build Binary ##
arch: $(PRECOMP) makedirs dep

recurse: $(BINDIR)/$(BINARY)


dep: $(DEPS)
	@$(MAKE) --no-print-directory recurse INCLUDEDEPS=1

##  Make directories for build files ##
makedirs:
	@if test ! -d $(OBJDIR); then $(MKDIR) $(OBJDIR); fi
	@if test ! -d $(BINDIR); then $(MKDIR) $(BINDIR); fi
	@if test ! -d $(DEPDIR); then $(MKDIR) $(DEPDIR); fi

.SUFFIXES : $(OBJSUFFIX) .c .cpp .br

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

## Include dependencies
ifdef INCLUDEDEPS
include $(DEPS)
endif

##  Compile .c files  ##
$(OBJDIR)/%$(OBJSUFFIX): %.c
ifndef COMPILER_ECHOS
	@$(ECHO) $<
endif
	$(CC) $(CFLAGS) $(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

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

##  Compile .br files ##
$(OBJDIR)/%.cpp: %.br
ifdef COMPILER_ECHOES
	@$(ECHO) $<
endif
	$(ROOTDIR)/bin/brcc$(BINSUFFIX) $(BRCCFLAGS) -o $(OBJDIR)/$* $<

##  Keep the generated .cpp files.
.PRECIOUS: $(OBJDIR)/%.cpp

##  Link  ##
$(BINDIR)/$(BINARY):  $(ADDITIONAL_DEPENDANCIES) $(OBJS)
	@$(ECHO) Building $@
ifdef STATIC_LIBRARY
	$(AR) $(ARFLAGS) $(AR_OUTPUT_FLAG)$@ $(OBJS)
else
	$(LD) $(LD_OUTPUT_FLAG)$@ $(OBJS) $(LDFLAGS)
endif

## Clean BRCC generated .cpp files ##
BR_FILES_CLEAN   = $(addsuffix .br-clean, $(FILES))
%.br-clean:
	@if test -f $*.br; then rm -rf $*.cpp; fi	

## Clean
clean: $(BR_FILES_CLEAN)
ifdef SUBDIRS
	@for i in $(SUBDIRS); do $(MAKE) --no-print-directory -C $$i clean; done
endif
ifdef BINARY
	@rm -rf $(OBJDIR) $(BINDIR)/$(BINARY) $(SLOP) 
	@rm -rf $(BINDIR)/$(BINARY_NAME).pdb *~ $(DEPDIR)
	@rm -rf $(BINARY_NAME).output
endif

## Regression testing ##

ifdef REGRESSIONDIRS

regression:
	@for i in $(REGRESSIONDIRS); do \
		$(MAKE) --no-print-directory -C $$i regression; \
	done

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
