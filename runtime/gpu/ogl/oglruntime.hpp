// oglruntime.hpp
#ifndef OGL_RUNTIME_HPP
#define OGL_RUNTIME_HPP

#include "../gpuruntime.hpp"

#define ARB_RUNTIME_STRING "arb"
#define NV30GL_RUNTIME_STRING "nv30gl"

namespace brook
{

  class OGLRuntime : public GPURuntime
  {

  public:
    static OGLRuntime* create();

  private:
    OGLRuntime() {}
  };
}

#endif
