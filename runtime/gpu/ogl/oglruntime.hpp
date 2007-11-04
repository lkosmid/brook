// oglruntime.hpp
#ifndef OGL_RUNTIME_HPP
#define OGL_RUNTIME_HPP

#include "../gpuruntime.hpp"

#define OGL_RUNTIME_STRING "ogl"

namespace brook
{

  class OGLRuntime : public GPURuntime
  {

  public:
    static OGLRuntime* create( void* inContextValue, const char* device = 0 );

  protected:
    OGLRuntime() {}

    virtual bool initialize( void* inContextValue, const char* device );
  };
}

#endif
