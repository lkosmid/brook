// gpuiterator.hpp
#ifndef GPU_ITERATOR_HPP
#define GPU_ITERATOR_HPP

#include "gpubase.hpp"

namespace brook {

  class GPUIterator : public Iter {
  public:
    static GPUIterator* create( GPURuntime* inRuntime, 
                                StreamType inElementType,
                                unsigned int inDimensionCount, 
                                const unsigned int inExtents[], 
                                const float inRanges[] );

    virtual void* getData (unsigned int flags);
    virtual void releaseData(unsigned int flags);
    virtual const unsigned int* getExtents() const { return _extents; }
    virtual unsigned int getDimension() const { return _dimensionCount; }
    virtual unsigned int getTotalSize() const { return _totalSize; }

    virtual const unsigned int * getDomainMin() const {
      return _domainMin;
    }
    
    virtual const unsigned int * getDomainMax() const {
      return _domainMax;
    }


    void getInterpolant(
      unsigned int inOutputWidth,
      unsigned int inOutputHeight,
      GPUInterpolant& outInterpolant );

  private:
    GPUIterator( GPURuntime* inRuntime, StreamType inElementType );
    bool initialize( unsigned int inDimensionCount, 
                     const unsigned int inExtents[], 
                     const float inRanges[] );

    unsigned int _dimensionCount;
    unsigned int _componentCount;
    unsigned int _totalSize;
    unsigned int _extents[2];
    float _ranges[8];
    unsigned int _domainMin[2];
    unsigned int _domainMax[2];

    GPUContext* _context;
    float4 _min1D, _max1D;
    float2 _min2D, _max2D;

    GPUInterpolant _defaultInterpolant;
    void* _cpuBuffer;
  };
}

#endif
