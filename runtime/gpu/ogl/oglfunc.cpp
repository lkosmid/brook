
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "oglfunc.hpp"
#include "../gpucontext.hpp"

#define XXX(type, fn) \
   type fn;

#ifdef WIN32
RUNTIME_BONUS_WGL_FNS
#endif

RUNTIME_BONUS_GL_FNS
RUNTIME_BONUS_GL_FNS_ATI

#undef XXX


void brook::initglfunc(void) {
#ifdef WIN32
#define  XXX(type, fn) fn = (type) wglGetProcAddress(#fn); \
                       GPUAssert(fn, "Failed to load" #fn);
  RUNTIME_BONUS_GL_FNS;
  RUNTIME_BONUS_WGL_FNS;
#undef XXX   
#define  XXX(type, fn) fn = (type) wglGetProcAddress(#fn);
  RUNTIME_BONUS_GL_FNS_ATI;
#undef XXX   
#endif
}


