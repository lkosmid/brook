
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
  
  // Create our floating point workspace
  createPBuffer();

}

Kernel * 
NV30GLRunTime::CreateKernel(const void*[]) {
  // XXX: TO DO

   fprintf (stderr, "NV30GL: Kernel creation not supported yet.\n");
   exit(1);
   
   return (Kernel *) 0;
}

Stream *
NV30GLRunTime::CreateStream(__BRTStreamType type,
                            int dims, int extents[]) {
   // XXX: TO DO

   fprintf (stderr, "NV30GL: Stream creation not supported yet.\n");
   exit(1);
   
   return (Stream *) 0;
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
