#include "gpuruntime.hpp"
#include "gpubase.hpp"

#include "gpucontext.hpp"
#include "gpukernel.hpp"
#include "gpustream.hpp"
#include "gpuiterator.hpp"

namespace brook
{

  GPURuntime::GPURuntime()
    : _context(NULL)
  {
    for( size_t i = 0; i < kGPUReductionTempBufferCount; i++ )
    {
      _reductionTempBuffers[i] = NULL;
      _reductionTempBufferWidths[i] = 0;
      _reductionTempBufferHeights[i] = 0;
    }
    _reductionTargetBuffer = NULL;
  }

  bool GPURuntime::initialize( GPUContext* inContext )
  {
    GPUAssert( inContext, "No context provided for gpu runtime." );

    _context = inContext;
    return true;
  }

  GPURuntime::~GPURuntime()
  {
    // TODO: release passthroughs and such
    if( _context )
      delete _context;
  }

  Kernel* GPURuntime::CreateKernel(const void* source[]) {
    Kernel* result = GPUKernel::create( this, source );
    GPUAssert( result != NULL, "Unable to allocate a kernel, exiting." );
    return result;
  }

  Stream* GPURuntime::CreateStream(
    int fieldCount, const __BRTStreamType fieldTypes[],
    int dims, const int extents[])
  {
    Stream* result = GPUStream::create( this, fieldCount, fieldTypes, dims, extents );
    GPUAssert( result != NULL, "Unable to allocate a stream, exiting." );
    return result;
  }

  Iter* GPURuntime::CreateIter(
    __BRTStreamType type, int dims, int extents[], float r[] )
  {
    Iter* result = GPUIterator::create( this, type, dims, extents , r );
    GPUAssert( result != NULL, "Unable to allocate an iterator, exiting." );
    return result;
  }

  GPUContext::TextureHandle GPURuntime::getReductionTempBuffer(
    GPUReductionTempBufferID inBufferID,
    size_t inMinWidth, size_t inMinHeight,
    size_t* outWidth, size_t* outHeight )
  {
    GPUAssert( inBufferID >= 0 && inBufferID < kGPUReductionTempBufferCount,
      "Invalid reduction temp buffer requested." );

    GPUContext::TextureHandle& buffer = _reductionTempBuffers[inBufferID];
    size_t& width = _reductionTempBufferWidths[inBufferID];
    size_t& height = _reductionTempBufferHeights[inBufferID];
    
    if( buffer == NULL || width < inMinWidth || height < inMinHeight )
    {
      if( buffer != NULL )
        _context->releaseTexture( buffer );

      if( inMinWidth > width )
        width = inMinWidth;
      if( inMinHeight > height )
        height = inMinHeight;

      buffer = _context->createTexture2D( width, height, GPUContext::kTextureFormat_Float4 );
      GPUAssert( buffer != NULL, "Failed to allocate reduction temp buffer." );
    }
    
    *outWidth = width;
    *outHeight = height;
    return buffer;
  }

  GPUContext::TextureHandle GPURuntime::getReductionTargetBuffer()
  {
    if( _reductionTargetBuffer == NULL )
    {
      _reductionTargetBuffer = _context->createTexture2D( 1, 1, GPUContext::kTextureFormat_Float4 );
      GPUAssert( _reductionTargetBuffer != NULL, "Failed to allocate reduction target buffer." );
    }
    return _reductionTargetBuffer;
  }
}