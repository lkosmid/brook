##  MS Vis Studio settings

COMPILER_ECHOS   := 1

CC               := cl
LD               := link
AR               := lib
PERL             := perl
OBJSUFFIX        := .obj
LIBPREFIX        := 
SHARED_LIBSUFFIX := .dll
STATIC_LIBSUFFIX := .lib
BINSUFFIX        := .exe

SYSTEM_LIBS      :=  kernel32 gdi32 user32 opengl32 d3dx9 d3d9 advapi32

# /w34505: Enabled warning 4505 (unreferenced static function) at level 3
CFLAGS           += /nologo /MDd /W3 /w34505 /DWIN32 /DWINDOWS /EHsc /Zm500 /I /DX90SDK/INCLUDE

ifndef I_AM_SLOPPY
# /WX: Make warnings fatal
CFLAGS           += /WX
endif

C_INCLUDE_FLAG   := /I
C_DEBUG_FLAG     := /Z7 /Yd /GZ 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := /Fo
C_COMPILE_FLAG   := /c

LDFLAGS           += /nologo /map /fixed:no /libpath:/DX90SDK/LIB
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
