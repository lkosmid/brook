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
namespace brook {
   GLArch getGLArch() {
      if (strstr((const char *) glGetString(GL_VENDOR), "ATI"))
         return ARCH_ATI;
      else if (strstr((const char *)glGetString(GL_VENDOR), "NVIDIA"))
         return  ARCH_NV30;
      else {
         fprintf (stderr, "GL: Warning unknown card\n\t%s\n\t%s\n",
                  glGetString(GL_VENDOR), glGetString(GL_RENDERER));
         return ARCH_UNKNOWN;
      }
   }
}
GLRunTime::GLRunTime()
{
   int i, n;

   streamlist = NULL;
   pbuffer_ncomp = -1;

#ifdef _WIN32
   createWindow();
   createWindowGLContext();
   arch = getGLArch();
#else
   //   arch = ARCH_NV30;
#endif
#ifdef _WIN32
   initglfunc();
#endif
   createPBuffer(4);
#ifndef _WIN32
   arch = getGLArch();      
#endif

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
