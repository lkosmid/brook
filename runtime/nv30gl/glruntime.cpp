#include "glruntime.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace brook;

GLRunTime::GLRunTime()
{
   createWindow();
   createWindowGLContext();
   initglfunc();

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
