// brt.cpp
#include "runtime.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>


#ifdef WIN32
#include "dx9/dx9.hpp"
#endif

#include "nv30gl/nv30gl.hpp"
#include "cpu/cpu.hpp"
#include "brtscatterintrinsic.hpp"

__StreamScatterAssign STREAM_SCATTER_ASSIGN;
__StreamScatterAdd STREAM_SCATTER_ADD;
__StreamScatterMul STREAM_SCATTER_MUL;
__StreamGatherInc STREAM_GATHER_INC;
__StreamGatherFetch STREAM_GATHER_FETCH;

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

#ifdef WIN32
    if (!strcmp(env, DX9_RUNTIME_STRING))
    {
      RunTime* result = DX9RunTime::create();
      if( result != NULL ) return result;
      fprintf(stderr, 
	      "Unable to initialize DX9 runtime, falling back to CPU\n");
      return new CPURunTime();
    }
#endif

    if (!strcmp(env, NV30GL_RUNTIME_STRING))
      return new NV30GLRunTime();

    if (strcmp(env,CPU_RUNTIME_STRING)) 
      fprintf (stderr, 
	       "Unknown runtime requested: %s falling back to CPU\n", env);
    return new CPURunTime();
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

  stream = brook::RunTime::GetInstance()->CreateStream( type, extents.size(), &extents[0] );
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
                                                    extents.size(), 
                                                    &extents[0],
                                                    &ranges[0]);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
__BRTKernel::__BRTKernel(const void* code[])
  : kernel(NULL)
{
  kernel = brook::RunTime::GetInstance()->CreateKernel( code );
}
