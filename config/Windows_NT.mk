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

#CFLAGS           += /nologo /MDd /W1 /O2 /DWIN32
CFLAGS           += /nologo /MDd /W3 /DWIN32 /DWINDOWS /EHsc
C_INCLUDE_FLAG   := /I
C_DEBUG_FLAG     := /Z7 /Yd /GZ 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := /Fo
C_COMPILE_FLAG   := /c

LDFLAGS           += /nologo /map /fixed:no
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


