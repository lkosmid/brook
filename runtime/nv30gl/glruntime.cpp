#include "glruntime.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace brook;

GLRunTime::GLRunTime()
{
#ifdef WIN32
   createWindow();
   createWindowGLContext();
   initglfunc();
#endif

   createPBuffer(4);
   glDrawBuffer(GL_FRONT);
   glReadBuffer(GL_FRONT);
   CHECK_GL();

   streamlist = NULL;
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
