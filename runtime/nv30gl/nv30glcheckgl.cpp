#if !defined(__GNUC__) || __GNUC__ != 2
#include <ios>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>

#include "nv30gl.hpp"

static const unsigned int nerrors = 6;

static char error_txt[][32] = {
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_STACK_OVERFLOW", 
    "GL_STACK_UNDERFLOW",
    "GL_OUT_OF_MEMORY" };


void brook::__check_gl(int line, char *file) {
   GLenum r = glGetError();
   if (r != GL_NO_ERROR) {
      if (r - GL_INVALID_ENUM >= nerrors)
         std::cerr << "NV30GL: Unknown GL error on line "
                   << line << " of " << "%s\n"; 
      else
         std::cerr << "NV30GL: glGetError returned " 
                   << error_txt[r - GL_INVALID_ENUM] 
                   << " on line "<< line << " of " << file << "\n";

      if (r == GL_OUT_OF_MEMORY) {
         NV30GLRunTime *rt = (NV30GLRunTime *) RunTime::GetInstance();
         NV30GLStream *s;
         int w = NV30GLRunTime::workspace;
         int mem = w * w * rt->pbuffer_ncomp * sizeof (float);

         std::cerr << "\n\nNV30GL Memory Usage\n"
                   << "***********************************\n"
                   << "Workspace (" << w << " x " << w
                   << " x " << rt->pbuffer_ncomp << "): " 
                   << mem/(1024*1024) << " MB\n";

         for (s=rt->streamlist; s; s=s->next) {
            int size = s->width*s->height*s->ncomp*sizeof(float);

         std::cerr << "Stream    (" << s->width << " x " 
                      << s->height << " x " << s->ncomp
                      << "): ";
            if (size < 1024) 
               std::cerr << size << " bytes\n";
            else if (size < 1024*1024)
               std::cerr << size/1024 << " KB\n";
            else
               std::cerr << size/(1024*1024) << " MB\n";
            mem += size;
         }
         std::cerr << "-----------------------------------\n"
                   << "Total:                       " 
                   << mem/(1024*1024) << " MB\n\n";
      }
      assert (r==GL_NO_ERROR);
  }
  return;
}
