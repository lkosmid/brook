#include "nv30gl.hpp"
// TIM: missing include file?
//#include <brtcommon.hpp>
#include "../common.h"

using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
}

NV30GLRunTime::NV30GLRunTime() {
// TIM: missing include file?
//  BrtLog("NV30GLRunTime()");

  // Create a window
//  hwnd = createwindow();

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

NV30GLRunTime::~NV30GLRunTime() {
  // Does nothing
}
