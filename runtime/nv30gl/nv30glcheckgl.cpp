
#include <stdio.h>

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
         fprintf (stderr, "Unknown GL error on line %d of %s\n", 
                  line, file);
      else
         fprintf (stderr, "glGetError returned %s on line %d of %s\n", 
                  error_txt[r - GL_INVALID_ENUM], line, file);
      exit(1);
  }
  return;
}
