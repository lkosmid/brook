// brt.cpp
#include "runtime.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <float.h>
#ifdef BUILD_DX9
#include "dx9/dx9.hpp"
#endif

#ifdef BUILD_NV30GL
#include "nv30gl/nv30gl.hpp"
#endif

#include "cpu/cpu.hpp"
#include "brtscatterintrinsic.hpp"

__StreamScatterAssign STREAM_SCATTER_ASSIGN;
__StreamScatterAdd STREAM_SCATTER_ADD;
__StreamScatterMul STREAM_SCATTER_MUL;
__StreamGatherInc STREAM_GATHER_INC;
__StreamGatherFetch STREAM_GATHER_FETCH;
inline float finite_flt (float x) {
#ifdef _WIN32
   return (float) _finite(x);
#else
#ifdef __APPLE__
   return (float) __isfinitef(x);
#else
   return (float) finite(x);
#endif
#endif
}
inline float isnan_flt (float x) {
#ifdef _WIN32
   return (float) _isnan(x);
#else
#ifdef __APPLE__
   return (float) __isnanf(x);
#else
   return (float) isnan(x);
#endif
#endif
}

namespace brook {

  static const char* RUNTIME_ENV_VAR = "BRT_RUNTIME";
    
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  RunTime* RunTime::GetInstance() {
    static RunTime* sResult = CreateInstance();
    return sResult;
  }

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  RunTime* RunTime::CreateInstance() {
    char *env = getenv(RUNTIME_ENV_VAR);

    if (!env) {
      fprintf (stderr,"No runtime requested. Using CPU\n");
      return new CPURunTime();
    }

#ifdef BUILD_DX9
    if (!strcmp(env, DX9_RUNTIME_STRING))
    {
      RunTime* result = DX9RunTime::create();
      if( result != NULL ) return result;
      fprintf(stderr, 
	      "Unable to initialize DX9 runtime, falling back to CPU\n");
      return new CPURunTime();
    }
#endif

#ifdef BUILD_NV30GL
    if (!strcmp(env, NV30GL_RUNTIME_STRING))
      return new NV30GLRunTime();
#endif

    if (strcmp(env,CPU_RUNTIME_STRING)) 
      fprintf (stderr, 
	       "Unknown runtime requested: %s falling back to CPU\n", env);
    return new CPURunTime();
  }
  void StreamInterface::readItem (void * output, unsigned int * index){
     unsigned int linearindex = index[0];
     unsigned int dim = getDimension();
     const unsigned int * extents = getExtents();
     for (unsigned int i=1;i<dim;++i) {
        linearindex*=extents[i];
        linearindex+=index[i];
     }
     __BRTStreamType type = getStreamType();
     unsigned int size;
     switch (type) {
     case __BRTFLOAT:
     case __BRTFLOAT2:
     case __BRTFLOAT3:
     case __BRTFLOAT4:
        size=type*sizeof(float);
     default:    
        size=sizeof(float);
     }
     char * data = (char*)getData(READ);
     data+=linearindex*size;
     memcpy (output,data,size);
     releaseData(READ);
  }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
__BRTStream::__BRTStream(__BRTStreamType type, ...)
  : stream(NULL)
{
  std::vector<int> extents;

  va_list args;
  va_start(args,type);
  for(;;)
  {
    int extent = va_arg(args,int);
    if( extent == -1 ) break;
    extents.push_back(extent);
  }
  va_end(args);

  stream = brook::RunTime::GetInstance()->CreateStream( type, (int)extents.size(), &extents[0] );
}
__BRTStream::__BRTStream(int * extents,int dims,__BRTStreamType type) {
   stream = brook::RunTime::GetInstance()->CreateStream(type,dims,extents);
}
void streamPrint(brook::StreamInterface * s, bool flatten) {
   unsigned int dims = s->getDimension();
   const unsigned int * extent = s->getExtents();
   unsigned int tot = s->getTotalSize();
   __BRTStreamType typ = s->getStreamType();
   float * data = (float *)s->getData(brook::StreamInterface::READ);
   for (unsigned int i=0;i<tot;++i) {
      if (typ!=1)printf( "{");
      for (unsigned int j=0;j<(unsigned int)typ;++j) {
         float x = data[i*typ+j];
         if (j!=0) {
            printf(",");
            printf(" ");
         }
         if (finite_flt(x))
            printf("%3.2f",x);
         else if (isnan_flt(x))
            printf("NaN");
         else 
            printf ("inf");
      }
      
      if (typ!=1)
         printf("}");
      else
         printf (" ");
      if (!flatten)
         if ((i+1)%extent[dims-1]==0)
            printf("\n");
   }
   s->releaseData(brook::StreamInterface::READ);
}
void readItem (brook::StreamInterface *s, void * p,...) {
   unsigned int dims = s->getDimension();
   std::vector<unsigned int> index;
   va_list args;
   va_start(args,p);
   for (unsigned int i=0;i<dims;++i) {
      index.push_back(va_arg(args,int));
   }
   va_end(args);
   s->readItem(p,&index[0]);
}
__BRTStream::__BRTStream( const __BRTIter& i )
  : stream(0)
{
  brook::Iter* iterator = i;

  __BRTStreamType elementType = iterator->getStreamType();
  int dimensionCount = iterator->getDimension();
  int* extents = (int*)(iterator->getExtents());

  stream = brook::RunTime::GetInstance()->CreateStream( elementType, dimensionCount, extents );
  stream->Read( iterator->getData( brook::Stream::READ ) );
  iterator->releaseData( brook::Stream::READ );
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
__BRTIter::__BRTIter(__BRTStreamType type, ...)
  : iter(NULL)
{
  std::vector<int> extents;
  std::vector<float> ranges;
  va_list args;
  va_start(args,type);
  for(;;)
  {
    int extent = va_arg(args,int);
    if( extent == -1 ) break;
    extents.push_back(extent);
  }
  for (int i=0;i<type;++i) {
     float f = (float) va_arg(args,double);
     //     fprintf(stderr, "float %f\n",f);
     ranges.push_back(f);
     f = (float) va_arg(args,double);
     //     fprintf(stderr, "float %f\n",f);
     ranges.push_back(f);
  }
  va_end(args);

  iter = brook::RunTime::GetInstance()->CreateIter( type, 
                                                    (int)extents.size(), 
                                                    &extents[0],
                                                    &ranges[0]);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
__BRTKernel::__BRTKernel(const void* code[])
  : kernel(NULL)
{
  kernel = brook::RunTime::GetInstance()->CreateKernel( code );
}
