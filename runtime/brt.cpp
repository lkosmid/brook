// brt.cpp
#include "runtime.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "dx9/dx9.hpp"
#include "nv30gl/nv30gl.hpp"
#include "cpu/cpu.hpp"
namespace brook {

  static const char* RUNTIME_ENV_VAR = "BRT_RUNTIME";

  RunTime* RunTime::GetInstance() {
    static RunTime* sResult = CreateInstance();
    return sResult;
  }

  RunTime* RunTime::CreateInstance() {
    char *env = getenv(RUNTIME_ENV_VAR);

    if (!env) {
      fprintf (stderr,"No runtime requested. Using CPU\n");
      return new CPURunTime();
    }
    if (!strcmp(env, DX9_RUNTIME_STRING))
      return new DX9RunTime();

    if (!strcmp(env, NV30GL_RUNTIME_STRING))
      return new NV30GLRunTime();
	if (strcmp(env,CPU_RUNTIME_STRING)) 
		fprintf (stderr, "Unknown runtime requested: %s falling back to CPU", env);
    return new CPURunTime();
  }
}

__BRTStream::__BRTStream(__BRTStreamType type, ...)
  : stream(NULL)
{
  int dimensions = 0;
  int extents[brook::MAXSTREAMDIMS];

  va_list args;
  va_start(args,type);
  for(;;)
  {
    int extent = va_arg(args,int);
    if( extent == -1 ) break;
    extents[dimensions++] = extent;
  }
  va_end(args);

  stream = brook::RunTime::GetInstance()->CreateStream( type, dimensions, extents );
}

__BRTKernel::__BRTKernel(const void* code[])
  : kernel(NULL)
{
  kernel = brook::RunTime::GetInstance()->CreateKernel( code );
}
