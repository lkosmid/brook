
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
  // XXX: TO DO

   fprintf (stderr, "NV30GL: Iter creation not supported yet.\n");
   exit(1);

  return (Iter *) 0;
}

NV30GLRunTime::~NV30GLRunTime() {
  // Does nothing
}
