
#if !defined(__GNUC__) || __GNUC__ != 2
#include <ios>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"
#include "nvcontext.hpp"

using namespace brook;

#ifdef WIN32
#define XXX(type, fn)   type brook::fn;
RUNTIME_BONUS_GL_FNS
RUNTIME_BONUS_NV_FNS
#undef XXX
#endif

void
OGLContext::init () {
  int i;
  
  initPbuffer();

  _passthroughVertexShader = NULL;
  _passthroughPixelShader = NULL;

  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i);
  _slopTextureUnit = (unsigned int) (i-1);
}




void initglfunc(void) {
#ifdef WIN32
#define  XXX(type, fn) fn = (type) wglGetProcAddress(#fn); assert(fn);
  RUNTIME_BONUS_GL_FNS;
#undef XXX   
#endif
}



/* Creates a vendor specific backend */
OGLContext *
OGLContext::oglContextFactory(void) {
  return new NVContext();
}

static const unsigned int nglErrors = 6;

static char glError_txt[][32] = {
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_STACK_OVERFLOW",
    "GL_STACK_UNDERFLOW",
    "GL_OUT_OF_MEMORY",
};


void brook::__check_gl(int line, char *file) {
  GLenum r = glGetError();
  
  if (r != GL_NO_ERROR) {
    if (r - GL_INVALID_ENUM >= nglErrors)
      std::cerr << "GL: Unknown GL error on line "
                << line << " of " << "%s\n";
    else
      std::cerr << "GL: glGetError returned "
                << glError_txt[r - GL_INVALID_ENUM]
                << " on line "<< line << " of " << file << "\n";
    
    assert (r==GL_NO_ERROR);
  }
  return;
}
