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
}