
#include <ios>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "nv30gl.hpp"

static const int nerrors = 6;

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
      assert (r==GL_NO_ERROR);
  }
  return;
}
