
ifndef ROOTDIR
ROOTDIR := .
endif

BIN        := bin
BUILT      := built
INC        := include
BINDIR     := $(ROOTDIR)/$(BIN)
OBJDIR 	   := $(BUILT)
INCLUDEDIR := $(ROOTDIR)/$(INC)

ECHO	 := echo
MKDIR    := mkdir

INCLUDEFILES := $(ROOTDIR)/$(INC)/*.h
MAKEFILEDEPS := $(ROOTDIR)/common.mk Makefile $(INCLUDEFILES)

include $(ROOTDIR)/config/$(OS).mk

CFLAGS    += $(C_INCLUDE_FLAG)$(INCLUDEDIR) $(C_DEBUG_FLAG)
LDFLAGS   += $(LD_LIBDIR_FLAG)$(ROOTDIR)/$(BIN)

TEMP      := $(addprefix $(LD_LIBLINK_PREFIX), $(LIBRARIES))
LDFLAGS   += $(addsuffix $(LD_LIBLINK_SUFFIX), $(TEMP))

LDFLAGS   += $(LD_DEBUG_FLAG)

OBJS      := $(addprefix $(OBJDIR)/, $(FILES))
OBJS      := $(addsuffix $(OBJSUFFIX), $(OBJS))

ifdef STATIC_LIBRARY
BINARY_NAME := $(STATIC_LIBRARY)
BINARY   := $(addprefix $(LIBPREFIX), $(STATIC_LIBRARY))
BINARY   := $(addsuffix $(STATIC_LIBSUFFIX), $(BINARY))
CFLAGS   := $(C_STATIC_FLAG) $(CFLAGS)
LDFLAGS  := $(LD_STATIC_FLAG) $(LDFLAGS)
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
BINARY   := $(addsuffix $(BINSUFFIX), $(EXECUTABLE))
endif
endif
endif

ifndef SUBDIRS

all: arch

else

SUBDIRS_ALL = $(foreach dir, $(SUBDIRS), $(dir).subdir)

subdirs: $(SUBDIRS_ALL)

$(SUBDIRS_ALL):
	@$(MAKE) -C $(basename $@) --no-print-directory

endif

FRAGMENT_PROGRAMS += $(addsuffix .fp, $(BROOK_FILES))
FRAGMENT_PROGRAMS += $(addsuffix .fp, $(CG_FILES))

arch: $(PRECOMP) makedirs $(BINDIR)/$(BINARY) $(FRAGMENT_PROGRAMS)

makedirs:
	@if test ! -d $(OBJDIR); then $(MKDIR) $(OBJDIR); fi
	@if test ! -d $(BINDIR); then $(MKDIR) $(BINDIR); fi


.SUFFIXES : $OBJSUFFIX .c .cpp .cg .br .fp
.PRECIOUS : reduce.cg

$(OBJDIR)/%$(OBJSUFFIX): %.c
	@$(CC) $(CFLAGS) $(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

$(OBJDIR)/%$(OBJSUFFIX): %.cpp
	@$(CC) $(CFLAGS)$(C_OUTPUT_FLAG)$@ $(C_COMPILE_FLAG) $<

.br.cg:
	@brcc $* < $<

.cg.fp:
	@cgc -profile fp30 -o $@ $<

$(BINDIR)/$(BINARY): $(OBJS) $(ADDITIONAL_DEPENDANCIES)
	@$(ECHO) Building $@
ifdef STATIC_LIBRARY
	@$(AR) $(ARFLAGS) $(AR_OUTPUT_FLAG)$@ $(OBJS)
else
	@$(LD) $(LDFLAGS) $(LD_OUTPUT_FLAG)$@ $(OBJS) $(LD_LIBRARIES_LINK)
endif


clean:
ifdef SUBDIRS
	@for i in $(SUBDIRS); do $(MAKE) --no-print-directory -C $$i clean; done
else
	@rm -rf $(OBJDIR) $(BINDIR)/$(BINARY) $(SLOP) *.proto.cpp $(BINDIR)/$(BINARY_NAME).pdb *~ *MCTEMPFILE* $(FRAGMENT_PROGRAMS)
endif


















