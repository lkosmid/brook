
#pragma once

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "oglfunc.hpp"
#include "../gpucontext.hpp"

using namespace brook;

#ifdef WIN32
/*
 * For some inexplicable reason, on Windows hosts, the OpenGL libraries
 * don't actually export some of their symbols.  Instead, you have to use
 * wglGetProcAddress() to pry them out.  This doesn't stop their header
 * files from defining said symbols, but you get awkward compiler complaints
 * if you actually allow them to be defined.  So, we fake it all here
 * instead of defining either WGL_WGLEXT_PROTOTYPES or GL_GLEXT_PROTOTYPES.
 */

#define RUNTIME_BONUS_GL_FNS \
   XXX(PFNWGLCREATEPBUFFERARBPROC,     wglCreatePbufferARB)            \
   XXX(PFNWGLGETPBUFFERDCARBPROC,      wglGetPbufferDCARB)             \
   XXX(PFNWGLRELEASEPBUFFERDCARBPROC,  wglReleasePbufferDCARB)         \
   XXX(PFNWGLDESTROYPBUFFERARBPROC,    wglDestroyPbufferARB)           \
   XXX(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB)        \
   XXX(PFNWGLBINDTEXIMAGEARBPROC,      wglBindTexImageARB)             \
   XXX(PFNWGLRELEASETEXIMAGEARBPROC,   wglReleaseTexImageARB)          \
                                                                       \
   XXX(PFNGLMULTITEXCOORD2FVARBPROC,   glMultiTexCoord2fvARB)          \
   XXX(PFNGLMULTITEXCOORD4FVARBPROC,   glMultiTexCoord4fvARB)          \
   XXX(PFNGLACTIVETEXTUREARBPROC,      glActiveTextureARB)             \
   XXX(PFNGLGENPROGRAMSARBPROC,        glGenProgramsARB)               \
   XXX(PFNGLBINDPROGRAMARBPROC,        glBindProgramARB)               \
   XXX(PFNGLPROGRAMSTRINGARBPROC,      glProgramStringARB)             \
   XXX(PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB) \


#define RUNTIME_BONUS_NV_FNS \
   XXX(PFNGLGENPROGRAMSNVPROC,         glGenProgramsNV)                \
   XXX(PFNGLLOADPROGRAMNVPROC,         glLoadProgramNV)                \
   XXX(PFNGLBINDPROGRAMNVPROC,         glBindProgramNV)                \
   XXX(PFNGLPROGRAMNAMEDPARAMETER4FNVPROC, glProgramNamedParameter4fNV)\

#define XXX(type, fn) \
   type brook::fn;

RUNTIME_BONUS_GL_FNS
RUNTIME_BONUS_NV_FNS
#undef XXX
#endif

void initglfunc(void) {
#ifdef WIN32
#define  XXX(type, fn) fn = (type) wglGetProcAddress(#fn); \
                       GPUAssert(fn, "Failed to load" #fn);
  RUNTIME_BONUS_GL_FNS;
#undef XXX   
#endif
}


