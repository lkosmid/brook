##  MS Vis Studio settings

CC               := cl
LD               := link
AR               := lib
OBJSUFFIX        := .obj
LIBPREFIX        := 
SHARED_LIBSUFFIX := .dll
STATIC_LIBSUFFIX := .lib
BINSUFFIX        := .exe

#CFLAGS           += /nologo /MDd /W1 /O2 /DWIN32
CFLAGS           += /nologo /MDd /W3 /DWIN32
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
LD_DEBUG_FLAG     := /debug /debugtype:both
LD_DEF_FLAG       := /def:
LIBRARIES         += kernel32 gdi32 user32 opengl32

ARFLAGS		 := /nologo /debugtype:both
AR_OUTPUT_FLAG	 := /out:



