#include "arb.hpp"

using namespace brook;

namespace brook {
  const char* ARB_RUNTIME_STRING = "arb";
}

Kernel *
ARBRunTime::CreateKernel(const void* sourcelist[]) {
   return new ARBKernel (this, sourcelist);
}

