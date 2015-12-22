// glesruntime.hpp
#ifndef GLES_RUNTIME_HPP
#define GLES_RUNTIME_HPP

#include "../gpuruntime.hpp"

#define GLES_RUNTIME_STRING "gles"

namespace brook
{

  class GLESRuntime : public GPURuntime
  {

  public:
    static GLESRuntime* create( void* inContextValue, const char* device = 0 );

  protected:
    GLESRuntime() {}

    virtual bool initialize( void* inContextValue, const char* device );
  };
}

#endif
