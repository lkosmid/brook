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

CFLAGS           += -DBUILD_GLES -DRPI_NO_X
#CFLAGS           += -fopenmp 
C_CPP_FLAG	 := -E
C_INCLUDE_FLAG   := -I
C_DEBUG_FLAG     := -g3 -Wall 
C_RELEASE_FLAG   := -O3 -funroll-all-loops  -fexpensive-optimizations -ffast-math -finline-functions -frerun-loop-opt 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := -o 
C_COMPILE_FLAG   := -c 

GLES_INCLUDE_DIR :=/opt/vc/include
EGL_INCLUDE_DIR  :=/opt/vc/include
GLES_LD_DIR      :=/usr/lib/intel-cdv/
EGL_LD_DIR       :=/usr/lib/intel-cdv/

CFLAGS           += $(C_INCLUDE_FLAG)$(GLES_INCLUDE_DIR) $(C_INCLUDE_FLAG)$(EGL_INCLUDE_DIR) $(C_INCLUDE_FLAG)/opt/vc/include $(C_INCLUDE_FLAG)/opt/vc/include/interface/vcos/pthreads $(C_INCLUDE_FLAG)/opt/vc/include/interface/vmcs_host/linux

LDFLAGS           += -lpthread -L/usr/X11R6/lib -L$(GLES_LD_DIR) -L$(EGL_LD_DIR) -lGLESv2 -lEGL -lm -lbcm_host -L/opt/vc/lib
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

RUNTIME_LIBS     := brook gles cpu gpu X11 GLESv2 EGL pthread

RANLIB           := ranlib
#Compile all benchmarks for gles and cpu backends
BRCCFLAGS        += -k -p gles -p cpu
