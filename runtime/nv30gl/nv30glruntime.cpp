#include "nv30gl.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace brook;

namespace brook {
  const char* NV30GL_RUNTIME_STRING = "nv30gl";
  const char* ARB_RUNTIME_STRING = "arb";
}

static const char passthrough[] =
#if 0
"!!FP1.0\n"
"TEX  R0, f[TEX0].xyyy, TEX0, RECT;\n"
"MOVR o[COLR], R0;\n"
"END\n";
#else
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";
#endif

NV30GLRunTime::NV30GLRunTime() : GLRunTime()
{
   int i, n;

   glEnable(GL_FRAGMENT_PROGRAM_ARB);
   CHECK_GL();

   glGenProgramsARB(1, &passthrough_id);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, passthrough_id);
   glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                      strlen(passthrough), (GLubyte *) passthrough);
   CHECK_GL();

   glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, (GLint *) &n);
   for (i=0; i<n; i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+i);
      glEnable(GL_TEXTURE_RECTANGLE_EXT);
   }
   CHECK_GL();
}

Kernel *
NV30GLRunTime::CreateKernel(const void* sourcelist[]) {
   return new NV30GLKernel (this, sourcelist);
}

Stream *
NV30GLRunTime::CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]) {
  return new GLStream(this, fieldCount, fieldTypes, dims, extents);
}

Iter *
NV30GLRunTime::CreateIter(__BRTStreamType type,
                          int dims, int extents[],float r[]) {
   return new GLIter(this, type, dims, extents, r);
}
