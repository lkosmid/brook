
#ifdef WIN32
#include <windows.h>
#else
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
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
#else
#define  XXX(type, fn) fn = (type) glXGetProcAddressARB(#fn); \
                       GPUAssert(fn, "Failed to load" #fn);
#endif

  RUNTIME_BONUS_GL_FNS;

#ifdef WIN32
  RUNTIME_BONUS_WGL_FNS;
#endif

#undef XXX   
#ifdef WIN32
#define  XXX(type, fn) fn = (type) wglGetProcAddress(#fn);
#else
#define  XXX(type, fn) fn = (type) glXGetProcAddressARB((const GLubyte *) #fn);
#endif

  RUNTIME_BONUS_GL_FNS_ATI;

}


