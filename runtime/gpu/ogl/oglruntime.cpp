
#include "oglruntime.hpp"
#include "oglwindow.hpp"
#include "nvcontext.hpp"
#include "aticontext.hpp"

using namespace brook;

OGLRuntime* OGLRuntime::create(void)
{
    OGLRuntime* result = new OGLRuntime();
    if( result && result->initialize() )
        return result;
    delete result;
    return NULL;
}

bool OGLRuntime::initialize(void)
{
  // Detect platform
  OGLWindow* wnd = new OGLWindow();
  bool isNV  = NVContext::isCompatibleContext();
  bool isATI = ATIContext::isCompatibleContext();
  if (isNV && isATI) {
    fprintf(stderr, "Graphics Hardware is compatible with both\n"
            "ATI and NV extensions.  Checking vendor strings.\n");
    isNV  = NVContext::isVendorContext();
    isATI = ATIContext::isVendorContext();
    GPUAssert(!isNV || !isATI,
              "Hardware cannot be both ATI and NV\n");
  }
  if (!isNV && !isATI)
    return NULL;

  delete wnd;

  // Create the context
  OGLContext *ctx;
  if (isNV)
    ctx = NVContext::create();
  else if (isATI)
    ctx = ATIContext::create();
  else {
    fprintf(stderr, "Unfamiliar hardware \"%s\"\n  Defaulting to ATI...",
            glGetString(GL_RENDERER));
    ctx = ATIContext::create();
  }

  if( !GPURuntime::initialize( ctx ) )
  {
      delete ctx;
      return false;
  }

  return true;
}
