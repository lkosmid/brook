
#include "oglruntime.hpp"
#include "oglcontext.hpp"

using namespace brook;

OGLRuntime* OGLRuntime::create( void* inContextValue, const char* device )
{
    OGLRuntime* result = new OGLRuntime();
    if( result && result->initialize( inContextValue, device ) )
        return result;
    delete result;
    return NULL;
}

bool OGLRuntime::initialize( void* inContextValue, const char* device )
{
  // Create the context
  OGLContext *ctx = OGLContext::create(device);

  if( inContextValue )
    ctx->shareLists( (HGLRC) inContextValue );

  if( !GPURuntime::initialize( ctx ) )
  {
      delete ctx;
      return false;
  }

  return true;
}
