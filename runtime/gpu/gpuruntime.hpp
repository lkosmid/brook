// gpuruntime.hpp
#ifndef GPU_RUNTIME_HPP
#define GPU_RUNTIME_HPP

#include "gpucontext.hpp"

namespace brook
{

  class GPURuntime : public RunTime
  {
  public:

    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]);
    virtual Iter* CreateIter(__BRTStreamType type, 
      int dims, 
      int extents[],
      float range[]);
    virtual ~GPURuntime();

    GPUContext* getContext() { return _context; }

  protected:
    GPURuntime();
    bool initialize( GPUContext* inContext );

  private:
    GPUContext* _context;
  };
}

#endif
