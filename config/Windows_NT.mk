##  MS Vis Studio settings

#COMPILER_ECHOS   := 1

CC               := cl
LD               := link
AR               := link /lib
PERL             := perl
OBJSUFFIX        := .obj
LIBPREFIX        := 
SHARED_LIBSUFFIX := .dll
STATIC_LIBSUFFIX := .lib
BINSUFFIX        := .exe
PLATFORM         := win32
# INCLUDEDEPS	 := 1

SYSTEM_LIBS      :=  kernel32 gdi32 user32 opengl32 d3dx9 d3d9 advapi32 Winmm

# /w34505: Enabled warning 4505 (unreferenced static function) at level 3
# I turned it off for now since gcc should pick this up.

CFLAGS   += /nologo /W3 /DBUILD_OGL /DBUILD_DX9 /DWIN32 /DWINDOWS /EHsc /Zm500 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE
#ifdef BUILD_CTM
#CFLAGS   += /DBUILD_CTM
#endif

ifndef I_AM_SLOPPY
# /WX: Make warnings fatal
CFLAGS           += /WX 
endif

C_CPP_FLAG	 := /nologo /EP
C_INCLUDE_FLAG   := /I
C_DEBUG_FLAG     := /Zi /RTC1 /EHsc /Fd$(OBJDIR)\\
#C_RELEASE_FLAG   := /ML /Ogisyb2 /Gs /arch:SSE2 /G7 /DNDEBUG  /Fd$(OBJDIR)\\
C_RELEASE_FLAG   := /O2 /arch:SSE2 /EHsc /DNDEBUG /Fd$(OBJDIR)\\
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := /Fo
C_COMPILE_FLAG   := /c

LDFLAGS           += /nologo /map /fixed:no /INCREMENTAL:NO #/NODEFAULTLIB:LIBCMT
LD_LIBDIR_FLAG    := /libpath:
LD_SHARED_FLAG    := /DLL
LD_OUTPUT_FLAG    := /out:
LD_LIBLINK_PREFIX :=  
LD_LIBLINK_SUFFIX := .lib 
LD_DEBUG_FLAG     := /debug
LD_DEF_FLAG       := /def:

ARFLAGS		  := /nologo
AR_OUTPUT_FLAG	  := /out:
AR_LIBDIR_FLAG    := /libpath:
AR_LIBLINK_PREFIX :=  
AR_LIBLINK_SUFFIX := .lib 


ARFLAGS   += $(AR_LIBDIR_FLAG)$(ROOTDIR)/$(BIN)
TEMP2     := $(addprefix $(AR_LIBLINK_PREFIX), $(LIBRARIES))
ARFLAGS   += $(addsuffix $(AR_LIBLINK_SUFFIX), $(TEMP2))
RUNTIME_LIBS  := brook
RANLIB    := true 

ifdef DXSDK_DIR
CFLAGS += $(C_INCLUDE_FLAG)"$(DXSDK_DIR)/Include"
LDFLAGS += $(LD_LIBDIR_FLAG)"$(DXSDK_DIR)/Lib/$(PROCESSOR_ARCHITECTURE)"
endif

