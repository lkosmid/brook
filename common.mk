#BRCCFLAGS := -m
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

include $(ROOTDIR)/config/$(OS).mk

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

ifdef BINARY
all: subdirs arch
else
all: subdirs
endif

##  Build subdirectories  ##
SUBDIRS_ALL = $(foreach dir, $(SUBDIRS), $(dir).subdir)

subdirs: $(SUBDIRS_ALL)

$(SUBDIRS_ALL):
	@$(MAKE) -C $(basename $@) --no-print-directory

##  Build Binary ##
arch: $(PRECOMP) makedirs dep

recurse: $(BINDIR)/$(BINARY)

builddeps: $(DEPS)

dep: builddeps
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


##  Compile .br files ##
.br.cpp:
	@$(ECHO) $<
	$(ROOTDIR)/bin/brcc$(BINSUFFIX) $(BRCCFLAGS) $<

.PRECIOUS: %.cpp


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

REGRESS_ALL = $(foreach dir, $(REGRESSIONDIRS), $(dir).regress)

regression: $(REGRESS_ALL)

$(REGRESS_ALL):
	@$(MAKE) -C $(basename $@) --no-print-directory regression

else

regression: arch
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
