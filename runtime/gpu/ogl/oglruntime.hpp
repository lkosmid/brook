// oglruntime.hpp
#ifndef OGL_RUNTIME_HPP
#define OGL_RUNTIME_HPP

#include "../gpuruntime.hpp"

#define ARB_RUNTIME_STRING "arb"

namespace brook
{

  class OGLRuntime : public GPURuntime
  {

  public:
    static OGLRuntime* create();

  protected:
    OGLRuntime() {}

    virtual bool initialize();
  };
}

#endif
