## Linux settings

CC               := g++ 
LD               := g++
AR               := ar
OBJSUFFIX        := .o
LIBPREFIX        := lib
SHARED_LIBSUFFIX := .so
STATIC_LIBSUFFIX := .a
BINSUFFIX        := 

CFLAGS           += 
C_INCLUDE_FLAG   := -I 
C_DEBUG_FLAG     := -g 
C_STATIC_FLAG    := 
C_OUTPUT_FLAG    := -o 
C_COMPILE_FLAG   := -c 

LDFLAGS           += 
LD_LIBDIR_FLAG    := -L  
LD_SHARED_FLAG    := -shared   
LD_OUTPUT_FLAG    := -o 
LD_LIBLINK_PREFIX := -l 
LD_LIBLINK_SUFFIX :=  
LD_DEBUG_FLAG     := 

LIBRARIES         += GL m c 

ARFLAGS		 := -rc 
AR_OUTPUT_FLAG	 := 

