// dx9runtime.hpp
#ifndef GPU_RUNTIME_DX9_HPP
#define GPU_RUNTIME_DX9_HPP

#include "../gpubase.hpp"
#include "../gpuruntime.hpp"

namespace brook
{

  class GPURuntimeDX9 : public GPURuntime
  {
  public:
    static GPURuntimeDX9* create();

  private:
    GPURuntimeDX9() {}
  };

}

#endif
