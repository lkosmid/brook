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

  unsigned int StreamInterface::getElementSize() const
  {
    unsigned int result = 0;
    int fieldCount = getFieldCount();
    for( int i = 0; i < fieldCount; i++ )
    {
      __BRTStreamType fieldType = getIndexedFieldType(i);
      switch(fieldType)
      {
      case __BRTFLOAT:
        result += sizeof(float);
        break;
      case __BRTFLOAT2:
        result += sizeof(float2);
        break;
      case __BRTFLOAT3:
        result += sizeof(float3);
        break;
      case __BRTFLOAT4:
        result += sizeof(float4);
        break;
      default:
        assert(false && "invalid stream element type");
      };
    }
    return result;
  }

  void StreamInterface::readItem (void * output, unsigned int * index){
     unsigned int linearindex = index[0];
     unsigned int dim = getDimension();
     const unsigned int * extents = getExtents();
     for (unsigned int i=1;i<dim;++i) {
        linearindex*=extents[i];
        linearindex+=index[i];
     }
     unsigned int size = getElementSize();
     char * data = (char*)getData(READ);
     data+=linearindex*size;
     memcpy (output,data,size);
     releaseData(READ);
  }

  
  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  stream::stream(const __BRTStreamType* inElementTypes, ...)
    : _stream(0)
  {
    std::vector<__BRTStreamType> elementTypes;
    std::vector<int> extents;

    const __BRTStreamType* e = inElementTypes;
    while(*e != __BRTNONE)
    {
      elementTypes.push_back(*e);
      e++;
    }

    va_list args;
    va_start(args,inElementTypes);
    for(;;)
    {
      int extent = va_arg(args,int);
      if( extent == -1 ) break;
      extents.push_back(extent);
    }
    va_end(args);

    _stream = brook::RunTime::GetInstance()->CreateStream(
      (int)elementTypes.size(), &elementTypes[0], (int)extents.size(), &extents[0] );
  }
  stream::stream(int * extents,int dims,const __BRTStreamType *type)
    : _stream(0)
  {
     std::vector<__BRTStreamType>elementTypes;
     const __BRTStreamType * e = type;
     while (*e!=__BRTNONE) {
        elementTypes.push_back(*e);
        e++;
     }
    _stream = brook::RunTime::GetInstance()->CreateStream
       (elementTypes.size(),&elementTypes[0], dims,extents);
  }

  stream::stream( const ::brook::iter& i )
    : _stream(0)
  {
    ::brook::Iter* iterator = i;

    __BRTStreamType elementType = iterator->getIndexedFieldType(0);
    int dimensionCount = iterator->getDimension();
    int* extents = (int*)(iterator->getExtents());

    _stream = brook::RunTime::GetInstance()->CreateStream( 1, &elementType, dimensionCount, extents );
    _stream->Read( iterator->getData( brook::Stream::READ ) );
    iterator->releaseData( brook::Stream::READ );
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  iter::iter(__BRTStreamType type, ...)
    : _iter(0)
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

    _iter = brook::RunTime::GetInstance()->CreateIter( type, 
                                                      (int)extents.size(), 
                                                      &extents[0],
                                                      &ranges[0]);
  }

}
__BRTStream * sentinelStream (int dim) {
   static std::vector <__BRTStream *> sentinels;
   if (dim<(int)sentinels.size())
      if (sentinels[dim]!=0)
         return sentinels[dim];
   while ((int)sentinels.size()<=dim)
      sentinels.push_back(0);
   std::vector<int> extents;
   for (int i=0;i<dim;++i){
      extents.push_back(1);
   }
   sentinels[dim]=new brook::stream(&extents[0],
                                    dim,
                                    brook::getStreamType((float*)0));   
   float inf = 1.0f/(float)floor(.5);
   streamRead(*sentinels[dim],&inf);
   return sentinels[dim];
}
void streamPrint(brook::StreamInterface * s, bool flatten) {
   unsigned int dims = s->getDimension();
   const unsigned int * extent = s->getExtents();
   unsigned int tot = s->getTotalSize();
   unsigned int numfloats = 0;
   unsigned int numfields = s->getFieldCount();
   for (unsigned int fields= 0; fields<numfields;++fields) {
     numfloats+=s->getIndexedFieldType(fields);
   }
   float * data = (float *)s->getData(brook::StreamInterface::READ);
   for (unsigned int i=0;i<tot;++i) {
      if (numfields!=1)printf( "{");
      for (unsigned int j=0;j<numfields;++j) {
         float x = data[i*numfields+j];
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
      
      if (numfields!=1)
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
__BRTKernel::__BRTKernel(const void* code[])
  : kernel(NULL)
{
  kernel = brook::RunTime::GetInstance()->CreateKernel( code );
}
