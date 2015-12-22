
#include "glesruntime.hpp"
#include "glescontext.hpp"

using namespace brook;

GLESRuntime* GLESRuntime::create( void* inContextValue, const char* device )
{
    GLESRuntime* result = new GLESRuntime();
    if( result && result->initialize( inContextValue, device ) )
        return result;
    delete result;
    return NULL;
}

bool GLESRuntime::initialize( void* inContextValue, const char* device )
{
  // Create the context
  GLESContext *ctx = GLESContext::create(device);

  if( inContextValue )
    ctx->shareLists( (HGLRC) inContextValue );

  if( !GPURuntime::initialize( ctx ) )
  {
      delete ctx;
      return false;
  }

  return true;
}
