
#include "nv30gl.hpp"

#include <stdio.h>
#include  <string.h>
using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
}

static const char passthrough[] =        \
"!!FP1.0\n"                              \
"TEX  R0, f[TEX0].xyyy, TEX0, RECT;\n"   \
"MOVR o[COLR], R0;\n"                    \
"END\n";

NV30GLRunTime::NV30GLRunTime()
{
   int i, n;
   // Create a window
   createWindow();
   
   // Create a window glcontext
   createWindowGLContext();
   
   initglfunc();
   
   createPBuffer(4);
   
   // Initial GL State
   glDrawBuffer(GL_FRONT);
   glReadBuffer(GL_FRONT);
   glEnable(GL_FRAGMENT_PROGRAM_NV);
   CHECK_GL();
   
   glGenProgramsNV (1, &passthrough_id);
   glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, 
                    passthrough_id, strlen(passthrough), 
                    (const GLubyte*) passthrough);
   CHECK_GL();
   
   glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, (GLint *) &n);
   for (i=0; i<n; i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+i);
      glEnable(GL_TEXTURE_RECTANGLE_NV);
   }
   CHECK_GL();
   
   streamlist = NULL;
}

Kernel * 
NV30GLRunTime::CreateKernel(const void* sourcelist[]) {
   return new NV30GLKernel (this, sourcelist);
}

Stream *
NV30GLRunTime::CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]) {
  assert(fieldCount == 1);
  __BRTStreamType type = fieldTypes[0];
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
