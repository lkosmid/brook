
#include "nv30gl.hpp"

#include <stdio.h>

using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
}

NV30GLRunTime::NV30GLRunTime() {

  // Create a window
  createWindow();

  // Create a window glcontext
  createWindowGLContext();
  
  // Initialize gl extension function pointers
  initglfunc();

  // Create our floating point workspace
  createPBuffer();


  // Initial GL State
  glDrawBuffer(GL_FRONT);
  glReadBuffer(GL_FRONT);
  glEnable(GL_FRAGMENT_PROGRAM_NV);

  CHECK_GL();

}

Kernel * 
NV30GLRunTime::CreateKernel(const void* sourcelist[]) {
   return new NV30GLKernel (this, sourcelist);
}

Stream *
NV30GLRunTime::CreateStream(__BRTStreamType type,
                            int dims, int extents[]) {
  return new NV30GLStream( this, type, dims, extents );
}

Iter * 
NV30GLRunTime::CreateIter(__BRTStreamType type, 
                          int dims, int extents[],float r[]) {
   return new NV30GLIter(this, type, dims, extents, r);
}

NV30GLRunTime::~NV30GLRunTime() {
  // Does nothing
}
