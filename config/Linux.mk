## Linux settings

CPP              := cpp 
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

CFLAGS           += -DBUILD_OGL
#CFLAGS           += -fopenmp 
C_INCLUDE_FLAG   := -I  
C_DEBUG_FLAG     := -g3 -msse -Wall 
C_RELEASE_FLAG   := -O3 -msse -funroll-all-loops  -fexpensive-optimizations -ffast-math -finline-functions -frerun-loop-opt 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := -o 
C_COMPILE_FLAG   := -c 

LDFLAGS           += -lpthread -L/usr/X11R6/lib 
#LDFLAGS           += -lgomp 
LD_LIBDIR_FLAG    := -L
LD_SHARED_FLAG    := -shared   
LD_OUTPUT_FLAG    := -o 
LD_LIBLINK_PREFIX := -l
LD_LIBLINK_SUFFIX := 
LD_DEBUG_FLAG     := 

LIBRARIES        +=
ARFLAGS		 := -rc 
AR_OUTPUT_FLAG	 := 

RUNTIME_LIBS     := brook ogl cpu gpu X11 Xt GL GLU GLEE pthread

RANLIB           := ranlib
