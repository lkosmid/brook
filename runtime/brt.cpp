// brt.cpp
#include "runtime.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#ifdef _WIN32
 #define GPU_ROUTINES
#else
 #ifdef GPU_ROUTINES
  #undef GPU_ROUTINES
 #endif
#endif

#ifdef GPU_ROUTINES
//so far the following runtimes only have Windows backends
#include "dx9/dx9.hpp"
#include "nv30gl/nv30gl.hpp"
#endif
#include "cpu/cpu.hpp"
#include "brtscatterintrinsic.hpp"
__StreamScatterAssign STREAM_SCATTER_ASSIGN;
__StreamScatterAdd STREAM_SCATTER_ADD;
__StreamScatterMul STREAM_SCATTER_MUL;
static float lerp (unsigned int i, unsigned int end,float lower,float upper) {
   float frac=end>1?((float)i)/(float)(end-1):(float)upper;
   return (1-frac)*lower+frac*upper;
}
namespace brook {

  static const char* RUNTIME_ENV_VAR = "BRT_RUNTIME";
  Stream * Iter::allocateStream(int dims, 
                                int extents[],
                                float ranges[])const {
     Stream * s = new CPUStream (type,dims,extents);
     float * data = (float *)s->getData(brook::Stream::READ);
     if (dims<2) {
        for (unsigned int i=0;i<extents[0];++i) {
           for (unsigned int j=0;j<type;++j) {
              data[i*type+j]=lerp(i,extents[0],ranges[j],ranges[j+type]);
           }
        }
     }else if (dims==2){
        //now we know dims == data type;
        unsigned int i[2]={0,0};
        for (i[0]=0;i[0]<extents[0];++i[0]) {
           for (i[1]=0;i[1]<extents[1];++i[1]) {
              for (unsigned int k=0;k<2;++k) {
                 data[(i[0]*extents[1]+i[1])*2+k]=
                    lerp (i[k],extents[k],ranges[k],ranges[2+k]);
              }
           }
        }
     }else {
        assert(0);
     }
     s->releaseData(brook::Stream::READ);
       //XXX daniel this needs to be done
       //will use standard brook BRTCreateStream syntax and then copy data in
       //dx9 can then call this to easily fallback if cpu is necessary
     return s;
  }
    

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
#ifdef GPU_ROUTINES
    if (!strcmp(env, DX9_RUNTIME_STRING))
      return new DX9RunTime();

    if (!strcmp(env, NV30GL_RUNTIME_STRING))
      return new NV30GLRunTime();
#endif
	if (strcmp(env,CPU_RUNTIME_STRING)) 
		fprintf (stderr, "Unknown runtime requested: %s falling back to CPU", env);
    return new CPURunTime();
  }
}

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
  for (unsigned int i=0;i<extents.size();++i) {
     switch (type) {
     case __BRTFLOAT4:
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        break;
     case __BRTFLOAT3:
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        break;
     case __BRTFLOAT2:
        ranges.push_back(va_arg(args,float));
        ranges.push_back(va_arg(args,float));
        break;
     default:
        ranges.push_back(va_arg(args,float));
     }
  }
  va_end(args);

  iter = brook::RunTime::GetInstance()->CreateIter( type, 
                                                    extents.size(), 
                                                    &extents[0],
                                                    &ranges[0]);
}


__BRTKernel::__BRTKernel(const void* code[])
  : kernel(NULL)
{
  kernel = brook::RunTime::GetInstance()->CreateKernel( code );
}
