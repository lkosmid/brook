// gpuiterator.hpp
#ifndef GPU_ITERATOR_HPP
#define GPU_ITERATOR_HPP

#include "gpubase.hpp"

namespace brook {

  class GPUIterator : public Iter {
  public:
    static GPUIterator* create( GPURuntime* inRuntime, __BRTStreamType inElementType,
      int inDimensionCount, int* inExtents, float* inRanges );

    virtual void* getData (unsigned int flags);
    virtual void releaseData(unsigned int flags);
    virtual const unsigned int* getExtents() const { return _extents; }
    virtual unsigned int getDimension() const { return _dimensionCount; }
    virtual unsigned int getTotalSize() const { return _totalSize; }

    void getInterpolant(
      unsigned int inOutputWidth,
      unsigned int inOutputHeight,
      GPUInterpolant& outInterpolant );

  private:
    GPUIterator( GPURuntime* inRuntime, __BRTStreamType inElementType );
    bool initialize( int inDimensionCount, int* inExtents, float* inRanges );

    unsigned int _dimensionCount;
    unsigned int _componentCount;
    unsigned int _totalSize;
    unsigned int _extents[2];
    float _ranges[8];

    GPUContext* _context;
    float4 _min1D, _max1D;
    float2 _min2D, _max2D;

    GPUInterpolant _defaultInterpolant;
//    GPUFatRect _rect;
    void* _cpuBuffer;
  };
}

#endif
