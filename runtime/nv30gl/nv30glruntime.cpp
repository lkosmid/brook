#include "nv30gl.hpp"
#include "../common.h"

using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
}

NV30GLRunTime::NV30GLRunTime() {

  // Create a window
  hwnd = createwindow();

  // Create a window glcontext
//  hglrc_window = createglcontext();

  // Create our floating point workspace
//  hglrc_workspace = createpbuffer();

  

}

Kernel * NV30GLRunTime::CreateKernel(const void*[]) {
  // XXX: TO DO
  return (Kernel *) 0;
}

Stream * NV30GLRunTime::CreateStream(__BRTStreamType type, int dims, int extents[]) {
  // XXX: TO DO
  return (Stream *) 0;
}
Iter * NV30GLRunTime::CreateIter(__BRTStreamType type, int dims, int extents[],float r[]) {
  // XXX: TO DO
  return (Iter *) 0;
}

NV30GLRunTime::~NV30GLRunTime() {
  // Does nothing
}
