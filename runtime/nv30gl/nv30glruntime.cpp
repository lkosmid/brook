#include "nv30gl.hpp"

using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
}

Kernel *
NV30GLRunTime::CreateKernel(const void* sourcelist[]) {
   return new NV30GLKernel (this, sourcelist);
}

