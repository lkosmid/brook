#include <nv30gl.hpp>
#include <brtcommon.hpp>

NV30GLRunTime::NV30GLRunTime() {
  BrtLog("NV30GLRunTime()");

  // Create a window
  hwnd = createwindow();

  // Create a window glcontext
  hglrc_window = createglcontext();

  // Create our floating point workspace
  hglrc_workspace = createpbuffer();

  

}

__BrookKernel * NV30GLRunTime::LoadKernel(const char*[]) {
  // XXX: TO DO
  return (__BrookKernel *) 0;
}

__BrookStream * NV30GLRunTime::CreateStream(const char type[], int dims, int extents[]) {
  // XXX: TO DO
  return (__BrookStream *) 0;
}

NV30GLRunTime::~NV30GLRunTime() {
  // Does nothing
}
