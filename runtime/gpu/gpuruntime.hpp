// gpuruntime.hpp
#ifndef GPU_RUNTIME_HPP
#define GPU_RUNTIME_HPP

#include "gpucontext.hpp"

namespace brook
{
  enum
  {
    kGPUReductionTempBuffer_Swap0 = 0,
    kGPUReductionTempBuffer_Swap1,
    kGPUReductionTempBuffer_Slop,
    kGPUReductionTempBufferCount
  };
  typedef int GPUReductionTempBufferID;

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

    // internal GPURuntime methods (not in RunTime interface)
    GPUContext* getContext() { return _context; }
    GPUContext::TextureHandle getReductionTempBuffer(
      GPUReductionTempBufferID inBufferID,
      size_t inMinWidth, size_t inMinHeight,
      size_t* outWidth, size_t* outHeight );
    GPUContext::TextureHandle getReductionTargetBuffer();

  protected:
    GPURuntime();
    bool initialize( GPUContext* inContext );

  private:
    GPUContext* _context;
    GPUContext::TextureHandle _reductionTempBuffers[kGPUReductionTempBufferCount];
    size_t _reductionTempBufferWidths[kGPUReductionTempBufferCount];
    size_t _reductionTempBufferHeights[kGPUReductionTempBufferCount];
    GPUContext::TextureHandle _reductionTargetBuffer;
  };
}

#endif
