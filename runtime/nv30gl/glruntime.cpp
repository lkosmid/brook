#include "glruntime.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace brook;

static const char passthrough[] =
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";

GLRunTime::GLRunTime()
{
   int i, n;

   streamlist = NULL;

#ifdef WIN32
   createWindow();
   createWindowGLContext();
   initglfunc();
#endif

   createPBuffer(4);
   glDrawBuffer(GL_FRONT);
   glReadBuffer(GL_FRONT);
   CHECK_GL();

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


void
GLRunTime::printMemUsage(void)
{
   GLStream *s;
   int mem = workspace * workspace * pbuffer_ncomp * sizeof (float);

   std::cerr << "\n\nGL Memory Usage\n"
             << "***********************************\n"
             << "Workspace (" << workspace << " x " << workspace
             << " x " << pbuffer_ncomp << "): "
             << mem/(1024*1024) << " MB\n";

   for (s=streamlist; s; s=s->getNext()) {
      mem += s->printMemUsage();
   }
   std::cerr << "-----------------------------------\n"
             << "Total:                       "
             << mem/(1024*1024) << " MB\n\n";
}
