// oglruntime.hpp
#ifndef OGL_RUNTIME_HPP
#define OGL_RUNTIME_HPP

#include "../gpuruntime.hpp"

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
