## Linux settings

CC               := g++ 
LD               := g++
AR               := ar
PERL             := perl
OBJSUFFIX        := .o
LIBPREFIX        := lib
SHARED_LIBSUFFIX := .so
STATIC_LIBSUFFIX := .a
BINSUFFIX        := 


SYSTEM_LIBS      := 

CFLAGS           += 
C_INCLUDE_FLAG   := -I  
C_DEBUG_FLAG     := -g3 -Wall 
#-O3 -funroll-all-loops  -fexpensive-optimizations -ffast-math -finline-functions -finline-limit=10000 -frerun-loop-opt 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := -o 
C_COMPILE_FLAG   := -c 

LDFLAGS           += -lpthread -L/usr/X11R6/lib 
LD_LIBDIR_FLAG    := -L
LD_SHARED_FLAG    := -shared   
LD_OUTPUT_FLAG    := -o 
LD_LIBLINK_PREFIX := -l
LD_LIBLINK_SUFFIX := 
LD_DEBUG_FLAG     := 

LIBRARIES        +=
ARFLAGS		 := -rc 
AR_OUTPUT_FLAG	 := 

RUNTIME_LIBS     := brook cpu nv30gl X11 Xmu Xt GL GLU

RANLIB           := ranlib
